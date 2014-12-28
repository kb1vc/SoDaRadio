/*
  Copyright (c) 2014, Matthew H. Reilly (kb1vc)
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in
  the documentation and/or other materials provided with the
  distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 * @file USRP2_Cordic_Model
 *
 * @brief a model of the 24 bit CORDIC oscillator in the 2nd generation USRP
 *
 * Taken from the sdr_lib/cordic_z24.v file and translated into C.  I did this
 * to do some fast searching of the tuning space for good and bad oscillator forms. 
 *
 * @author Matt Reilly (kb1vc)
 */

#include <iostream>
#include <fstream>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <math.h>
#include <complex>

#include "Spectrogram.hxx"
#include "DCBlock.hxx"

#define I24SIGN 0x800000
#define I25SIGN 0x1000000
#define I27SIGN 0x4000000

class Parms {
public:
  Parms(int argc, char * argv[]) {
    namespace po = boost::program_options;
    po::options_description desc("Options");

    desc.add_options()
      ("help", "print this message")
      ("surf", po::value<bool>(&en_surface_plot)->default_value(false),
       "Enable production of a surface plot on <basename>.dat")
      ("basename", po::value<std::string>(&basename)->default_value(std::string("cordic_test")),
       "Basename for output files <basename>.dat and <basename>.stat")
      ("min", po::value<float>(&min_freq)->default_value(-20.0e6),
       "minimum frequency setting for DDC CORDIC")
      ("max", po::value<float>(&max_freq)->default_value(20.0e6),
       "maximum frequency setting for DDC CORDIC")
      ("incr", po::value<float>(&freq_incr)->default_value(10.0e3),
       "CORDIC oscillator setting stepsize")
      ("corr", po::value<bool>(&corr_ena)->default_value(true),
       "Enable correlation test -- better resolution near the carrier.")
      ("spect", po::value<bool>(&spect_ena)->default_value(false),
       "Enable spectrum test -- lotsa bits, useful insight.")
      ("loglen", po::value<unsigned int>(&log_samp_len)->default_value(15),
       "Log2 of sample vector length -- must be >= 15")
      ;

    
    po::variables_map vm;

    po::store(po::parse_command_line(argc, argv, desc), vm);

    po::notify(vm);
    
    if(vm.count("help")) {
      std::cout << "USRP2 Cordic Test Experiment" << std::endl
		<< desc << std::endl;
      exit(-1); 
    }

    if(log_samp_len < 15) log_samp_len = 15; 
    char espc = en_surface_plot ? 'T' : 'F'; 
    char cec = corr_ena ? 'T' : 'F'; 
    char sec = spect_ena ? 'T' : 'F'; 
    std::cerr << boost::format("P: basename [%s] max_freq %g freq_incr %g surf_plot %c corr %c spec %c log_samp_len %d\n") % basename % max_freq % freq_incr % espc % cec % sec % log_samp_len; 
  }

public:
  std::string basename; 
  bool en_surface_plot;
  bool corr_ena; 
  bool spect_ena; 
  float min_freq;
  float max_freq;
  float freq_incr;
  unsigned int log_samp_len; 
}; 

class CORDIC_Z24 {
public:
  CORDIC_Z24() {
    // initialize the C table.
    C[0] = 2097152;
    C[1] = 1238021;
    C[2] = 654136;
    C[3] = 332050;
    C[4] = 166669;
    C[5] = 83416;
    C[6] = 41718;
    C[7] = 20860;
    C[8] = 10430;
    C[9] = 5215;
    C[10] = 2608;
    C[11] = 1304;
    C[12] = 652;
    C[13] = 326;
    C[14] = 163;
    C[15] = 81;
    C[16] = 41;
    C[17] = 20;
    C[18] = 10;
    C[19] = 5;
    C[20] = 3;
    C[21] = 1;
    C[22] = 1;
    C[23] = 0;

    init();
  }

  void init() {
    x[0] = 0;
    y[0] = 0;
    z[0] = 0; 
  }

  // Parameters for the actual instantiation
  // in DDC cwidth = 25   zwidth = 24
  // 
  // BITWIDTH 25 -- for xi, xo, yi, yo
  // ZWIDTH 24 -- for zi, zo
  // x[0]..x[20] and y[0]..y[20] are 27 bits wide (top two are quadrant bits?)
  // z[0]..z[20] are 23 bits wide
  // xi/xo is input/output in-phase
  // yi/yo is input/output quadrature
  // zi/zo is the phase (24 bit counter)


