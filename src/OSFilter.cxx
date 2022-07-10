/*
  Copyright (c) 2012,2022 Matthew H. Reilly (kb1vc)
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

#include "OSFilter.hxx"

#include <iostream>
#include <string.h>
#include <fftw3.h>
#include <math.h>
#include <vector>


static unsigned int ipow(unsigned int x, unsigned int y)
{
  unsigned int ret;
  ret = 1;
  unsigned int i;

  for(i = 0; i < y; i++) {
    ret *= x; 
  }

  return ret; 
}

#if 0
SoDa::OSFilter::OSFilter(float * filter_impulse_response,
			 unsigned int filter_length,
			 float filter_gain, 
			 unsigned int inout_buffer_length,
			 OSFilter * cascade, 
			 unsigned int suggested_transform_length)
{
  // these are the salient dimensions for this Overlap/Save
  // widget (for terminology, see Lyons pages 719ff
  Q = filter_length; // to start with. 
  M = inout_buffer_length;

  if((cascade != NULL) && (cascade->M != M)) cascade = NULL;

  if(M < 4 * Q) {
    std::cerr << "Warning -- OSFilter asked to implement a long filter against a short buffer." << std::endl;
  }
  
  // now find N.
  if(suggested_transform_length > (M + Q - 1)) {
    N = suggested_transform_length; 
  }
  else {
    N = guessN(); 
  }
  
  // now that we have N, we can back-calculate Q.
  Q = N - M;

  tail_index = M - (Q - 1);

  // now build the transform image for the filter.
  std::complex<float> * filter_in = (std::complex<float> *) fftwf_malloc(sizeof(std::complex<float>) * N);
  filter_fft = (std::complex<float> *) fftwf_malloc(sizeof(std::complex<float>) * N);
  
  // create a temporary plan
  fftwf_plan tplan = fftwf_plan_dft_1d(N, (fftwf_complex*) filter_in, (fftwf_complex*) filter_fft,
				       FFTW_FORWARD, FFTW_ESTIMATE);
  
  // now build the filter
  // fill with zeros
  unsigned int i; 
  for(i = 0; i < N; i++) filter_in[i] = std::complex<float>(0.0,0.0);
  // fill in the impulse response
  float gain_corr = 1.0 / ((float) N) * filter_gain;
  for(i = 0; i < filter_length; i++) filter_in[i] = std::complex<float>(filter_impulse_response[i] * gain_corr, 0.0);
  
  // transform the filter. 
  fftwf_execute(tplan);

  // and forget the plan. 
  fftwf_destroy_plan(tplan);

  // filter lengths must be equal too
  if((cascade != NULL) && (cascade->N != N)) cascade = NULL;

  // if there is a cascaded filter, multiply our filter coeffs by the cascaded filter coeffs
  if(cascade != NULL) {
    for(i = 0; i < N; i++) {
      std::complex<double> a = filter_fft[i];
      std::complex<double> b = cascade->filter_fft[i];
      std::complex<double> ff = a * b * ((double) N); 
      filter_fft[i] = std::complex<float>(ff.real(), ff.imag()); 
    }
  }
  
  
  // setup the fft buffers
  setupFFT();
}
#endif

SoDa::OSFilter::OSFilter(float low_cutoff,
			 float low_pass_edge,
			 float high_pass_edge,
			 float high_cutoff,

			 unsigned int filter_length,
			 float filter_gain,
			 float sample_rate, 

			 unsigned int inout_buffer_length,
			 unsigned int suggested_transform_length)
{
  // remember our edges
  low_edge = (double) low_pass_edge;
  high_edge = (double) high_pass_edge;
  
  // first find our buffer sizes.
  Q = filter_length;
  M = inout_buffer_length;
  if(M < 4 * Q) {
    std::cerr << "Warning -- OSFilter asked to implement a long filter against a short buffer." << std::endl;
  }
  
  // now find N.
  if(suggested_transform_length > (M + Q - 1)) {
    N = suggested_transform_length; 
  }
  else {
    N = guessN(); 
  }

  // now back calculate the actual filter length
  Q = N - M;

  tail_index = M - (Q - 1);

  // OK.  now we build the filter.
  // First, build an allpass filter with the appropriate delay
  std::complex<float> * filter_in = (std::complex<float> *) fftwf_malloc(sizeof(std::complex<float>) * N);
  filter_fft = (std::complex<float> *) fftwf_malloc(sizeof(std::complex<float>) * N);

  unsigned int i, j;
  for(i = 0; i < N; i++) filter_in[i] = std::complex<float>(0.0,0.0);
  filter_in[Q/2] = std::complex<float>(1.0, 0.0);
  // we'll use the image of this filter for the phase part of our filter.
  
  fftwf_plan fplan = fftwf_plan_dft_1d(N, (fftwf_complex *) filter_in, (fftwf_complex *) filter_fft,
				       FFTW_FORWARD, FFTW_ESTIMATE);
  fftwf_plan ifplan = fftwf_plan_dft_1d(N, (fftwf_complex *) filter_fft, (fftwf_complex *) filter_in,
				       FFTW_BACKWARD, FFTW_ESTIMATE);					

  // create an image that we can fill in.
  fftwf_execute(fplan);

  float freq_step = sample_rate / ((float) N);
  float fr; 
  // now setup the filter image;
  for(fr = 0.0, i = 0, j = (N - 1); i < N/2; i++, j--, fr += freq_step) {
    if(fr < low_cutoff) {
      filter_fft[i] = std::complex<float>(0.0,0.0);
      filter_fft[j] = std::complex<float>(0.0,0.0);
    }
    else if(fr < low_pass_edge) {
      float mult = (fr - low_cutoff) / (low_pass_edge - low_cutoff);
      filter_fft[i] = filter_fft[i] * std::complex<float>(mult, 0.0);
      filter_fft[j] = filter_fft[j] * std::complex<float>(mult, 0.0);
    }
    else if(fr < high_pass_edge) {
      // keep the all pass value
    }
    else if(fr < high_cutoff) {
      float mult = 1.0 - ((fr - high_pass_edge) / (high_cutoff - high_pass_edge));
      filter_fft[i] = filter_fft[i] * std::complex<float>(mult, 0.0);
      filter_fft[j] = filter_fft[j] * std::complex<float>(mult, 0.0);
    }
    else {
      filter_fft[i] = std::complex<float>(0.0,0.0);
      filter_fft[j] = std::complex<float>(0.0,0.0);
    }
  }

  // inverse transform the filter image
  fftwf_execute(ifplan);

  // now we've got a FIR filter, but it needs to be windowed before we can trust it.
  // create a HammingWindow of length Q
  std::vector<float> window(Q);
  hammingWindow(window);


  for(i = 0; i < Q; i++) {
    filter_in[i] = filter_in[i] * window[i];
    filter_in[i].imag(0.0);
  }
  for(i = Q; i < N; i++) {
    filter_in[i] = std::complex<float>(0.0, 0.0); 
  }
  
  // now create the filter image
  fftwf_execute(fplan);

  // now we need to normalize the envelope so that we get the specified gain.
  float maxval = 0.0;
  for(i = 0; i < N; i++) {
    float v = abs(filter_fft[i]);
    if(v > maxval) maxval = v; 
  }

  std::complex<float> normalize(filter_gain / (((float) N) * maxval), 0.0);

  for(i = 0; i < N; i++) {
    filter_fft[i] = filter_fft[i] * normalize; 
  }

  // and destroy the plans
  fftwf_destroy_plan(fplan); 
  fftwf_destroy_plan(ifplan); 

  // and free the impulse response
  fftwf_free(filter_in);
  
  // and setup the FFT buffers
  setupFFT(); 
}



int SoDa::OSFilter::guessN()
{
    // give preferences to convenient powers of two.
    // but let's search for the nearest solution that is
    // a multiple of 2^a * 3^b * 5^c where b and c are in the range 0..3
    // and a is in the range 1..16
  unsigned int N_guess, N_best, E_best;
  N_best = 2; 
  E_best = 0x80000000;
  int i; 
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
  
  return N_best; 
}

void SoDa::OSFilter::setupFFT()
{
  unsigned int i; 
  // now allocate all the storage vectors
  fft_input = (std::complex<float> *) fftwf_malloc(sizeof(std::complex<float>) * N);
  fft_output = (std::complex<float> *) fftwf_malloc(sizeof(std::complex<float>) * N);
  ifft_output = (std::complex<float> *) fftwf_malloc(sizeof(std::complex<float>) * N);
  
  // and create the plans
  forward_plan = fftwf_plan_dft_1d(N,
				   (fftwf_complex *) fft_input,
				   (fftwf_complex *) fft_output,
				   FFTW_FORWARD, FFTW_ESTIMATE);
  backward_plan = fftwf_plan_dft_1d(N,
				    (fftwf_complex *) fft_output,
				    (fftwf_complex *) ifft_output,
				    FFTW_BACKWARD, FFTW_ESTIMATE);

  // zero out the start of the fft_input buffer for the first iteration.
  for(i = 0; i < Q-1; i++) fft_input[i] = std::complex<float>(0.0,0.0);

}

unsigned int SoDa::OSFilter::apply(float * inbuf, float * outbuf, float outgain, int instride, int outstride)
{
  unsigned int i, j;
  // copy the input buffer.
  for(i = 0, j = Q-1; i < (M * instride); i += instride, j++) {
    fft_input[j] = std::complex<float>(inbuf[i], 0.0); 
  }
  
  // now do the forward FFT on the input
  fftwf_execute(forward_plan);

  // save the last bits of the input buffer to the Q-1 side of the FFT input vector
  // Do this now incase in buf and outbuf are the same buffers.  (we're going to
  // over-write most of outbuf with the memcpy at the bottom.... )
  for(i = (tail_index * instride), j = 0; j < Q-1; i += instride, j++) {
    fft_input[j] = std::complex<float>(inbuf[i], 0.0);
  }

  // apply the filter.
  for(i = 0; i < N; i++) {
    fft_output[i] = (fft_output[i] * filter_fft[i]) * outgain; 
  }
  
  // now do the backward FFT on the result
  fftwf_execute(backward_plan);


  // and copy the result to the output buffer, but discard the
  // first Q-1 chunks
  for(i = 0, j = Q-1; i < (M * outstride); i += outstride, j++) {
    outbuf[i] = ifft_output[j].real();
  }

  return M; 
}

unsigned int SoDa::OSFilter::apply(std::complex<float> * inbuf, std::complex<float> * outbuf, float outgain)
{
  // This is the overlap-save FFT filter technique described in Lyons pages 719ff.
  // first we need to copy the input buffer to the M side of the FFT input vector.
  memcpy(&(fft_input[Q-1]), inbuf, sizeof(std::complex<float>) * M);

  // now do the forward FFT on the input
  fftwf_execute(forward_plan);

  // save the last bits of the input buffer to the Q-1 side of the FFT input vector
  // Do this now incase inbuf and outbuf are the same buffers.  (we're going to
  // over-write most of outbuf with the memcpy at the bottom.... )
  memcpy(fft_input, &(inbuf[tail_index]), sizeof(std::complex<float>) * (Q-1));

  // apply the filter.
  unsigned int i;
  for(i = 0; i < N; i++) {
    fft_output[i] = (fft_output[i] * filter_fft[i]) * outgain; 
  }
  
  // now do the backward FFT on the result
  fftwf_execute(backward_plan);

  // and copy the result to the output buffer, but discard the
  // first Q-1 chunks
  memcpy(outbuf, &(ifft_output[Q-1]), sizeof(std::complex<float>) * M);

  return M; 
}

void SoDa::OSFilter::dump(std::ostream & os)
{
  unsigned int i;
  os << "# idx  real   imag   abs   arg" << std::endl; 
  for(i = 0; i < N; i++) {
    float mag = abs(filter_fft[i]);
    float phase = arg(filter_fft[i]); 
    os << i << " " << filter_fft[i].real() << " " << filter_fft[i].imag() << " " << mag << " " << phase << std::endl;
  }
}

void SoDa::OSFilter::hammingWindow(std::vector<float> & w) {
  float a0 = 25.0 / 46.0;
  float a1 = 1.0 - a0; 
  unsigned int N = w.size();
  float anginc = 2.0 * M_PI / ((float) (N-1));
  
  for(int n = 0; n < N; n++) {
    float ang = ((float) n) * anginc;
    w[n] = a0 - a1 * cos(ang);
  }
}
