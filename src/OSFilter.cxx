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


namespace SoDa {
  OSFilter::OSFilter(std::unique_ptr<BlockOp> block_op, 
	     unsigned int save_length,
		     unsigned int discard_length) : 
    block_op(block_op), save_length(save_length),
    discard_length(discard_length)
  {
    if(discard_length == 0) {
      discard_length = save_length;
    }
  }

  template<typename T> 
  unsigned int generic_apply(std::unique_ptr<BlockOp> & block_op, 
			     std::vector<T> & inbuf,
			     std::vector<T> & outbuf,
			     std::vector<T> & loc_inbuf,
			     std::vector<T> & loc_outbuf,			     
			     float outgain) {
    // is the input buffer sufficiently large to at least
    // produce a save segment?
    if(inbuf.size() < save_length) {
      throw InputBufShort(save_length, inbuf.size());
    }
    
    // create the input buffer from our saved stuff and the new stuff.
    if(loc_inbuf.size() != (save_length + inbuf.size())) {
      loc_inbuf.resize(save_length + inbuf.size());
    }

    if(loc_outbuf.size() != (block_op->outLenReq(loc_inbuf_size()))) {
      loc_outbuf.resize(block_op->outLenReq(loc_inbuf_size()));
    }

    // fill the input buffer from the end of the save block
    for(int i = 0; i < inbuf.size()) {
      loc_inbuf[i + save_length] = inbuf[i];;
    }

    // apply the operator
    block_op->apply(loc_inbuf, loc_outbuf, outgain);

    // how many samples will we produce?
    unsigned int result_samples = loc_outbuf.size() - discard_length;
    // is the output buffer sufficiently large?
    if(outbuf.size() < result_samples) {
      throw OutputBufShort(result_samples, outbuf.size());
    }

    // discard the resulting samples
    for(int i = 0; i < outbuf.size(); i++) {
      outbuf[i] = loc_outbuf[i + discard_length]; 
    }

    // save the overlap samples
    for(int i = 0; i < save_length; i++) {
      loc_inbuf[i] = inbuf[i + inbuf.size() - save_length];
    }
    
    return outbuf.size();
  }
			     
    
  unsigned int OSFilter::apply(std::vector<std::complex<float>> & inbuf, 
			       std::vector<std::complex<float>> & outbuf, 
			       float outgain = 1.0) {
    return generic_apply<std::complex<float>>(block_op, 
					      inbuf, outbuf,
					      c_in_buffer, c_out_buffer,
					      outgain);
  }

  unsigned int OSFilter::apply(std::vector<float> & inbuf, 
			       std::vector<float> & outbuf, 
			       float outgain = 1.0) {
    return generic_apply<float>(block_op, 
				inbuf, outbuf,
				f_in_buffer, f_out_buffer,
				outgain);
  }
}