  void doStep(int xi, int yi, // 25 bits
	      int zi, // 24 bits
	      int & xo, int & yo, // 25 bits
	      int & zo // 24 bits
	      ) __attribute__ ((noinline)) {
    int xi_ext = sign_extend(xi, 25);
    int yi_ext = sign_extend(yi, 25);

    z[0] = zi & 0x3fffff;
    int quad = (zi >> 22) & 3;
    // std::cerr << boost::format("angle = 0x%x quad = %x\n") % zi % quad;
    // top two bits of the phase input identify the quadrant
    switch (quad) {
    case 0:
      x[0] = xi_ext;
      y[0] = yi_ext;
      break;
    case 3:
      x[0] = yi_ext;
      y[0] = -xi_ext; 
      break;
    case 1:
      x[0] = yi_ext;  // -xi, -yi  yi,xi
      y[0] = xi_ext;
      break;
    case 2:
      x[0] = -xi_ext; 
      y[0] = yi_ext;
      break; 
    }

    int i; 
    for(i = 0; i < 20; i++) {
      cordic_stage(x[i], y[i], z[i],
		   x[i+1], y[i+1], z[i+1], 
		   i, C[i]); 
    }

    xo = (sign_extend(x[20], 25) >> 1);
    yo = (sign_extend(y[20], 25) >> 1);
    zo = sign_extend(z[20], 23);
  }

  void test_signext() {
    int w = 16;
    int i;
    for(i = 1; i != 0; i = i << 1) {
      int j = sign_extend(i, w);
      std::cerr << boost::format("SignExt i = %d 0x%x  w = %d  j = %d 0x%x\n")
	% i % i % w % j % j;
    }
  }
  
private:

  int sign_extend(int v, int width) {
    int mask = (1 << (width - 1));
    if(v & mask) {
      v = v | (0xffffffff << (width - 1)); 
    }
    else {
      v = v & ~(0xffffffff << (width - 1)); 
    }

    return v; 
  }
  


  // bitwidth = 27 for x, y, z
  // zwidth = 23
  void cordic_stage(int xi, int yi, // 27 bits
		    int zi,  // 23 bits
		    int & xo, int & yo, // 27 bits
		    int & zo, // 23  bits
		    int shift, int constant) __attribute__ ((noinline)) {


    xi = sign_extend(xi, 27); 
    yi = sign_extend(yi, 27); 
    zi = sign_extend(zi, 23);
    
    if(zi > 0) {
      xo = xi - (yi >> shift); 
      yo = yi + (xi >> shift);
      zo = zi - constant;
      // std::cerr << "zpos\t";
    }
    else {
      xo = xi + (yi >> shift); 
      yo = yi - (xi >> shift); 
      zo = zi + constant; 
      // std::cerr << "zneg\t";
    }

#if 0    
    xo = xo & 0x3ffffff;
    yo = yo & 0x3ffffff;
    zo = zo & 0x7fffff; 
#endif 
    
    float zoang = 180.0 * ((float) sign_extend(zo, 23)) / ((float) (1 << 22));
    
      
    // std::cerr << boost::format("z0ang %f zi/zo = 0x%08x 0x%08x  xi/xo = 0x%08x 0x%08x  yi/yo = 0x%08x 0x%08x\n")
    //        % zoang % zi % zo % xi % xo % yi % yo; 

    return; 
  }
  
  int C[24];
  int x[21], y[21]; // 18 bits wide
  int z[21]; // 23 bits wide

}; 

class CorrTest {
public:
  CorrTest(int inlen, const std::string basename) {
    dc_close = new   SoDa::DCBlock<std::complex<double>, double>(0.999);
    dc_far = new   SoDa::DCBlock<std::complex<double>, double>(0.9999);

    std::string cfname = basename + std::string(".cdat");
    cf.open(cfname.c_str());

    dv = new std::complex<double>[inlen]; 
    dv_close_out = new std::complex<double>[inlen]; 
    dv_far_out = new std::complex<double>[inlen]; 
  }

