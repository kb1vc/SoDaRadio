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

namespace SoDa {
  FFT::FFT(unsigned int len) : len(len) {
    f_dummy_in = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * dim);
    f_dummy_out = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * dim);  
  
    forward_plan = fftwf_plan_dft_1d(len, f_dummy_in, f_dummy_out, FFTW_FORWARD);
    backward_plan = fftwf_plan_dft_1d(len, f_dummy_in, f_dummy_out, FFTW_BACKWARD);

    fftw_free(f_dummy_in);
    fftw_free(f_dummy_out);
  }
    
  void FFT::fft(std::vector<std::complex<float>> & in, 
		std::vector<std::complex<float>> & out) {
    if(in.size() != out.size()) {
      throw UnmatchedSizes("fft", in.size(), out.size());
    }
    if(in.size() != len) {
      throw BadSize("fft", in.size(), len);
    }
    fftw_execute_dft(forward_plan, in.data(), out.data());
  }

  void FFT::ifft(std::vector<std::complex<float>> & in, 
		 std::vector<std::complex<float>> & out) {
    if(in.size() != out.size()) {
      throw UnmatchedSizes("fft", in.size(), out.size());
    }
    if(in.size() != len) {
      throw BadSize("fft", in.size(), len);
    }
    fftw_execute_dft(backward_plan, in.data(), out.data());
  }
  
  void FFT::shift(std::vector<std::complex<float>> & in, 
		  std::vector<std::complex<float>> & out) {
    // the inputs must be the same size
    if(in.size() != out.size()) {
      throw UnmatchedSizes("shift", in.size(), out.size());
    }
    // take the middle and shift it down
    // the two vectors must be distinct.
    unsigned int mid = (in.size() - 1) / 2;
    unsigned int mod = in.size();
    for(int i = 0; i < in.size(); i++) {
      out[(mid + i) % mod] = in[i]; 
    }
  }
  
  void FFT::ishift(std::vector<std::complex<float>> & in, 
		   std::vector<std::complex<float>> & out) {
    // the inputs must be the same size
    if(in.size() != out.size()) {
      throw UnmatchedSizes("ishift", in.size(), out.size());
    }
    // take the middle and shift it down
    // the two vectors must be distinct.
    unsigned int mid = (in.size() + 1) / 2;
    unsigned int mod = in.size();
    for(int i = 0; i < in.size(); i++) {
      out[(mid + i) % mod] = in[i]; 
    }
  }

}

