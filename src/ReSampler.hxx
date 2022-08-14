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
#include <cstdint>
#include <fftw3.h>
#include "Filter.hxx"

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
     * @param input_sample_rate
     * @param output_sample_rate
     * @param time_span how many samples (in time) should a buffer hold? 
     *
     * The constructor creates the temporary overlap-and-save buffers, 
     * calculates the sizes of the input and output buffers, 
     * and builds the anti-aliasing filter. 
     * 
     * Note that the user has little influence over the size of the input
     * buffers (and the output buffer size follows from the ratio of the sample
     * rates.) These are set by the necessary overlap to provide a continuous
     * resampler. 
     * 
     * The prototype for this resampler can be found in ../jupyter/ReSamplerII.ipynb
     * 
     * Here's the general principle: 
     * We take an input buffer x with a sample rate f1 and want to convert to y with a sample rate f2. 
     * Assume for the moment we're talking a non-continous resampler. 
     * 
     * Our object is to create an FFT image of x and an FFT image of y with exactly the same bucket sizes. 
     * 
     * U = Fy / gcd(Fx,Fy)
     * D = Fx / gcd(Fx,Fy)
     * 
     * Then we can create X with an FFT length of (and length of x) some multiple of  D so  Lx = k  * D  That makes each bucket in X
     * Bx = Fx / Lx wide. Y and y's length will be Ly = Lx * U / D = k * U.  Each bucket in Y will be Fy / Ly wide. 
     *
     * But Fy = Fx * U / D so Bx = Fy / Ly = (Fx * U / D) / (Lx * U / D) = Fx / Lx  
     * which makes Bx = Lx. 
     * 
     * We pick the smallest k so that k * D * Fx > time_span
     * 
     * So, ignoring the continous case, the scheme is: 
     * 
     * X = FFT(LPF(x))
     * Y[0:ly/2] = X[0:ly/2]
     * Y[-ly/2:] = X[-ly/2:]
     * all other Y = 0
     * 
     * y = IFFT(Y)
     * 
     * And now for the continous case: 
     * 
     * It takes U samples of the input stream to produce the first D samples of the output stream. 
     * If U and D are relatively prime (we did that by removing all common factors) then we can't trust 
     * the first D samples for a stand-alone resample operation, because we don't have input samples 
     * x[-U..0].  But for the continous case, we do. We can use the last U samples from the previous 
     * round to create the first D samples of this round. But those last U samples already gave us 
     * the last D samples of the previous round. So we'll discard the first D samples of output
     * from each round, and save the last U samples of input from each round. This is exactly like 
     * overlap-and-save, but with a slightly different criteria for choosing the overlap size. 
     */
    ReSampler(float input_sample_rate,
	      float output_sample_rate,
	      float time_span);

    uint32_t getInputBufferSize();

    uint32_t getOutputBufferSize();
      
    /**
     * @brief apply the resampler to a buffer of IQ samples.
     *
     * @param in input buffer 
     * @param out output buffer
     */
    uint32_t apply(std::vector<std::complex<float>> & in,
		       std::vector<std::complex<float>> & out, 
		       );
    /**
     * @brief apply the resampler to a buffer of scalar samples.
     *
     * @param in input buffer
     * @param out output buffer
     */
    uint32_t apply(float * in,
		       float * out);


  protected:
    std::unique_ptr<Filter> lpf; /// the anti-aliasing low pass filter. 
    
    uint32_t U; /// upsample rate
    uint32_t D; /// decimation rate

    uint32_t input_buffer_size;
    uint32_t output_buffer_size; 
    
    uint32_t output_lower_half, output_upper_half; /// selectors for output from input image

    uint32_t save_buf_size;   /// we do an overlap-and-save approach here
    std::vector<std::complex<float>> save_buf; /// here are the saved samples
    uint32_t discard_count;  /// and we throw out samples at the end. 
  }; 
}

