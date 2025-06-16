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

#include "HilbertTransformer.hxx"

#include <iostream>
#include <string.h>
#include <fftw3.h>
#include <SoDa/Format.hxx>

int dbgctr = 0;

static unsigned int ipow(unsigned int x, unsigned int y) __attribute__ ((unused));
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

SoDa::HilbertTransformer::HilbertTransformer(unsigned int inout_buffer_length,
					     unsigned int filter_length) :
  SoDa::Base("HilbertTransformer")
{
  // these are the salient dimensions for this Overlap/Save
  // widget (for terminology, see Lyons pages 719ff
  M = inout_buffer_length;

  // now find N.
  N = 4 * filter_length;
  while(N < (M + filter_length)) {
    N = N * 2; 
  }
  // now that we have N, we can back-calculate Q.
  Q = (N - M) + 1;

  //  std::cerr << "\n\nHILBERT picked N = " << N << " Q = " << Q << " M = " << M << std::endl;


  std::vector<std::complex<float>> htu(N), htl(N); 

  // create the impulse response images
  // There is probably a simpler way, but the obvious real/imag swap
  // scheme doesn't work at all well. 
  HTu_filter = (std::complex<float> *) fftwf_malloc(sizeof(std::complex<float>) * N);
  HTl_filter = (std::complex<float> *) fftwf_malloc(sizeof(std::complex<float>) * N);
  Pass_U_filter = (std::complex<float> *) fftwf_malloc(sizeof(std::complex<float>) * N);
  Pass_L_filter = (std::complex<float> *) fftwf_malloc(sizeof(std::complex<float>) * N);
  fftwf_plan HTu_plan = fftwf_plan_dft_1d(N,
					  (fftwf_complex *) htu.data(), (fftwf_complex *) HTu_filter, 
					 FFTW_FORWARD, FFTW_ESTIMATE);
  if(HTu_plan == NULL) {
    throw SoDa::Radio::Exception("Hilbert had trouble creating HT upper plan...\n");
  }
  fftwf_plan HTl_plan = fftwf_plan_dft_1d(N,
					  (fftwf_complex *) htl.data(), (fftwf_complex *) HTl_filter, 
					 FFTW_FORWARD, FFTW_ESTIMATE);
  if(HTl_plan == NULL) {
    throw SoDa::Radio::Exception("Hilbert had trouble creating HT lower plan...\n");
  }

  // now build the time domain image of the filter.
  unsigned int i, j;
  for(i = 0; i < htu.size(); i++) {
    htu[i] = htl[i] = std::complex<float>(0.0, 0.0);
  }
  // this is actually a scaled ht -- removing the Fs * 2 / pi scaling factor.
  for(i = 0, j = (Q / 2); i < (Q / 2); i++) {
    if((i & 1) != 0) {
      htu[j + i] = std::complex<float>(1.0 / ((float) i), 0.0);
      htu[j - i] = std::complex<float>(-1.0 / ((float) i), 0.0);
      htl[j + i] = std::complex<float>(-1.0 / ((float) i), 0.0);
      htl[j - i] = std::complex<float>(1.0 / ((float) i), 0.0);
    }
  } 
  fftwf_execute(HTu_plan);
  fftwf_execute(HTl_plan);
  // now we have the HT filter image
  fftwf_destroy_plan(HTu_plan);
  fftwf_destroy_plan(HTl_plan);

  // now do the delay filter
  fftwf_plan dly_plan = fftwf_plan_dft_1d(N,
					  (fftwf_complex *) htu.data(), (fftwf_complex *) Pass_U_filter, 
					 FFTW_FORWARD, FFTW_ESTIMATE);
  // load up a new impulse response
  for(i = 0; i < N; i++) htu[i] = std::complex<float>(0.0, 0.0);
  htu[Q/2] = std::complex<float>(1.0, 0.0);
  // now create the passthrough filter
  fftwf_execute(dly_plan);
  fftwf_destroy_plan(dly_plan); 

  // Do some equalization on the pass filter to fix the low frequency response
  // to match the response of the HT filter.
  for(i = 0; i < N; i++) {
    float tumag = abs(HTu_filter[i]);
    float tlmag = abs(HTl_filter[i]);
    float pmag = abs(Pass_U_filter[i]);
    float uadj = 1.0;
    float ladj = 1.0;
    if(pmag > 0.001) {
      uadj = tumag / pmag;
      ladj = tlmag / pmag;
      if (uadj > 2) uadj = 1.0; 
      if (ladj > 2) ladj = 1.0; 
    }
    Pass_L_filter[i] = ladj * Pass_U_filter[i]; 
    Pass_U_filter[i] = uadj * Pass_U_filter[i]; 
  }


  // calculate the magnitudes of the two filters.
  float hmag, pmag;
  hmag = 0.0;
  pmag = 0.0; 
  for(i = 0; i < N; i++) {
    hmag += HTu_filter[i].real() * HTu_filter[i].real()
      + HTu_filter[i].imag() * HTu_filter[i].imag();
    pmag += Pass_U_filter[i].real() * Pass_U_filter[i].real()
      + Pass_U_filter[i].imag() * Pass_U_filter[i].imag(); 
  }
  
  // now allocate all the storage vectors
  fft_I_input = (std::complex<float> *) fftwf_malloc(sizeof(std::complex<float>) * (N + 128));
  fft_Q_input = (std::complex<float> *) fftwf_malloc(sizeof(std::complex<float>) * (N + 128));
  fft_I_output = (std::complex<float> *) fftwf_malloc(sizeof(std::complex<float>) * (N + 128));
  fft_Q_output = (std::complex<float> *) fftwf_malloc(sizeof(std::complex<float>) * (N + 128));
  ifft_I_input = (std::complex<float> *) fftwf_malloc(sizeof(std::complex<float>) * (N + 128));
  ifft_Q_input = (std::complex<float> *) fftwf_malloc(sizeof(std::complex<float>) * (N + 128));
  ifft_I_output = (std::complex<float> *) fftwf_malloc(sizeof(std::complex<float>) * (N + 128));
  ifft_Q_output = (std::complex<float> *) fftwf_malloc(sizeof(std::complex<float>) * (N + 128));

  // and create the plans
  forward_I_plan = fftwf_plan_dft_1d(N,
				     (fftwf_complex *) fft_I_input, (fftwf_complex *) fft_I_output,
				     FFTW_FORWARD, FFTW_ESTIMATE);
  if(forward_I_plan == NULL) {
    throw SoDa::Radio::Exception("Hilbert had trouble creating forward I plan...\n");
  }

  forward_Q_plan = fftwf_plan_dft_1d(N,
				     (fftwf_complex *) fft_Q_input,
				     (fftwf_complex *) fft_Q_output,
				     FFTW_FORWARD, FFTW_ESTIMATE);
  if(forward_Q_plan == NULL) {
    throw SoDa::Radio::Exception("Hilbert had trouble creating forward Q plan...\n");
  }

  backward_I_plan = fftwf_plan_dft_1d(N, (fftwf_complex *) ifft_I_input,
				      (fftwf_complex *) ifft_I_output,
				      FFTW_BACKWARD, FFTW_ESTIMATE);
  if(backward_I_plan == NULL) {
    throw SoDa::Radio::Exception("Hilbert had trouble creating backward I plan...\n");
  }
  backward_Q_plan = fftwf_plan_dft_1d(N, (fftwf_complex *) ifft_Q_input,
				      (fftwf_complex *) ifft_Q_output,
				      FFTW_BACKWARD, FFTW_ESTIMATE);
  if(backward_Q_plan == NULL) {
    throw SoDa::Radio::Exception("Hilbert had trouble creating backward Q plan...\n");
  }
  // zero out the start of the fft_input buffer for the first iteration.
  for(i = 0; i <= Q-1; i++) {
    fft_I_input[i] = 0.0;
    fft_Q_input[i] = 0.0;
  }
  
  // finally, set the transform gain (1/N)
  //   passthrough_gain = 1.0 / ((float) N); 
  //  H_transform_gain = 2.0 / (M_PI * 0.966 * ((float) N)); // 0.966 is a fudge factor
  H_transform_gain = 2.0 / (M_PI * ((float) N)); 
  passthrough_gain = H_transform_gain;
}

