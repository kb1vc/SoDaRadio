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
///  @file FFT.hxx
///  @brief General wrapper for fftw3 or whatever DFT widget we're going to use.
///  Inspired by the numpy fft functions.
///
///  @author M. H. Reilly (kb1vc)
///  @date   July 2022
///

#include <complex>
#include <vector>
#include <fftw3.h>
#include "Radio.hxx"
namespace SoDa {
  class FFT {
  public:
    class UnmatchedSizes : public Radio::Exception {
    public:
      UnmatchedSizes(const std::string & st, unsigned int ins, unsigned int outs) :
	Radio::Exception(SoDa::Format("Vector arguments to function FFT::%0 must be the same size. In.size = %1  Out.size = %2\n")
			 .addS(st).addI(ins).addI(outs).str()) { }
    };
    class BadSize : public Radio::Exception {
    public:
      BadSize(const std::string & st, unsigned int was, unsigned int should_be) :
	Radio::Exception(SoDa::Format("Vector arguments to function FFT::%0 must %2 but were %1 instead\n")
			 .addS(st).addI(was).addI(should_be).str()) { }
    };
    
    FFT(unsigned int len); 
    
    void fft(std::vector<std::complex<float>> & in, 
	     std::vector<std::complex<float>> & out);

    void ifft(std::vector<std::complex<float>> & in, 
	     std::vector<std::complex<float>> & out);

    void shift(std::vector<std::complex<float>> & in, 
	     std::vector<std::complex<float>> & out);

    void ishift(std::vector<std::complex<float>> & in, 
	     std::vector<std::complex<float>> & out);

  protected:
    fftwf_plan forward_plan, backward_plan;
    unsigned int len; 
  };
}

