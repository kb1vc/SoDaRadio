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
  static uint32_t getGCD(uint32_t a, uint32_t b) {
    if(b == 0) return a;
    else if(a < b) {
      return getGCD(b, a);
    }
    else {
      return getGCD(b, a % b);
    }
  }
  
  ReSampler::ReSampler(float FS_in,
		       float FS_out,
		       float time_span_min) {
    uint32_t i_fs_in = ((uint32_t) FS_in);
    uint32_t i_fs_out = ((uint32_t) FS_out);
    auto gcd = getGCD(i_fs_in, i_fs_out);

    U = uint32_t(i_fs_out / gcd);
    D = uint32_t(i_fs_in / gcd);

    // now find the input buffer size -- make it long enough to span time_span_min
    uint32_t min_in_samples = uint32_t(FS_in * time_span_min);
    uint32_t min_out_samples = (U * min_in_samples) / D;

    std::cerr << SoDa::Format("ReSampler:: min_in_samples = %0 min_out_samples = %1\n").addI(min_in_samples).addI(min_out_samples); 
    while((min_in_samples < 1000) || (min_out_samples < 1000)) {
      // need to goose min_in_samples until it is big enough
      min_in_samples += 1000;
      min_out_samples = (U * min_in_samples) / D;
      std::cerr << SoDa::Format("ReSampler:: adj: min_in_samples = %0 min_out_samples = %1\n").addI(min_in_samples).addI(min_out_samples);       
    }
    
    scale_factor = float(D) / float(U);
    
    auto k = (min_in_samples + D - 1) / D;
    Lx = k * D;
    Ly = k * U;

    std::cerr << SoDa::Format("ReSampler:: Lx = %0 Ly = %1 k = %2\n")
      .addI(Lx)
      .addI(Ly)
      .addI(k);
    // setup the save buffer
    save_count = D;

    // remember our discard
    discard_count = U;


    // now the bucket-copy boundary
    auto extract_buckets = (Lx < Ly) ? Lx : Ly;
    extract_count = (extract_buckets + 1) / 2;

    // now create the low pass filter.
    double cutoff = std::min(FS_in, FS_out) * 0.4;
    // make this fit with a unique ptr
    uint32_t num_taps = uint32_t((FS_in / (0.05 * cutoff)) * (80.0 / 22));
    if((num_taps % 2) == 0) num_taps++;
    if(num_taps < 31) num_taps = 31;
    
    lpf_p = std::unique_ptr<SoDa::Filter>(new SoDa::Filter(-cutoff, cutoff, 0.015 * cutoff, FS_in, 
							   num_taps, Lx));

    // create the input and output buffers
    x.resize(Lx);
    y.resize(Ly);
    X.resize(Lx);
    Y.resize(Ly);
    // zero the input buffer since we use the
    // end of it for the save buffer. 
    for(auto & s : x) {
      s = std::complex<float>(0.0,0.0);
    }

    // create the input and output FFTs.
    in_fft_p = std::unique_ptr<SoDa::FFT>(new SoDa::FFT(Lx));
    out_fft_p = std::unique_ptr<SoDa::FFT>(new SoDa::FFT(Ly));    
    
    // and that's it. 
  }



  uint32_t ReSampler::getInputBufferSize() {
    return Lx - D;
  }

  
  uint32_t ReSampler::getOutputBufferSize() {
    return Ly - U;
  }
  
  
  uint32_t ReSampler::apply(std::vector<std::complex<float>> & in,
			    std::vector<std::complex<float>> & out) {
    if(in.size() != getInputBufferSize()) {
      throw BadBufferSize("Input", in.size(), getInputBufferSize());
    }

    if(out.size() != getOutputBufferSize()) {
      throw BadBufferSize("Output", out.size(), getOutputBufferSize());
    }

    // first do the overlap-and-save thing.
    for(int i = 0; i < save_count; i++) {
      x[i] = x[Lx - save_count + i];
    }
    for(int i = save_count; i < Lx; i++) {
      x[i] = in[i - save_count]; 
    }

    // now do the FFT
    in_fft_p->fft(x, X);

    // apply the filter
    lpf_p->apply(X, X, InOutMode(false,false));

    // now load the output Y vector
    for(int i = 0; i < extract_count; i++) {
      Y[i] = X[i];
      Y[extract_count + i] = X[Lx + i - extract_count];
    }
    
    // do the inverse FFT
    out_fft_p->ifft(Y, y);

    // and copy to the output
    for(int i = 0; i < getOutputBufferSize(); i++) {
      out[i] = y[i + discard_count] * scale_factor;
    }
    
    // and that's it!
    return 0;
  }

  uint32_t ReSampler::apply(float * in,
			    float * out) {
    return 0;
  }
}