  ~CorrTest() {
    cf.close(); 
  }

  void apply(float fr, int phase_add, std::complex<double> * v, int inlen) __attribute__ ((noinline)) {
    int i, j;

    // build the reference oscillator..
    std::complex<double> ref_osc;
    double ang = 0.0;
    double d_phase_add = M_PI * fr / 50.0e6; 

    // We're looking at a synthesized oscillator whose output is
    // less than ideal.  It is exp(-j * theta) + junk.
    // beat it against an "ideal" oscillator at exp(j*theta) to
    // eliminate all but the "junk" and then measure the power
    // that remains. 
    
    for(i = 0; i < inlen; i++) {
      // This beats the test oscillator against a reference osc
      // that is opposite in frequency -- producing a shift to DC
      // and placing all the phase noise on either side of the
      // 0 frequency line
      ref_osc = std::complex<double>(cos(ang), sin(ang));
      // run the ref osc at the complimentary frequency to get
      // exp(-j * 
      ang = ang - d_phase_add;
      if(ang < (-1.0 * M_PI)) ang += 2.0 * M_PI;
      if(ang > M_PI) ang -= 2.0 * M_PI;
      
      dv[i] = v[i] * ref_osc;
    }

    // Filter out the DC component. 
    dc_close->apply(dv, dv_close_out, inlen);
    dc_far->apply(dv, dv_far_out, inlen);

    // now calculate the RMS value
    double rms_cl = 0.0;
    double rms_fa = 0.0;
    
    // skip the first 10K points
    for(i = 10000, j = 0; i < inlen; i++, j++) {
      double re, im;
      re = dv_close_out[i].real();
      im = dv_close_out[i].imag();
      rms_cl += (re*re + im*im); 
      re = dv_far_out[i].real();
      im = dv_far_out[i].imag();
      rms_fa += (re*re + im*im); 
    }

    rms_cl = rms_cl / ((double) j); 
    rms_fa = rms_fa / ((double) j); 
    cf << boost::format("%12.9g %d %g %g\n") % fr % phase_add % rms_cl % rms_fa;
    
  }

  std::ofstream cf; 
  SoDa::DCBlock<std::complex<double>, double> * dc_close;
  SoDa::DCBlock<std::complex<double>, double> * dc_far;
  std::complex<double> * dv; 
  std::complex<double> * dv_close_out; 
  std::complex<double> * dv_far_out; 
};

class SpectrumTest {
public:
  SpectrumTest(unsigned int _len, float fmax, const std::string outfname_base, bool _enable_surfplot = false) {
    len = _len;
    enable_surfplot = _enable_surfplot;

    std::string datfile = outfname_base + std::string(".dat");
    std::string statfile = outfname_base + std::string(".stat");
    if (enable_surfplot) {
      of.open(datfile.c_str(), std::ios::out);
    }
    sf.open(statfile.c_str(), std::ios::out);

    of << "# CORDIC_FREQ  SPEC_FREQ POWER(dB)" << std::endl;
    sf << "# CORDIC_FREQ  near_in_avg_power(dB) far_avg_power(dB)  " << std::endl;
    freq_max = fmax;
    
    fftbuf = new float[len]; 
    
    spec = new SoDa::Spectrogram(len); 
  }

  ~SpectrumTest() {
    if (enable_surfplot) {
      of.close();
    }
    sf.close();
  }

  
			       
  void apply(float fr, int phase_add, std::complex<float> * v, int inlen) __attribute__ ((noinline)) {
    // do the fft
    spec->apply_acc(v, inlen, fftbuf, 0.0);
    
    // scan the fft;
    // negative freqs first
    float ffr = - freq_max;
    float ffi = 2.0 * freq_max / ((float) len);
    int i;
    float max_lev = -1.0e4;
    float max_freq = 0.0;
    int max_idx = 0;
    float vmag[len]; 
    for(i = 0; i < len; i++) {
      float mmag = 10.0 * log10(fftbuf[i]);
      vmag[i] = mmag;
      if(mmag > max_lev) {
	max_lev = mmag;
	max_freq = ffr;
	max_idx = i; 
      }
      if (enable_surfplot) {      
	of << boost::format("%12.9g %g %g\n")
	  % fr % ffr % mmag;
      }
      ffr += ffi; 
    }

    // now rescan and find the average level in the near portion (+/- 10 buckets)
    // and far portion
    int ni_count = 0;
    float near = 0.0;
    int fi_count = 0;
    float far = 0.0; 
    for(i = 0; i < len; i++) {
      if(abs(i - max_idx) < 10) {
	near += (vmag[i] - max_lev);
	ni_count++; 
      }
      else {
	far += (vmag[i] - max_lev);
	fi_count++; 
      }
    }

    far = far / ((float) fi_count);
    near = near / ((float) ni_count);
		 
    sf << boost::format("%g %d %g %g\n") 
      % fr % phase_add % near % far; 

  }