unsigned int SoDa::HilbertTransformer::apply(std::vector<std::complex<float>> & inbuf,
					     std::vector<std::complex<float>> & outbuf,
					     bool pos_sided, float gain)
{
  unsigned int i, j;

  std::complex<float> *HT_F;
  std::complex<float> *PA_F;

  if(pos_sided) {
    HT_F = HTl_filter;
    PA_F = Pass_L_filter; 
  }
  else {
    HT_F = HTu_filter;
    PA_F = Pass_U_filter; 
  }
  // Note we're using overlap-and-save  see the OSFilter implementation
  // or Lyons pages 719ff
  // copy the I channel to the tail of the input buffer.
  memcpy(&(fft_I_input[Q-1]), inbuf.data(), sizeof(std::complex<float>) * M);

  // do a pass through
  fftwf_execute(forward_I_plan); 

  // now save the tail of the input to the save buffer
  memcpy(fft_I_input, &(inbuf[1 + (M - Q)]), sizeof(std::complex<float>) * (Q - 1));

  // now apply the delay filter and the hilbert transform
  for(i = 0; i < N; i++) {
    ifft_Q_input[i] = fft_I_output[i] * HT_F[i]; 
    ifft_I_input[i] = fft_I_output[i] * PA_F[i]; 
  }
  
  // do the inverse fft for the I and Q channels
  fftwf_execute(backward_I_plan);
  fftwf_execute(backward_Q_plan);

  // now put the two channels together
  // Note that we're shifting the normal sampling window.  This is because the
  // quadrature sampler is just not quite right for
  for(i = 0, j = Q-1; i < M; i++, j++) {
    // seems like it worked once... but apparent shift is same for either sideband
    outbuf[i] = std::complex<float>(ifft_I_output[j].real() * passthrough_gain * gain,
				    ifft_Q_output[j].real() * H_transform_gain * gain);
  }

  dbgctr++;


  return M; 
}


