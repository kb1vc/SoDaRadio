/*
Copyright (c) 2012, Matthew H. Reilly (kb1vc)
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

#define SAMPLE_RATE 48000
#include <complex>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "OSFilter.hxx"
#include <time.h>
#include <fftw3.h>
#include <math.h>
#include <boost/format.hpp>

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "HilbertTransformer.hxx"

using namespace std;
#include <sys/time.h>

namespace SoDaTest {
class Histogram {
public:
  Histogram(unsigned int nb, float _min, float _max) {
    num_buckets = nb;
    min = _min;
    max = _max;
    float spread = _max - _min;
    bucket_size = (_max - _min) / ((float) num_buckets); 
    buckets = new unsigned int[num_buckets];
    underflow = 0; overflow = 0;
    unsigned int i;
    for(i = 0; i < num_buckets; i++) buckets[i] = 0; 
  }

  void record(float samp) {
    if(samp < min) underflow++;
    else if(samp > max) overflow++;
    else {
      unsigned int idx = (unsigned int) floor((samp - min) / bucket_size);
      buckets[idx]++; 
    }
  }

  void dump(ostream & os, const std::string & tag) {
    os << boost::format("%s %f %d\n") % tag % (min - bucket_size) % underflow;
    int i;
    for(i = 0; i < num_buckets; i++) {
      if(buckets[i] == 0) continue;
      float b = min + ((float) i) * bucket_size; 
      os << boost::format("%s %f %d\n") % tag % b % buckets[i]; 
    }
    os << boost::format("%s %f %d\n") % tag % (max + bucket_size) % overflow; 
  }

  unsigned int num_buckets;
  float min;
  float max;
  unsigned int underflow, overflow; 
  unsigned int * buckets;
  float bucket_size; 
}; 
}

double curtime()
{
  double ret;
  struct timeval tp; 
  gettimeofday(&tp, NULL); 
  ret = ((double) tp.tv_sec) + 1e-6*((double)tp.tv_usec);

  return ret; 
}




#define BUFLEN 1024
int dumpTest(int argc, char * argv[])
{
  SoDa::HilbertTransformer htR(BUFLEN);
  SoDa::HilbertTransformer htC(BUFLEN);
  float inbuf[BUFLEN];
  std::complex<float> cinbuf[BUFLEN];
  std::complex<float> outbuf[BUFLEN];
  std::complex<float> coutbuf[BUFLEN];
  std::complex<float> checkbuf[BUFLEN];
  
  int i, j, k;

  // load up the input buffer
  const int num_freqs = 8;
  float angles[num_freqs];
  float ang_incs[num_freqs];

  srandom(0x13255);
  

  for(i = 0; i < num_freqs; i++) {
    angles[i] = 0.0;
    long irand = random();
    ang_incs[i] = M_PI * ((float) (3.535 * ((irand % 8) + 15))) / (0.82 * ((float) (BUFLEN)));
  }

  float gain; 
  for(k = 0; k < 20; k++) {
    
    for(i = 0; i < BUFLEN; i++) {
      checkbuf[i] = std::complex<float>(0.0, 0.0);
      inbuf[i] = 0.0;
      cinbuf[i] = std::complex<float>(0.0, 0.0);
    }
    
    for(i = 0; i < BUFLEN; i++) {
      if(k < 3) {
	float x = ((float) (i + BUFLEN + k * BUFLEN)) / (4.0 * ((float) BUFLEN));
	gain = 0.5 * (1.0 - cos(M_PI * x)); 
      }
      else {
	gain = 1.0; 
      }
      for(j = 0; j < num_freqs; j++) {
	inbuf[i] += gain * sin(angles[j]);
	cinbuf[i] += std::complex<float>(gain * sin(angles[j]),0.0);
	checkbuf[i] += std::complex<float>(gain * sin(angles[j]), -1.0 * gain * cos(angles[j]));
	angles[j] += ang_incs[j];
	if(angles[j] > M_PI) angles[j] -= (2.0 * M_PI);
      }
    }

    // now do a hilbert transform.
    htR.apply(inbuf, outbuf, false, 1.0);
    htC.apply(cinbuf, coutbuf, true, 1.0);

    if(k > -1) {
      for(i = 0; i < BUFLEN; i++) {
	std::cout << boost::format("ZZZ: %d %f %f  %f %f %f %f\n")
	  % (i + k * BUFLEN) % outbuf[i].real() % outbuf[i].imag() % coutbuf[i].real() % coutbuf[i].imag() % checkbuf[i].real() % checkbuf[i].imag(); 
      }
    }
  }


  // Now do a modulation test...
  SoDa::HilbertTransformer htDemod(BUFLEN);
  gain = 0.0; 
  for(k = 0; k < 20; k++) {
    for(i = 0; i < BUFLEN; i++) {
      checkbuf[i] = std::complex<float>(0.0, 0.0);
      cinbuf[i] = std::complex<float>(0.0, 0.0);
    }
    
    for(i = 0; i < BUFLEN; i++) {
      if(k < 3) {
	float x = ((float) (i + BUFLEN + k * BUFLEN)) / (4.0 * ((float) BUFLEN));
	gain = 0.5 * (1.0 - cos(M_PI * x)); 
      }
      else {
	gain = 1.0; 
      }
      for(j = 0; j < num_freqs; j++) {
	cinbuf[i] += gain * std::complex<float>(sin(angles[j]), cos(angles[j]));
	// cinbuf[i].real() += gain * sin(angles[j]);
	// cinbuf[i].imag() += gain * cos(angles[j]);
	checkbuf[i] += gain * std::complex<float>(sin(angles[j]), -1.0 * cos(angles[j]));
	// checkbuf[i].real() = checkbuf[i].real() + gain * sin(angles[j]);
	// checkbuf[i].imag() = checkbuf[i].imag() - gain * cos(angles[j]);
	angles[j] += ang_incs[j];
	if(angles[j] > M_PI) angles[j] -= (2.0 * M_PI);
      }
    }

    // now do a hilbert transform.
    htDemod.applyIQ(cinbuf, outbuf, 1.0);

    // dump the output
    if(k > -1) {
      for(i = 0; i < BUFLEN; i++) {
	std::cout << boost::format("MMM: %d %f %f %f %f %f %f\n")
	  % (i + k * BUFLEN) % outbuf[i].real() % outbuf[i].imag() % cinbuf[i].real() % cinbuf[i].imag() % checkbuf[i].real() % checkbuf[i].imag(); 
      }
    }
  }
  return 0; 
}


/**
 *    Applying HT twice to a real valued signal f(t) will
 *    produce a delayed version of -1 * f(t + d) at its output.
 *    The HT module's complex result, should be (a + ib) = 
 *    f(t + d) + i * -1 * f(t + d).  Therefore, we should see
 *    that (a + b) == 0 and (a - b) = 2a.
 *
 *    In this test, we accumulate a histogram of the error
 *    magnitude (b + a) -- this should ideally be 0.0; 
 */
