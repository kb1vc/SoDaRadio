/*
  Copyright (c) 2012,2013,2014 Matthew H. Reilly (kb1vc)
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

#include "ReSampler.hxx"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

static void bindump(char * fn, std::complex<float> * buf, unsigned int num_elts, int append = 0) __attribute__ ((unused));
static void bindump(char * fn, std::complex<float> * buf, unsigned int num_elts, int append)
{
  FILE * of;
  if(append != 0) {
    of = fopen(fn, "a");
  }
  else {
    of = fopen(fn, "w");
  }
  unsigned int i;
  for(i = 0; i < num_elts; i++) {
    fprintf(of, "%d %g %g\n", i + (append * num_elts), buf[i].real(), buf[i].imag()); 
  }
  fclose(of); 
}

static void bindump_2(char * fn, std::complex<float> * buf, unsigned int num_elts, unsigned int num_cols) __attribute__ ((unused));
static void bindump_2(char * fn, std::complex<float> * buf, unsigned int num_elts, unsigned int num_cols)
{
  FILE * of;
  of = fopen(fn, "w");
  unsigned int i, j;
  for(i = 0; i < num_elts; i++) {
    fprintf(of, "%d ", i);
    for(j = 0; j < num_cols; j++) {
      fprintf(of, "%g %g ", buf[i * num_cols + j].real(), buf[i * num_cols + j].imag());
    }
    fprintf(of, "\n");
  }
  fclose(of); 
}

static unsigned int ipow(unsigned int a, unsigned int b)
{
  unsigned int ret = 1;
  unsigned int i ;

  for(i = 0; i < b; i++) {
    ret *= a; 
  }

  return ret; 
}

SoDa::ReSampler::ReSampler(unsigned int interpolate_ratio,
			   unsigned int decimate_ratio,
			   unsigned int _inlen, 
			   unsigned int _filter_len)
{
  // save the parameters
  inlen = _inlen;
  iM = interpolate_ratio;
  dN = decimate_ratio;
  filter_len = _filter_len; 
  Q = filter_len;
  M = _inlen;


  // give preferences to convenient powers of two.
  // but let's search for the nearest solution that is
  // a multiple of 2^a * 3^b * 5^c where b and c are in the range 0..3
  // and a is in the range 1..16
  unsigned int N_guess, N_best, E_best;
  N_best = 2; 
  E_best = 0x80000000;
  unsigned int i; 
  for(i = 1; i <= 0xff; i++) {
    unsigned int a = i & 0xf;
    unsigned int b = (i >> 4) & 0x3;
    unsigned int c = (i >> 6) & 0x3; 
    N_guess = ipow(2, a) * ipow(3, b) * ipow(5, c);
    if(N_guess >= (M + Q - 1)) {
      unsigned slop = N_guess - (M + Q - 1);
      if(slop < E_best) {
	N_best = N_guess;
	E_best = slop; 
      }
    }
  }
  N = N_best; 

  
  // now that we have N, we can back-calculate Q.
  Q = N - M;

  filter_len = Q; 
  
  tail_index = M - (Q - 1);
  
  // create the buffers.
  // a copy of the input sequence
  inbuf = (std::complex<float> *) fftwf_alloc_complex(N);
  // the transformed input
  in_fft = (std::complex<float> *) fftwf_alloc_complex(N);

  // zero out the whole of the input buffer
  for(i = 0; i < N; i++) inbuf[i] = std::complex<float>(0.0,0.0);
  
  // the filter bank is really iM interleaved filters
  // but we can't seem to make that work right now, so we're
  // backing off to a simpler way. 
  c_filt = new std::complex<float>*[iM];
  for(i = 0; i < iM; i++) {
    c_filt[i] = (std::complex<float> *) fftwf_alloc_complex(N);
  }

  // the upsampled result
  filt_fft = (std::complex<float> *) fftwf_alloc_complex(iM*N);
  // the inverse transform of the upsampled result
  interp_res = (std::complex<float> *) fftwf_alloc_complex(iM*N);

  // the first plan is pretty simple. 
  in_fft_plan = fftwf_plan_dft_1d(N,
				  (fftwf_complex *) inbuf,
				  (fftwf_complex *) in_fft,
				  FFTW_FORWARD, FFTW_ESTIMATE);


  // now create the inverting plans
  int n[1];
  n[0] = N; 
  mid_ifft_plan = fftwf_plan_many_dft(1, n, iM, 
				      (fftwf_complex *) filt_fft, NULL, iM, 1,
				      (fftwf_complex *) interp_res, NULL, iM, 1,
				      FFTW_BACKWARD, FFTW_ESTIMATE);
  

  // now create the polyphase filter banks.
  CreateFilter(filter_len);

  transform_gain = 1.0 / ((float) N); 
}

void SoDa::ReSampler::CreateFilter(unsigned int filter_len)
{
  // filter_len must be odd
  int fN = filter_len;
  if((fN & 1) == 0) fN--; 
  
  unsigned int i, j, k;

  // Use the plan of fischer in "The mkfilter Digital Filter Generation Program" mkshape...
  // This is borrowed quite heavily and with tremendous gratitude from 
  // http://www-users.cs.york.ac.uk/~fisher   Thanks, Prof. Fisher...
  float alpha, beta;
  float cutdown = ((float) ((iM > dN) ? iM : dN));
  beta = 1.0 / (2.2 * cutdown); // This is the transition freq -- a little shy of 0.5/cutdown
  alpha = 0.1; // this is a shape factor

  // create a frequency domain image of the filter.
  float f1 = (1.0 - alpha) * beta;
  float f2 = (1.0 + alpha) * beta;
  float tau = 0.5 / alpha; 

  // now create the filter banks.
  std::complex<float> * filt_FD = (std::complex<float>*) fftwf_alloc_complex(N); 
  std::complex<float> * filt_td = (std::complex<float>*) fftwf_alloc_complex(N); 
  std::complex<float> * filt_td2 = (std::complex<float>*) fftwf_alloc_complex(N); 

  float f;
  float f_incr = 1.0 / ((float) (N));
  unsigned int hlim = (N);
  float gain_corr = 1.0 / ((float) hlim);
  //  gain_corr = gain_corr * gain_corr; 
  for(i = 0, f = 0.0; i <= hlim / 2; i++, f += f_incr) {
    if(f <= f1) {
      filt_FD[i] = std::complex<float>(1.0, 0.0);
    }
    else if (f <= f2) {
      float h = 0.5 * (1.0 + cos(M_PI * tau * (f - f1) / beta));
      filt_FD[i] = std::complex<float>(h, 0.0);      
    }
    else {
      filt_FD[i] = std::complex<float>(0.0, 0.0);            
    }

    filt_FD[i] *= tau * gain_corr;

    if(i != 0) filt_FD[hlim - i] = filt_FD[i]; 
  }

  // then ifft it to time domain
  fftwf_plan filt_ifft_plan = fftwf_plan_dft_1d(hlim,
						(fftwf_complex *) filt_FD,
						(fftwf_complex *) filt_td,
						FFTW_BACKWARD, FFTW_ESTIMATE);
  fftwf_execute(filt_ifft_plan);

  float sum = 0.0;
  for(i = 0; i < hlim; i++) sum += filt_td[i].real();

  float sgain = 1.0 / sum; 
  for(i = 0; i < hlim; i++) filt_td[i] *= sgain; 
  
  fftwf_destroy_plan(filt_ifft_plan); 

  // now truncate the filter
  // We've got an image that is symmetric around 0.
  for(i = 0; i < N; i++) {
    filt_td2[i] = std::complex<float>(0.0,0.0);
  }

  // shift it up to be symmetric around filt_len / 2
  for(i = 0, j = N - (filter_len/2); j < N; i++, j++) {
    filt_td2[i] = filt_td[j]; 
  }
  for(j = 0; j < filter_len / 2; j++, i++) {
    filt_td2[i] = filt_td[j];     
  }

  //  bindump("filt_td2.dat", filt_td2, N);
  
  // and fft back to frequency domain for each of the iM filters.
  for(i = 0; i < iM; i++) {
    // grab the slice of the filter that we need to transform.

    for(j = i, k = 0; j < filter_len; k++, j += iM) {
      filt_td[k] = filt_td2[j]; 
    }
    for(; k < N; k++) filt_td[k] = std::complex<float>(0.0,0.0);
    fftwf_plan filt_fft_plan = fftwf_plan_dft_1d(N,
						 (fftwf_complex *) filt_td,
						 (fftwf_complex *) c_filt[i],
						 FFTW_FORWARD, FFTW_ESTIMATE);
    fftwf_execute(filt_fft_plan);
    fftwf_destroy_plan(filt_fft_plan); 
  }

  // now free up buffers
  fftwf_free(filt_td2);
  fftwf_free(filt_td);
  fftwf_free(filt_FD);  
}

unsigned int SoDa::ReSampler::apply(std::complex<float> * in,
				    std::complex<float> * out,
				    float gain)
{
  unsigned int i, j;
  // copy the input buffer
  memcpy(&(inbuf[Q-1]), in, sizeof(std::complex<float>) * M);
  
  // transform the input
  fftwf_execute(in_fft_plan); // , (fftwf_complex *) in, (fftwf_complex *) in_fft);

  // now multiply by the M filters
  for(i = 0; i < N; i++) {
    for(j = 0; j < iM; j++) {
      filt_fft[j + i*iM] = in_fft[i] * c_filt[j][i]; 
    }
  }
  // do M inverse transforms on the filter result ...
  // to produce M interleaved time domain vectors.
  fftwf_execute(mid_ifft_plan);
	 
  // save the last bits of the input buffer to the Q-1 side of the FFT input vector
  // Do this now incase inbuf and outbuf are the same buffers.  (we may
  // over-write the outbuf with the downsample at the bottom.... )
  memcpy(inbuf, &(in[tail_index]), sizeof(std::complex<float>) * (Q-1));

  // now build the output buffer. 

  // now downsample by a factor of dN.
  int idx = 0; 
  unsigned int mctr = 0;
  unsigned int out_lim = (M * iM) / dN;
  std::complex<float> * ir_clean = &(interp_res[(Q-1)]); 
  // std::cerr << " iM, dN, M, N, out_lim = "
  // 	    << iM << " " << dN << " " << M << " " << N << " " << out_lim << std::endl; 
  for(j = 0; j < out_lim; j++) {
    out[j] = ir_clean[mctr + idx * iM] * gain * transform_gain * ((float) iM);
    mctr += dN;
    while(mctr >= iM) {
      mctr -= iM;
      idx += 1; 
    }
  }
  //  bindump("out.dat", out, j); 

  return 0; 
}



unsigned int SoDa::ReSampler::apply(float * in,
				    float * out,
				    float gain)
{
  unsigned int i, j;
  
  for(i = 0; i < M; i++) {
    inbuf[i+(Q-1)] = std::complex<float>(in[i], 0.0);
  }

  // transform the input
  fftwf_execute(in_fft_plan); // , (fftwf_complex *) in, (fftwf_complex *) in_fft);
  
  // now multiply by the M filters
  for(i = 0; i < N; i++) {
    for(j = 0; j < iM; j++) {
      filt_fft[j + i*iM] = in_fft[i] * c_filt[j][i]; 
    }
  }
  // do M inverse transforms on the filter result ...
  // to produce M interleaved time domain vectors.
  fftwf_execute(mid_ifft_plan);
	 
  // save the last bits of the input buffer to the Q-1 side of the FFT input vector
  // Do this now incase inbuf and outbuf are the same buffers.  (we may
  // over-write the outbuf with the downsample at the bottom.... )
  for(i = 0; i < (Q-1); i++) {
    inbuf[i] = std::complex<float>(in[i + tail_index], 0.0);
  }

  // now build the output buffer. 

  // now downsample by a factor of dN.
  int idx = 0; 
  unsigned int mctr = 0;
  unsigned int out_lim = (M * iM) / dN;
  std::complex<float> * ir_clean = &(interp_res[(Q-1)]); 
  for(j = 0; j < out_lim; j++) {
    out[j] = ir_clean[mctr + idx * iM].real() * gain * transform_gain * ((float) iM);
    mctr += dN;
    while(mctr >= iM) {
      mctr -= iM;
      idx += 1; 
    }
  }

  return 0; 
}