unsigned int SoDa::HilbertTransformer::apply(std::vector<float> & inbuf,
					     std::vector<std::complex<float>> & outbuf,
					     bool pos_sided, float gain)
{
  unsigned int i;
  // This creates an analytic signal from a single input buffer.

  // Note we're using overlap-and-save  see the OSFilter implementation
  // or Lyons pages 719ff
  std::vector<std::complex<float>> cinbuf(M);


  // copy the I channel to the tail of the input buffer.
  for(i = 0; i < M; i++) {
    cinbuf[i] = std::complex<float>(inbuf[i], 0.0); 
  }

  // call the complex HT
  return apply(cinbuf, outbuf, pos_sided, gain); 

  
  return M; 
}


unsigned int SoDa::HilbertTransformer::applyIQ(std::vector<std::complex<float>> & inbuf,
					       std::vector<std::complex<float>> & outbuf,
					       float gain)
{
  unsigned int i, j;
  // This creates an analytic signal from a single input buffer.

  // Note we're using overlap-and-save  see the OSFilter implementation
  // or Lyons pages 719ff

  // copy the I channel to the tail of the I input buffer.
  // copy the I channel to the tail of the Q input buffer.
  for(i = 0; i < M; i++) {
    fft_I_input[i + (Q-1)] = std::complex<float>(inbuf[i].real(), 0.0); 
    fft_Q_input[i + (Q-1)] = std::complex<float>(inbuf[i].imag(), 0.0); 
  }

  // do a the I (passthrough) and Q channel FFTs
  fftwf_execute(forward_I_plan);
  fftwf_execute(forward_Q_plan);
  

  // now save the tail of the input to the save buffer
  for(i = 0; i < (Q - 1); i++) {
    fft_I_input[i] = std::complex<float>(inbuf[i + 1 + (M - Q)].real(), 0.0); 
    fft_Q_input[i] = std::complex<float>(inbuf[i + 1 + (M - Q)].imag(), 0.0); 
  }

  // now apply the delay filter (to I) and the hilbert transform (to Q)
  for(i = 0; i < N; i++) {
    ifft_Q_input[i] = fft_Q_output[i] * HTu_filter[i]; 
    ifft_I_input[i] = fft_I_output[i] * Pass_U_filter[i]; 
  }
  
  // do the inverse fft for the I and Q channels
  fftwf_execute(backward_I_plan);
  fftwf_execute(backward_Q_plan);

  // now put the two channels together
  // Note that we're shifting the normal sampling window.  This is because the
  // quadrature sampler is just not quite right for
  for(i = 0, j = Q-1; i < M; i++, j++) {
    outbuf[i] = std::complex<float>(ifft_I_output[j].real() * passthrough_gain * gain,
				    ifft_Q_output[j].real() * H_transform_gain * gain);
  }

  dbgctr++;

  return M; 
}



std::ostream & SoDa::HilbertTransformer::dump(std::ostream & os)
{
  unsigned int i, j;
  for(i = 0; i < N; i++) {
    j = i; 
    float mag = std::abs(HTl_filter[i]);
    float ang = std::arg(HTl_filter[i]); 
    float pmag = std::abs(Pass_L_filter[i]);
    float pang = std::arg(Pass_L_filter[i]); 
    os << SoDa::Format("%0 %1 %2 %3 %4 %5 %6 %7 %8\n")
      .addI(j)
      .addF(HTu_filter[i].real())
      .addF(HTu_filter[i].imag())
      .addF(mag)
      .addF(ang)
      .addF(Pass_U_filter[i].real())
      .addF(Pass_U_filter[i].imag())
      .addF(pmag)
      .addF(pang);
  }
  return os; 
}  
