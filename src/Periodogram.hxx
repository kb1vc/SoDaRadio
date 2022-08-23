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


///
///  @file Periodogram.hxx
///  @brief Overlapped cumulative periodogram
///
///  @author M. H. Reilly (kb1vc)
///  @date   July 2022
///

#include <complex>
#include <vector>
#include "FFT.hxx"
#include "Radio.hxx"
namespace SoDa {
  class Periodogram {
  public:
    /**
     * @brief constructor
     *
     * The periodogram is constructed by sequentially accumulating windowed FFTs over
     * fixed segments. Segments are overlapped by 1/2 the segment length; 
     * 
     * @param segment_length The periodogram will be calculated over "windows" of this length
     * @param alpha if zero, add each segment's contribution to the accumulated result. 
     * otherwise acc = alpha * acc + (1 - alpha) * contrib
     */
    Periodogram(unsigned int segment_length, float alpha = 0.0); 

    /**
     * @brief add the input buffer's spectrum to the accumulated spectrum. 
     * 
     * @param in the input vector. This need not be a multiple of the segment_length
     */
    void accumulate(std::vector<std::complex<float>> & in);

    /**
     * @brief return the current accumulated periodogram
     *
     * @return a reference to the accumulator
     */
    const std::vector<float> & get() const;
    
    /**
     * @brief the magitude of the accumulator may increase with each
     * accumulated segment. This returns a scale factor that will restore
     * the magnitudes to "true" levels independent of the number of accumulated segments. 
     */
    float getScaleFactor(); 

    /**
     * @brief clear the state of the periodogram -- zero out the accumulator, 
     * and empty the input save buffer. 
     */
    void clear(); 

    //  protected:
    std::unique_ptr<FFT> fft_p;
    float alpha, beta;     
    uint32_t segment_length; 
    std::vector<std::complex<float>> input_save_buffer;
    uint32_t input_save_buffer_valid_count; 
    std::vector<float> acc_buffer;
    std::vector<std::complex<float>> fft_in_buffer;
    std::vector<std::complex<float>> fft_out_buffer;        
    std::vector<float> window; 
    uint32_t accumulation_count;
    float fft_scale; 
  };
}

