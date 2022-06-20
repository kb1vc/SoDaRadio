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
#include "Spectrogram.hxx"
#include <math.h>
#include <iostream>
#include "SoDaBase.hxx"
#include <SoDa/Format.hxx>

SoDa::Spectrogram::Spectrogram(unsigned int fftlen)
  : SoDa::Base("Spectrogram")
{
  // remember how long the output will be
  fft_len = fftlen; 

  // allocate the internal buffers
  win_samp = (std::complex<float>*) fftwf_alloc_complex(fft_len);
  fft_out = (std::complex<float>*) fftwf_alloc_complex(fft_len);
  result = new float[fft_len];

  // create the FFT plan
  fftplan = fftwf_plan_dft_1d(fft_len,
			      (fftwf_complex *) win_samp,
			      (fftwf_complex *) fft_out,
			      FFTW_FORWARD, FFTW_ESTIMATE); 
  
  // setup the blackman harris window.
  window = initBlackmanHarris(); 
}


float * SoDa::Spectrogram::initBlackmanHarris()
{
  unsigned int i;
  float a0 = 0.35875;
  float a1 = 0.48829;
  float a2 = 0.14128;
  float a3 = 0.01168;

  float * w = new float[fft_len];
  float anginc = 2.0 * M_PI / ((float) fft_len - 1);
  float ang = 0.0; 
  for(i = 0; i < fft_len; i++) {
    ang = anginc * ((float) i); 
    w[i] = a0 - a1 * cos(ang) + a2 * cos(2.0 * ang) -a3 * cos(3.0 * ang); 
  }

  return w;
}

void SoDa::Spectrogram::apply_common(std::complex<float> * invec,
				     unsigned int inveclen)
{
  unsigned int i, j;
  float repl_count;

  repl_count = floor(((float) inveclen) / ((float) fft_len));
  float gain_adj = 1.0 / repl_count; 

  // zero the result accumulate buffer. 
  for(j = 0; j < fft_len; j++) result[j] = 0.0;
  if(fft_len > inveclen) {
    throw SoDa::Radio::Exception(SoDa::Format("inveclen %0 less than fftlen %1\n") 
			  .addI(inveclen)
			  .addI(fft_len), 
			  this);
  }
  // accumulate FFT results over the length of the buffer.
  for(i = 0; i < (inveclen + 1 - fft_len); i += (fft_len / 2)) {
    std::complex<float> *v = &(invec[i]);

    // window the input
    for(j = 0; j < fft_len; j++) {
      win_samp[j] = v[j] * window[j]; 
    }  

    // do the fft
    fftwf_execute(fftplan); 

    // accumulate the magnitude squared result
    for(j = 0; j < fft_len; j++) {
      float re, im;
      re = fft_out[j].real();
      im = fft_out[j].imag();
      result[j] += gain_adj * (re * re + im * im); 
    }
  }
}


void SoDa::Spectrogram::apply_acc(std::complex<float> * invec,
				  unsigned int inveclen, 
				  float * outvec,
				  float accumulation_gain)
{

  if(inveclen < fft_len) {
    std::cerr << "Input vector is shorter than FFT buffer " <<
      inveclen << " less than " << fft_len << std::endl;
    return; 
  }
  
  // do the front end FFT
  apply_common(invec, inveclen);
  
  // now copy to the output buffer
  // this is mag^2 divided by the square of of the
  // number of segments we FFTd.
  unsigned int j, k;

  for(j = 0, k = (fft_len / 2); j < (fft_len / 2); j++, k++) {
    outvec[j] = result[k] * (1.0 - accumulation_gain) +
      outvec[j] * accumulation_gain; 
  }

  for(j = (fft_len / 2), k = 0; j < fft_len; j++, k++) {
    outvec[j] = result[k] * (1.0 - accumulation_gain) +
      outvec[j] * accumulation_gain; 
  }
}

void SoDa::Spectrogram::apply_max(std::complex<float> * invec,
				  unsigned int inveclen, 
				  float * outvec,
				  bool first)
{
  // do the front end FFT
  apply_common(invec, inveclen);
  
  // now copy to the output buffer
  // this is mag^2 divided by the square of of the
  // number of segments we FFTd.
  unsigned int j, k;

  for(j = 0, k = (fft_len / 2); j < (fft_len / 2); j++, k++) {
    if(first) outvec[j] = result[k];
    else {
      outvec[j] = (result[k] > outvec[j]) ? result[k] : outvec[j];
    }
  }

  for(j = (fft_len / 2), k = 0; j < fft_len; j++, k++) {
    if(first) outvec[j] = result[k];
    else {
      outvec[j] = (result[k] > outvec[j]) ? result[k] : outvec[j];
    }
  }
}
