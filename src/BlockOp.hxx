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
/// @file BlockOp.hxx
/// @brief This pure virtual class describes the interface specification for any process
/// that may be adapted to an overlap-and-save scheme. That is, we implement
/// filters and resamplers as block operations that are used as kernels in
/// overlap-and-save filters. 
///
/// To maintain a consistent interface, especially for filters where we may want
/// to use the filter in a block mode where we desire the return value (or input value)
/// to be in the frequency domain, the apply virtual functions are more elaborate than
/// is necessary to support an OSfilter scheme. In all cases, the OSFilter framework
/// will only apply the filters in time-domain in / time-domain out mode. 
///
///  @author M. H. Reilly (kb1vc)
///  @date   July 2022
///

#include <memory>
#include <iostream>
#include <complex>
#include <vector>
#include <fftw3.h>

namespace SoDa {

  // all "apply" functions can bypass one or more of the input and output conversions
  // into the frequency domain. 
  enum INOUT_MODE { TIME_TIME, TIME_FFT, FFT_TIME, FFT_FFT };
  
  class BlockOp {
  public:
    /**
     * @constructor nothing to see here. 
     */
    BlockOp() {}

    /** 
     */
    /** run the filter on a complex input stream
     * @param in_buf the input buffer I/Q samples (complex)
     * @param out_buf the output buffer I/Q samples (complex)
     * @param outgain normalized output gain
     * @param in_out_mode input or output can be time samples or frequency (FFT format) samples
     * @return the length of the input buffer
     */
    unsigned int apply(std::vector<std::complex<float>> & in_buf, 
		       std::vector<std::complex<float>> & out_buf, 
		       float outgain = 1.0,
		       INOUT_MODE in_out_mode = TIME_TIME) = 0;

    /** run the filter on a real input stream
     * @param in_buf the input buffer samples
     * @param out_buf the output buffer samples (this can overlap the in_buf vector)
     * @param outgain normalized output gain
     * @param in_out_mode input or output can be time samples or frequency (FFT format) samples
     * @return the length of the input buffer
     *
     * @throws Filter::BadRealFilter if the original filter spec was not "real"
     */
    unsigned int apply(std::vector<float> & in_buf, 
		       std::vector<float> & out_buf, 
		       float out_gain = 1.0,
		       INOUT_MODE in_out_mode = TIME_TIME) = 0;
  };
}