  unsigned int len;
  std::ofstream of, sf;
  float * fftbuf;
  float freq_max; 
  bool enable_surfplot; 
  SoDa::Spectrogram * spec; 
};

int main(int argc, char * argv[])
{
  Parms params(argc, argv);
  
  CORDIC_Z24 cord;

  // cord.test_signext();

  int phase = 0x0;
  int phase_add = 0x10000;

  int xi, yi, xo, yo, zi, zo;

  yi = 0;
  xi = 0x800000; // works 0x800000;
  xi = 0x9b74e0;
  xi = 0x9b7000;
  // xi = xi >> 1; 

  unsigned int samp_vec_len = 1 << params.log_samp_len; // 1024 * 32; 
  unsigned int fft_len = samp_vec_len >> 1; 
  std::complex<float> svec[samp_vec_len];
  std::complex<double> dvec[samp_vec_len];
  
  SpectrumTest * spec_test;
  if(params.spect_ena) {
    spec_test = new SpectrumTest(fft_len, 50.0e6, params.basename, params.en_surface_plot);
  }

  CorrTest * corr_test;
  if(params.corr_ena) {
    corr_test = new CorrTest(samp_vec_len, params.basename);
  }
  
  // Let's sweep from -20MHz to 20MHz and see what kind of quality we've got.
  int sampcount = 0; 
  // for(float freq = -20.0e6; freq < 20.05e6; freq += 100e3) {
  std::cerr << boost::format("min_freq = %g  max_freq = %g  freq_incr = %g\n")
    % params.min_freq % params.max_freq % params.freq_incr;
  
  for(float freq = params.min_freq; freq <= params.max_freq; freq += params.freq_incr) {
    // for(float freq = -2.0e6; freq < 2.15e6; freq += 100e3) {
    // The first conversion runs at 100 MHz?  I think...
    double dff = ((double) freq) / 100.0e6;
    long int lpa = (long int) floor(dff * ((double) 0x7fffffff));
    phase_add = (int) (lpa >> 32L);
    int opa = (int) floor(freq * ((float) 0x7fffff) / 100e6);
    std::cout << boost::format("# phase_add %d 0x%x   old %d 0x%x\n")
      % phase_add % phase_add % opa % opa; 
    // was    phase_add = (int) floor(freq * ((float) 0x) / 100e6); 
    for(int samps = 0; samps < samp_vec_len; samps++) {
      // phase input to cordic is top 24 bits of the phase accumulator
      int cord_phase = phase >> 8; 
      cord.doStep(xi, yi, phase, xo, yo, zo);
      if(params.spect_ena) {
	float msin = ((float) yo) / ((float) 0x7fffff);
	float mcos = ((float) xo) / ((float) 0x7fffff);
	std::cout << boost::format("%d %d %d %d %d %d  %lg %lg\n")
	  % sampcount % phase % xi % yi % xo % yo % msin % mcos;
	svec[samps] = std::complex<float>(mcos, msin);
      }

      if(params.corr_ena) {
	double dmsin = ((double) yo) / ((double) 0x7fffff);
	double dmcos = ((double) xo) / ((double) 0x7fffff);
	dvec[samps] = std::complex<double>(dmcos, dmsin);
      }
      
      phase += phase_add;
      sampcount++;
    }
    exit(-1); 
    std::cout << boost::format("%12.9g\n") % freq;
    // now test the vector
    if(params.spect_ena) {
      spec_test->apply(freq, phase_add, svec, samp_vec_len);
    }
    if(params.corr_ena) {
      corr_test->apply(freq, phase_add, dvec, samp_vec_len); 
    }
  }
}
		 
