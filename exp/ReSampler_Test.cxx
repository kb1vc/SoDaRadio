#include "../src/ReSampler.hxx"
#include <SoDa/Format.hxx>
#include <iostream>
#include <fstream>
#include <chrono>
#include <cmath>

template <typename T>
float toDb(T v1, T v2 = 1.0) {
  return 20 * (std::log10(std::abs(v1) + 1e-20) - std::log10(std::abs(v2) + 1e-20));
}
template <typename T>
void copyVec(std::vector<T> & out, uint32_t out_start, const std::vector<T> & in,
	     uint32_t in_start = 0, uint32_t in_size = 0) {

  if(in_size == 0) in_size = in.size();
  
  for(int i = 0; i < in_size; i++) {
    out[out_start + i] = in[in_start + i]; 
  }
}

bool doTestFreq(SoDa::ReSampler & resamp, 
		double freq, 
		double FS_in, 
		double FS_out, 
		double in_band_limit,
		double stop_band_limit, 
		uint32_t num_passes) {

  double freq_limit = std::min(FS_in, FS_out) / 2.0;   
  bool is_in_band = std::abs(freq) < freq_limit;
    
  // create input and output vectors
  auto U = resamp.getU();
  auto D = resamp.getD();

  auto Lx = resamp.getInputBufferSize() * num_passes;
  auto Ly = resamp.getOutputBufferSize() * num_passes;
 
  std::vector<std::complex<float>> x(Lx);
  std::vector<std::complex<float>> y(Ly);  

  // We ignore the first part of the operation, as we need to "prime the pump"
  auto Lx_out = Lx - resamp.getInputBufferSize();
  auto Ly_out = Ly - resamp.getOutputBufferSize();

  std::cerr << SoDa::Format("Lx = %0 Lx_out = %1, Ly = %2, Ly_out = %3\n")
    .addI(Lx)
    .addI(Lx_out)
    .addI(Ly)
    .addI(Ly_out);

  std::vector<std::complex<float>> X(Lx_out);
  std::vector<std::complex<float>> xt(Lx_out);  
  std::vector<std::complex<float>> Y(Ly_out);
  std::vector<std::complex<float>> yt(Ly_out);  
  
  // load the input vector. 
  double ang = 0.0;
  double ang_inc = 2.0 * M_PI * freq / FS_in; 
  for(int i = num_passes; i < Lx; i++) {
    x[i] = std::complex<float>(std::cos(ang), std::sin(ang));
    ang = ang + ang_inc;
    if(ang > M_PI) ang = ang - 2.0 * M_PI;
    if(ang < M_PI) ang = ang + 2.0 * M_PI; 
  }

  // resample the input vector
  auto ibsize = resamp.getInputBufferSize();
  auto obsize = resamp.getOutputBufferSize();

  std::cerr << SoDa::Format("X size %0 ibsize %1 Y size %2 obsize %3\n")
    .addI(x.size())
    .addI(ibsize)
    .addI(y.size())
    .addI(obsize);

  std::vector<std::complex<float>> xx(ibsize);
  for(uint32_t i = 0; i < num_passes; i++) {
    auto iidx = i * ibsize;
    auto oidx = i * obsize;

    copyVec(xx, 0, x, iidx, ibsize);
    std::vector<std::complex<float>> yy(obsize);    

    try {
      resamp.apply(xx, yy);
    }
    catch (SoDa::Radio::Exception & e) {
      std::cerr << "ReSampler apply -- " << e.what() << "\n";
      throw e;
    }
    // now copy the yy vector to the output
    if(i > 0) {
      copyVec(yt, oidx - obsize, yy);
      copyVec(xt, iidx - ibsize, xx);
    }
  }

  // now compare the two streams.
  // these will be really big vectors, so the FFT is
  // going to be costly. 
  SoDa::FFT in_fft(Lx_out);
  SoDa::FFT out_fft(Ly_out);

  try {
    in_fft.fft(xt, X);
  }
  catch (SoDa::Radio::Exception & e) {
    std::cerr << "FFT Test X -- " << e.what() << "\n";
    throw e;
  }
  
  try {
    out_fft.fft(yt, Y);
  }
  catch (SoDa::Radio::Exception & e) {
    std::cerr << "FFT Test Y -- " << e.what() << "\n";
    throw e;
  }

  float xscale = 1.0 / float(X.size());
  float yscale = 1.0 / float(Y.size());
  for(auto & bx : X) {
    bx = bx * xscale; 
  }
  for(auto & by : Y) {
    by = by * yscale; 
  }
  
  double cutoff = std::min(FS_in, FS_out) * 0.4;
  double stopband = std::min(FS_in, FS_out) * 0.5;

  double out_freq = 0.0;
  double out_incr = 0.5 * FS_out / ((double) Ly_out);

  uint32_t extended_oob_plus_count = 0;
  uint32_t extended_oob_minus_count = 0;  
  uint32_t oob_plus_count = 0;
  uint32_t oob_minus_count = 0;  
  uint32_t ib_plus_count = 0;
  uint32_t ib_minus_count = 0;  
  uint32_t tr_plus_count = 0;
  uint32_t tr_minus_count = 0;  

  bool passed = true;

  // need to change this test to do a windowed periodogram over the two vectors.
   the periodogram needs to overlap by 50% to catch start/end transitions.
  // but we need a window to get rid of the transition artifact at the start and
  // end of the test. 

  if(is_in_band) {
    // The test frequency is within the nyquist region for both the input and output streams
    // check to see if the response in the passband is +/- 1dB and outside the passband < -60 dB ? 
    for(int i = 0; i < std::max(Lx_out, Ly_out) / 2; i++) {
      if(i >= (Ly_out / 2)) {
	// we are certainly out of band. -- we can leave now
	break; 
      }

      if(i >= (Lx_out / 2)) {
	// we are certainly outside the passband for the input filter.
	// make sure that we're below the stop band limit
	auto power_plus = toDb(Y[i]);
	auto power_minus = toDb(Y[Ly_out - 1 - i]);
	if(power_plus > stop_band_limit) {
	  extended_oob_plus_count++;
	  passed = false;
	}
	if(power_minus > stop_band_limit) {
	  extended_oob_minus_count++;
	  passed = false;	
	}

	if(!passed) {
	  std::cerr << SoDa::Format("doTestFreq failed stop band test i > Lx_out/2 i %0\n")
	    .addI(i);
	  
	  std::ofstream of(SoDa::Format("%0_%1_test.dat").addI(U).addI(D).str());  	
	  for(int i = 0; i < std::min(X.size(), Y.size()); i++) {
	    of << SoDa::Format("%0 %1 %2 %3 %4\n")
	      .addI(i)
	      .addF(toDb(Y[i]))
	      .addF(std::abs(Y[i]))
	      .addF(toDb(Y[Ly_out - 1 - i]))
	      .addF(std::abs(Y[Ly_out - 1 - i]));
	  }
	  of.close();
	  std::ofstream ofy(SoDa::Format("%0_%1_test_y.dat").addI(U).addI(D).str());  	
	  for(int i = 0; i < yt.size(); i++) {
	    ofy << SoDa::Format("%0 %1 %2\n")
	      .addI(i)
	      .addF(yt[i].real())
	      .addF(yt[i].imag());
	  }
	  ofy.close();
	  exit(-1);
	}
	
      }
      else if(i < (Lx_out / 2)) {
	// we may be within the nyquist region. 
	auto ratio_plus = toDb(Y[i], X[i]);
	auto ratio_minus = toDb(Y[Ly_out - 1 - i], X[Lx_out - 1 -i]);

	auto max_power_p = std::max(toDb(Y[i]), toDb(X[i]));
	auto max_power_m = std::max(toDb(Y[Ly_out - 1 - i]), toDb(X[Lx_out - 1 - i]));
      
	// if neither the input or the output has energy in this bin above -80 dB, then
	// any result for this case is acceptable
	bool too_low_to_care_p = max_power_p < -80.0;
	bool too_low_to_care_m = max_power_m < -80.0;      

	if(out_freq > stopband) {
	  // this bucket is in the stopband.  	  
	  if((ratio_plus > stop_band_limit) && !too_low_to_care_p) {
	    oob_plus_count++;
	    std::cerr << SoDa::Format("doTestFreq fail %0 ratio %1 limit %2 freq %3 i %4\n")
	      .addS("SB+").addF(ratio_plus).addF(stop_band_limit).addF(out_freq, 'e').addI(i);
	    passed = false;	  
	  }
	  if((ratio_minus > stop_band_limit) && !too_low_to_care_m)  {
	    oob_minus_count++;
	    std::cerr << SoDa::Format("doTestFreq fail %0 ratio %1 limit %2 freq %3 i %4\n")
	      .addS("SB-").addF(ratio_minus).addF(stop_band_limit).addF(out_freq, 'e').addI(i);
	    passed = false;	  
	  }
	}
	else if(out_freq < cutoff) {
	  // this bucket is within the nyquist region. 
	  if((std::abs(ratio_plus) < in_band_limit)  && !too_low_to_care_p)  {
	    ib_plus_count++;
	    std::cerr << SoDa::Format("doTestFreq fail %0 ratio %1 limit %2 freq %3 i %4\n")
	      .addS("IB+").addF(ratio_plus).addF(in_band_limit).addF(out_freq, 'e').addI(i);
	    passed = false;	  
	  }
	  if((std::abs(ratio_minus) < in_band_limit)  && !too_low_to_care_m) {
	    ib_minus_count++;
	    std::cerr << SoDa::Format("doTestFreq fail %0 ratio %1 limit %2 freq %3 i %4\n")
	      .addS("IB-").addF(ratio_minus).addF(in_band_limit).addF(out_freq, 'e').addI(i);
	    passed = false;
	  }
	}
	else {
	  // this bucket is in the transition region.  Any ratio < 1.0 dB is OK. 
	  // we're below the sampling cutoff
	  if((ratio_plus < in_band_limit)  && !too_low_to_care_p)  {
	    tr_plus_count++;
	    std::cerr << SoDa::Format("doTestFreq fail %0 ratio %1 limit %2 freq %3 i %4\n")
	      .addS("TR+").addF(ratio_plus).addF(1.0).addF(out_freq, 'e').addI(i);
	    passed = false;	
	  }
	  if((ratio_minus < in_band_limit)  && !too_low_to_care_m) {
	    tr_minus_count++;
	    std::cerr << SoDa::Format("doTestFreq fail %0 ratio %1 limit %2 freq %3 i %4\n")
	      .addS("TR-").addF(ratio_minus).addF(1.0).addF(out_freq, 'e').addI(i);
	    passed = false;		  
	  }
	}
	out_freq += out_incr;
      }
    }
  }
  else {
    // the input frequency is outside the nyquist limit for both the input and output.
    // so the output spectrum should have no output buckets > stop_band_limit
    for(int i = 0; i < Ly_out; i++) {
      if(toDb(Y[i]) > stop_band_limit) {
	std::cerr << SoDa::Format("doTestFreq out-of-band fail. i = %0 dB(Y[i]) = %1\n")
	  .addI(i)
	  .addF(toDb(Y[i])); 
	passed = false; 
      }
    }
    if(!passed) {
      std::ofstream of(SoDa::Format("%0_%1_test.dat").addI(U).addI(D).str());  	
      for(int i = 0; i < std::min(X.size(), Y.size()); i++) {
	of << SoDa::Format("%0 %1 %2 %3 %4 %5 %6 %7\n")
	  .addF(toDb(X[i]))
	  .addF(toDb(Y[i]))
	  .addF(std::abs(X[i]))
	  .addF(std::abs(Y[i]))
	  .addF(toDb(X[Lx_out - 1 - i]))
	  .addF(toDb(Y[Ly_out - 1 - i]))
	  .addF(std::abs(X[Lx_out - 1 - i]))
	  .addF(std::abs(Y[Ly_out - 1 - i]));
      }
      of.close();
      std::ofstream ofx(SoDa::Format("%0_%1_test_x.dat").addI(U).addI(D).str());  	
      for(int i = 0; i < xt.size(); i++) {
	ofx << SoDa::Format("%0 %1 %2\n")
	  .addI(i)
	  .addF(xt[i].real())
	  .addF(xt[i].imag());
      }
      ofx.close();
      std::ofstream ofy(SoDa::Format("%0_%1_test_y.dat").addI(U).addI(D).str());  	
      for(int i = 0; i < yt.size(); i++) {
	ofy << SoDa::Format("%0 %1 %2\n")
	  .addI(i)
	  .addF(yt[i].real())
	  .addF(yt[i].imag());
      }
      ofy.close();
      exit(-1);
    }
  }
  std::cerr << "doTestFreq:: leaving passed = " << ((char) (passed ? 'T' : 'F')) << "\n";
  return passed; 
}