int doSSBTest(int argc, char * argv[])
{
  // create an HT for an audio buffer size of 2304
  // (This corresponds to the magic choice in Params.hxx.)
  int buflen = 2304;
  // make sure the HT impulse response is at least 256 samples long. 
  SoDa::HilbertTransformer HT_ui(buflen, 256);
  SoDa::HilbertTransformer HT_li(buflen, 256);
  SoDa::HilbertTransformer HT_usb(buflen, 256);
  SoDa::HilbertTransformer HT_lsb(buflen, 256);

  // create a test input buffer
  float inbuf[buflen];
  // output buffers
  std::complex<float> analytic_U_inbuf[buflen];
  std::complex<float> analytic_L_inbuf[buflen];
  std::complex<float> usb_outbuf[buflen];
  std::complex<float> lsb_outbuf[buflen];

  // now setup an input test signal parameters.
  // We're going to use a signal made up of eight
  // more or less random frequencies
  const int num_freqs = 16;
  float angles[num_freqs];
  float ang_incs[num_freqs];

  srandom(0x13255);

  int i, j, k; 
  for(i = 0; i < num_freqs; i++) {
    angles[i] = 0.0;
    long irand = random();
    // we want arg_inc (phase advance per step) to vary
    // over the range 0 to pi / 8 (0.125 * Fnyquist).
    float frand = ((float) (irand & 0xffff)) / (8.0 * 65536.0); 
    ang_incs[i] = frand * M_PI; 
  }

  ang_incs[0] = 0.01 * M_PI;
  
  // build the error histograms
  SoDaTest::Histogram usb_sum_err(1000, -0.2, 0.2); 
  SoDaTest::Histogram lsb_diff_err(1000, -0.2, 0.2); 
  
  // now do the test over 1000 buffers
  int trial_count = 10;
  int ignore_buffer_count = 1; // ignore the first buffer, or so to get over the startup transient. 
  for(k = 0; k < trial_count; k++) {
    // load the input buffer.
    for(j = 0; j < buflen; j++) {
      inbuf[j] = 0.0; 
    }
    for(i = 0; i < num_freqs; i++) {
      for(j = 0; j < buflen; j++) {
	inbuf[j] += sin(angles[i]);
	angles[i] += ang_incs[i]; 
      }
    }

    // perform the transform.
    HT_ui.apply(inbuf, analytic_U_inbuf, false, 1.0); 
    HT_li.apply(inbuf, analytic_L_inbuf, true, 1.0); 
    HT_usb.applyIQ(analytic_U_inbuf, usb_outbuf, 1.0); 
    HT_lsb.applyIQ(analytic_L_inbuf, lsb_outbuf, 1.0);

    
    // now calculate the errors
    if(k >= ignore_buffer_count) {
      float norm = 0.0;
      for(j = 0; j < buflen; j++) {
	norm += (inbuf[j] * inbuf[j]); 
      }
      norm = sqrt(norm); 
      for(j = 0; j < buflen; j++) {
	float usb_sum, lsb_diff;
	usb_sum = usb_outbuf[j].real() + usb_outbuf[j].imag();
	lsb_diff = lsb_outbuf[j].real() - lsb_outbuf[j].imag();

	usb_sum_err.record(usb_sum / norm); 
	lsb_diff_err.record(lsb_diff / norm);
#if 0
	std::cerr << boost::format("%d %f %f %f %f %f %f %f %f %f %f\n") %
	  (j + k * buflen) %
	  usb_sum % lsb_diff %
	  usb_outbuf[j].real() % usb_outbuf[j].imag() % 
	  lsb_outbuf[j].real() % lsb_outbuf[j].imag() %
	  analytic_U_inbuf[j].real() % analytic_U_inbuf[j].imag() %
	  analytic_L_inbuf[j].real() % analytic_L_inbuf[j].imag()
	  ;
#endif
      }
    }
  }

  std::cout << boost::format("# Dumps histograms of USB and LSB normalized error\n# -- all should be clustered around 0.0 -- normalized to mean squared amplitude.\n"); 
  usb_sum_err.dump(std::cout, std::string("USB: ")); 
  lsb_diff_err.dump(std::cout, std::string("LSB: ")); 

  return 1; 
}

