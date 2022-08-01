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

#include "ReSampler.hxx"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

namespace SoDa {
  /**
   * @class ReSampler
   * 
   * This is a rational resampler class providing arbitrary conversions
   * though certainly not optimal. However, all the conversion is done 
   * in the frequency domain, so the cost is more-or-less NlogN in the 
   * size of the *larger* buffer. 
   *
   * The resampler is specified in terms of the input sampling rate (Fsi) and
   * the output sampling rate (Fso). The interpolation (U) and decimation (D) factors
   * are determined by:
   * 
   * U = Fso / gcd(Fsi,Fso)  D = Fsi / gcd(Fsi,Fso)
   *  
   * There are some restrictions on the input and output buffers, however. 
   * The input buffer size must always be a multiple of the decimation rate D. This
   * follows from the output buffer size : 
   * 
   * out_len = in_len * U / D
   * 
   * which must be an integer. As long as in_len is a multiple of D, we're home free. 
   * 
   * 
   * This is a block-application filter. To adapt it to continuous use, 
   */
  
  ReSampler::ReSampler(float input_sample_rate,
		       float output_sample_rate,
		       float time_span_min, 
		       float time_span_max) {
    // first figure out the input corner frequencies.
    // We want to trim 10% of the top and bottom of the
    // lower frequency component to get a suppresion of
    // about 40dB. 
    float dB_atten = 40.0; 

    // We're going to build a low-pass anti-aliasing filter.
    float fs_smaller = (input_sample_rate < output_sample_rate) ? 
      input_sample_rate : output_sample_rate;
    float fs_bigger = (input_sample_rate > output_sample_rate) ? 
      input_sample_rate : output_sample_rate;
    float fs_transition = 0.1 * fs_smaller;

    // we apply the filter to the input buffer. We'll use
    // fred harriss's approximation
    float fN = (input_sample_rate / fs_transition) * dB_atten / 22.0;

    // This needs to be odd.
    int N = (2 * (int(fN / 2))) + 1;

    // So that is the minimum N. we need to calculate the minimum input buffer
    // size, and then "round up" to a good buffer length that is both a multiple
    // of the decimation rate and the interpolation rate. This is the larger
    // buffer of the two. The actual buffer length will be longer to accommodate
    // the necesasry filter.
    //
    // This is a little dicey, so watch my hands carefully.
    // First find the GCD of the two rates. We're going to assume the
    // not-so-special case that both rates are INTEGERs
    unsigned int in_sr = ((unsigned int) input_sample_rate);
    unsigned int out_sr = ((unsigned int) output_sample_rate);

    // what is the minimum large buffer size?
    unsigned int min_buf_len = (unsigned int) (fs_bigger * time_span_min);

    auto gcd = getGCD(in_sr, out_sr);
    in_sr = in_sr / gcd;
    out_sr = out_sr / gcd;

    stuff_in = out_sr - 1;
    trim_out = in_sr - 1;
    // so we're going to interpolate by out_sr and decimate by in_sr...
    // regardless of whether the net is upsampling or downsampling. 
    
    // now the large buffer must be at least
    auto large_buf_len = in_sr * out_sr;
    while(large_buf_len < min_buf_len) {
      large_buf_len += in_sr * out_sr; 
    }

    // Here's the strange part: We're doing this as a continuous frequency
    // domain resampler. So we're *really* doing the resampling on an overlap
    // and save buffer

    /// There's no magic here.  for U > D input -> FFT ->filter -> sel Buf * D / U -> IFFT
    /// U < D input --> FFT ---> stuff --> select Buf * D / U -->IFFT
    /// But buf must be a multiple of U in length.
    /// We also need to make sure Buf * D / U is larger than the number of filter taps. - 1
    /// as we'll be throwing out the first M * D / U samples
      
    // Now create the input filter -- two corners at +/- fs_transition
    filter = std::unique_ptr<OSFilter>(new OSFilter(Filter::FilterSpec(input_sample_rate, N, COMPLEX)
						    .add(-input_sample_rate / 2, 0)
						    .add(-(1.1 * fs_smaller), 0)
						    .add(-(0.9 * fs_smaller), 1)
						    .add(+(0.9 * fs_smaller), 1)
						    .add(+(1.1 * fs_smaller), 1)), 
				       total_in_buffer_length);



  unsigned int ReSampler::getInputBufferSize();
  
  unsigned int ReSampler::getOutputBufferSize();
  
  
  unsigned int ReSampler::apply(std::vector<std::complex<float>> & in,
		       std::vector<std::complex<float>> & out, 
		       float gain = 1.0, 
		       );

  unsigned int ReSampler::apply(float * in,
		       float * out, float gain = 1.0);
}
