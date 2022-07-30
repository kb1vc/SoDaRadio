#pragma once
/*
  Copyright (c) 2022 Matthew H. Reilly (kb1vc)
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

#include <complex>
#include <vector>
#include <fftw3.h>

namespace SoDa {
  /**
   * Rational Resampler
   *
   * Create a resampler that upsamples by an interpolation rate and
   * downsamples by a decimation rate. 
   */
  class ReSampler : public BlockOp {
  public:
    /**
     * @brief Constructor
     *
     * @param input_sample_rate
     * @param output_sample_rate
     * @param time_span_min how many samples (in time) should a buffer hold? 
     * @param time_span_max upper bound on samples per buffer
     */
    ReSampler(float input_sample_rate,
	      float output_sample_rate,
	      float time_span_min, 
	      float time_span_max); 


    unsigned int getInputBufferSize();

    unsigned int getOutputBufferSize();
      
    
    /**
     * @brief apply the resampler to a buffer of IQ samples.
     *
     * @param in input buffer
     * @param out output buffer
     * @param gain -- multiply output by gain
     * @param in_out_mode -- always ignored, assumed to be TIME in TIME out
     */
    unsigned int apply(std::vector<std::complex<float>> & in,
		       std::vector<std::complex<float>> & out, 
		       float gain = 1.0, 
		       INOUT_MODE in_out_mode
		       );
    /**
     * @brief apply the resampler to a buffer of scalar samples.
     *
     * @param in input buffer
     * @param out output buffer
     * @param gain -- multiply output by gain
     * @param in_out_mode -- always ignored, assumed to be TIME in TIME out
     */
    unsigned int apply(float * in,
		       float * out, 
		       float gain = 1.0, 
		       INOUT_MODE in_out_mode);

  protected:
    std::unique_ptr<Filter> lpf; /// the anti-aliasing low pass filter. 
  }; 
}

