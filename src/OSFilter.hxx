#pragma once
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


 ///
 ///  @file OSFilter.hxx
 ///  @brief This is an overlap-and-save frequency domain implementation
 ///  of a general FIR filter widget.
 ///
 ///  @author M. H. Reilly (kb1vc)
 ///  @date   July 2013
 ///

#include <complex>
#include <vector>
#include <memory>
#include "BlockOp.hxx"

namespace SoDa {
  /// Overlap-and-save filter class.  
  class OSFilter {
  public:
    /**
     *
     * @param block_op a pointer to a BlockOp object (like a filter or resampler)
     * @param save_length the number of elements S in the save/overlap buffer. 
     * @param discard_length the number of elements to discard from the output
     * buffer to account for the overlap. If zero, S elements will be discarded. 
     * 
     * The discard_length parameter is used for rational resamplers where we don't
     * discard the number of elements saved at the input stage but rather M * U / D
     * elements at the output stage. 
     *
     */
    OSFilter(std::unique_ptr<BlockOp> block_op, 
	     unsigned int save_length,
	     unsigned int discard_length);

    
    /// run the filter on a complex input stream
    /// @param inbuf the input buffer I/Q samples (complex)
    /// @param outbuf the output buffer I/Q samples (complex)
    /// @param outgain normalized output gain
    /// @return the length of the input buffer
    unsigned int apply(std::vector<std::complex<float>> & inbuf, 
		       std::vector<std::complex<float>> & outbuf, 
		       float outgain = 1.0);

    /// run the filter on a real input stream
    /// @param inbuf the input buffer samples
    /// @param outbuf the output buffer samples (this can overlap the inbuf vector)
    /// @param outgain normalized output gain
    /// @param instride index increment for the input buffer
    /// @param outstride index increment for the output buffer
    /// @return the length of the input buffer
    unsigned int apply(std::vector<float> & inbuf, 
		       std::vector<float> & outbuf, 
		       float outgain = 1.0);

  protected:
    std::unique_ptr<BlockOp> block_op;

    unsigned int save_length;
    unsigned int discard_length;
    
    std::vector<std::complex<float>> c_in_buffer;
    std::vector<float> f_in_buffer;
    std::vector<std::complex<float>> c_out_buffer;
    std::vector<float> f_out_buffer;

  };
}