bool doTest(float FS_in, float FS_out) {
  // create the resampler
  std::cerr << "doTest:: Creating new Resampler (" << FS_in << "->" << FS_out << "\n";
  try {
    SoDa::ReSampler resamp(FS_in, FS_out, 0.01);
    std::cerr << "doTest:: Created new resampler\n";
    // sweep through the frequences from -f to +f where f*2 = FS_in;
    double f_incr = FS_in * 0.1; // 0.001; 
    bool pass = true;
    for(double freq = - FS_in / 2; freq < FS_in / 2; freq += f_incr) {
      std::cerr << "Testing freq = " << freq << "\n";
      pass = pass & doTestFreq(resamp, freq, FS_in, FS_out, 
			       -1.0, -60, 10);
      std::cerr << "doTest: doTestFreq returned " << ((char) (pass ? 'T' : 'F')) << "\n";
    }
    std::cerr << "doTest leaving, pass = " << ((char) (pass ? 'T' : 'F')) << "\n";      
    return pass; 
  }
  catch (SoDa::Radio::Exception & e) {
    std::cerr << "doTest -- " << e.what() << "\n";
    throw e;
  }
}

void justCreate(float FS_in, float FS_out) {
  std::cerr << "justCreate:: creating sampler\n";  
  SoDa::ReSampler rs(FS_in, FS_out, 0.01);
  std::cerr << SoDa::Format("justCreate:: FS_in %4 FS_out %5insize %0 outsize %1 U %2 D %3\n")
    .addI(rs.getInputBufferSize())
    .addI(rs.getOutputBufferSize())
    .addI(rs.getU())
    .addI(rs.getD())
    .addF(FS_in, 'e')
    .addF(FS_out, 'e');
}
int main() {
  // create a resampler
  std::list<float> samp_rates = { 625000, 120000, 1200000, 5000000, 96000, 48000 };

  bool passed = true; 
  for(auto in_s : samp_rates) {
    std::cerr << "in_s = " << in_s << "\n";
    for(auto out_s : samp_rates) {
      if(uint32_t(in_s) == uint32_t(out_s)) continue;
      std::cerr << "out_s = " << out_s << "\n";
      justCreate(in_s, out_s);
      passed = passed & doTest(in_s, out_s); 
      std::cerr << "main: doTest returned " << ((char) (passed ? 'T' : 'F')) << "\n";
    }
  }
  
  if(passed) std::cout << "PASSED\n";
  else std::cout << "FAILED\n";
  
  return 0; 
}

