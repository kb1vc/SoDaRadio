#include "../src/Periodogram.hxx"
#include <cmath>
#include <iostream>
#include <fstream>
#include <complex>
#include <vector>

#include <SoDa/Format.hxx>
#include <SoDa/Options.hxx>

typedef std::vector<std::complex<float>> CVec;
typedef std::vector<float> FVec;

void makeSigVector(CVec & sig, double Fs, double Fr) {

  float ang = 0.0; 
  float anginc = (2.0 * Fr / Fs) * M_PI;
  std::cerr << SoDa::Format("Fr = %0 Fs = %1 anginc %2\n")
    .addF(Fr, 'e')
    .addF(Fs, 'e')
    .addF(anginc);
  
  for(auto & v : sig) {
    v = std::complex<float>(cos(ang), sin(ang)); 
    ang = ang + anginc;
    if(ang > M_PI) ang = ang - 2.0 * M_PI;
    if(ang < -M_PI) ang = ang + 2.0 * M_PI;    
  }
}


void makePDG(SoDa::Periodogram & pdg, CVec & pdg_in) {
  pdg.accumulate(pdg_in); 
}

void dumpVec(const std::string & msg, FVec v, double Fs, double Fr) {
  double freq_inc = Fs / double(v.size());
  
  std::cout << SoDa::Format("%0 Fs = %1 Fr = %2\n")
    .addS(msg)
    .addF(Fs, 'e')
    .addF(Fr, 'e');

  for(int i = 0; i < v.size(); i++) {
    std::cout << SoDa::Format("%0 %1\n").addF(double(i) * freq_inc - Fs / 2, 'e').addF(v[i]);
  }
}

bool checkPDG(SoDa::Periodogram & pdg, double Fs, double Fr) {
  FVec pdg_out;

  pdg.get(pdg_out);

  // what is the peak value?
  float peak_a = 0.0;
  for(auto v : pdg_out) {
    peak_a = std::max(peak_a, v); 
  }
  
  // convert to dB
  float lpeak_a = std::log10(peak_a);
  for(auto & v : pdg_out) {
    if(v > 0.0) {
      v = 20 * (std::log10(v) - lpeak_a); 
    }
    else {
      v = -120.0; 
    }
  }
  
  // peak and min
  float peak = -120.0;
  float min = 120.0;
  int peak_idx = 0; 
  int idx = 0; 
  for(auto v : pdg_out) {
    if(v > peak) {
      peak = v; 
      peak_idx = idx; 
    }
    min = std::min(min, v);
    idx++; 
  }

  // how many distinctive peaks?
  enum State { STOPBAND, SKIRT_RISE, PASSBAND, SKIRT_FALL, SEEN_PASSBAND };
  State state = STOPBAND; 
  
  float stopband_min = -40.0; // anything above is not in the stopband
  float passband_min = -3.0; // corners
  // can we figure out the stopband? 
  idx = 0; 
  int fall_idx = 0;
  int lo_corner_idx, hi_corner_idx;
  lo_corner_idx = hi_corner_idx = 0; 

  for(auto v : pdg_out) {
    switch(state) {
    case STOPBAND: 
      if (v > stopband_min) {
	state = SKIRT_RISE;
	stopband_min = v;
      }
      break; 
    case SKIRT_RISE:
      if (v < (stopband_min - 5.0)) {
	// we dropped back out of the stopband 
	state = STOPBAND; 
      }
      else if(v > passband_min) {
	state = PASSBAND; 
	lo_corner_idx = idx; 
      }
      break; 
    case PASSBAND:
      if(v < passband_min) {
	state = SKIRT_FALL; 
	hi_corner_idx = idx; 
      }
      break; 
    case SKIRT_FALL:
      if(v < stopband_min) {
	state = SEEN_PASSBAND; 
      }
      break; 
    case SEEN_PASSBAND: 
      if(v > stopband_min + 2.0) {
	// this is bad -- we should be in the stopband
	// but we've got a peak. 
	dumpVec("Blip after the passband", pdg_out, Fs, Fr);
	return false; 
      }
      break; 
    }
    idx++; 
  }

  double peak_freq = double(peak_idx) * Fs / double(pdg_out.size()) - Fs/2; 
  double bucket_width = Fs / double(pdg_out.size());
  double lo_freq = double(lo_corner_idx) * bucket_width - Fs/2;
  double hi_freq = double(hi_corner_idx) * bucket_width - Fs/2;   

  // is the target frequency between the low and high corners
  if((Fr < lo_freq) || (Fr > hi_freq)) {
    dumpVec(SoDa::Format("Target freq %0 outside of peak envelope %1 to %2\n")
	    .addF(Fr, 'e')
	    .addF(lo_freq, 'e')
	    .addF(hi_freq, 'e')
	    .str(), pdg_out, Fs, Fr); 
    return false; 
    
  }

  return (state == SEEN_PASSBAND); 
}

int main(int argc, char ** argv) {
  SoDa::Options cmd;
  uint32_t pdg_size, vec_size;
  double Fs, Fr; 

  cmd.add<uint32_t>(&pdg_size, "psize", 'p', 1024, "Size of periodogram")
    .add<uint32_t>(&vec_size, "vsize", 'v', 64 * 1024, "Size of input stream")
    .add<double>(&Fs, "fsamp", 'S', 48e3, "Sample Rate")
    .add<double>(&Fr, "ftest", 'f', 5e3, "Test Frequency");

  if(!cmd.parse(argc, argv)) {
    std::cout << "FAIL - bad command options\n";
    exit(-1);
  }
  
  CVec pdg_in(vec_size); 

  SoDa::Periodogram pdg(pdg_size); 

  makeSigVector(pdg_in, Fs, Fr);

  makePDG(pdg, pdg_in);
  
  bool pass = checkPDG(pdg, Fs, Fr); 
  
  if(pass) {
    std::cout << "PASSED\n"; 
  }
  else {
    std::cout << "FAILED\n";
  }
}

