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
#ifndef RESAMPLER_HDR
#define RESAMPLER_HDR
#include <complex>
#include <fftw3.h>
#include "OSFilter.hxx"

namespace SoDa {
  /**
   * Rational Resampler
   *
   * Create a resampler that upsamples by an interpolation rate and
   * downsamples by a decimation rate. 
   */
  class ReSampler {
  public:
    /**
     * @brief Constructor
     *
     * @param interp_ratio upsample rate
     * @param decim_ratio downsample rate
     * @param _inlen the input buffer length (output buffer will be _inlin * interp_ratio / decimation_ratio)
     * @param filter_len the minimum length of the antialiasing filter
     */
    ReSampler(unsigned int interp_ratio,
	      unsigned int decim_ratio,
	      unsigned int _inlen, 
	      unsigned int filter_len);

    /**
     * @brief apply the resampler to a buffer of IQ samples.
     *
     * @param in input buffer
     * @param out output buffer
     * @param gain -- multiply output by gain
     */
    unsigned int apply(std::complex<float> * in,
		       std::complex<float> * out, float gain = 1.0);
    /**
     * @brief apply the resampler to a buffer of scalar samples.
     *
     * @param in input buffer
     * @param out output buffer
     * @param gain -- multiply output by gain
     */
    unsigned int apply(float * in,
		       float * out, float gain = 1.0);

  private:
    /**
     * @brief create a filter bank for a polyphase resampler. (Though we do it in
     * the frequency domain.)
     * @param filter_len the minimum length of the anti-alias filter
     */
    void CreateFilter(unsigned int filter_len);

    int filter_len; ///< the length of the polyphase filter that we're using.
    
    unsigned int iM, dN, Q, tail_index;
    unsigned int inlen, M, N; 
    std::complex<float> ** c_filt; ///< filter bank for interp/deci filter
    std::complex<float> * filt_fft; ///< the filtered image of the input stream
    std::complex<float> * interp_res; ///< the interpolation result
    std::complex<float> * in_fft; ///< the result of the first fft stage
    std::complex<float> * inbuf; ///< the fft input buffer

    // gain corrections and other stuff
    float transform_gain;
    float gain_correction; 
    
    // we'll create plans
    fftwf_plan in_fft_plan;
    fftwf_plan mid_ifft_plan;
  }; 
}

#endif // RESAMPLER_HDR