/**
 * doPMTest -- run a Phase Modulated signal through a hilbert transform
 * shift to baseband, then demodulate.
 */
int doPMTest(int argc, char * argv[])
{
  // make a PM signal and find it. 
  // create an HT for an audio buffer size of 2304
  // (This corresponds to the magic choice in Params.hxx.)
  int buflen = 2304;
  // make sure the HT impulse response is at least 256 samples long. 
  SoDa::HilbertTransformer HT(buflen, 256);

  // create a test input modulating buffer
  float inbuf[buflen];
  // create the FM widget
  float rf_inbuf[buflen]; 

  // output buffers
  std::complex<float> transform_buf[buflen];

  // now setup an input test signal parameters.
  // We're going to use a signal made up of eight
  // more or less random frequencies
  const int num_freqs = 16;
  float angles[num_freqs];
  float ang_incs[num_freqs];

  srandom(0x13255);

  int i, j, k; 
  for(i = 0; i < num_freqs; i++) {
    angles[i] = 0.0;
    long irand = random();
    // we want arg_inc (phase advance per step) to vary
    // over the range 0 to pi / 64
    float frand = ((float) (irand & 0xffff)) / (64.0 * 65536.0); 
    ang_incs[i] = frand * M_PI; 
  }

  ang_incs[0] = 0.01 * M_PI;

  // what is the phase modulation shift? 
  float phase_incr = M_PI / 100.0;

  // what is the carrier frequency ?
  // make it fsamp / 4;
  float carrier_freq = M_PI / 20.0;
  float tx_carrier_angle = 0.0; 
  float rx_carrier_angle = 0.0; 
  
  // now do the test over 1000 buffers
  int trial_count = 10;
  int ignore_buffer_count = 1; // ignore the first buffer, or so to get over the startup transient. 
  for(k = 0; k < trial_count; k++) {
    // load the input buffer.
    for(j = 0; j < buflen; j++) {
      inbuf[j] = 0.0; 
    }
    for(i = 0; i < num_freqs; i++) {
      for(j = 0; j < buflen; j++) {
	inbuf[j] += sin(angles[i]);
	angles[i] += ang_incs[i]; 
      }
    }

    for(j = 0; j < buflen; j++) {
      rf_inbuf[j] = sin(tx_carrier_angle + inbuf[j] * phase_incr);
      tx_carrier_angle += carrier_freq; 
    }
    
    // perform the transform.
    HT.apply(rf_inbuf, transform_buf, false, 1.0); 

    // now shift down to baseband
    for(j = 0; j < buflen; j++) {
      std::complex<float> rx_carrier = std::complex<float>(sin(rx_carrier_angle), cos(rx_carrier_angle));
      rx_carrier_angle += carrier_freq;
      transform_buf[j] = transform_buf[j] * rx_carrier; 
    }
    
    // now calculate the errors
    if(k >= ignore_buffer_count) {
      float norm = 0.0;
      for(j = 0; j < buflen; j++) {
	norm += (inbuf[j] * inbuf[j]); 
      }
      norm = sqrt(norm); 
      for(j = 0; j < buflen; j++) {
	float demod;
	// demodulate the FM carrier
	demod = atan2(transform_buf[j].imag(), transform_buf[j].real()); 
#if 1
	std::cerr << boost::format("PM %d %f %f %f %f %f\n") %
	  (j + k * buflen) %
	  inbuf[j] %
	  demod %
	  rf_inbuf[j] %
	  transform_buf[j].real() % transform_buf[j].imag(); 
#endif
      }
    }
  }

  return 1; 
}

int main(int argc, char * argv[])
{
  // Test out the Hilbert Transformer module

  // Rather than doing a comparison with a reference,
  // the HT is tested out by ensuring that the transformer
  // produces results whose properties are those we expect
  // from a Hilbert transform network.
  //
  // 1. Applying HT twice to a real valued signal f(t) will
  //    produce a delayed version of -1 * f(t + d) at its output.
  //    The HT module's complex result, should be (a + ib) = 
  //    f(t + d) + i * -1 * f(t + d).  Therefore, we should see
  //    that (a + b) == 0 and (a - b) = 2a.
  //
  // 2. When HT(f(t)) -> a + ib, applying HT to a phase 
  //    modulated signal f(t) = sin(\omega t + \phi(t))
  //    should recover \phi(t) = atan(b/a). 
  //

  doSSBTest(argc, argv);

  doPMTest(argc, argv);
}

