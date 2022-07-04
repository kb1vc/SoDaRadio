#pragma once
/*
Copyright (c) 2012,2013,2014,2022 Matthew H. Reilly (kb1vc)
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


#include <memory>
#include <vector>
#include <complex>

namespace SoDa {
  template<typename T> 
  std::shared_ptr<std::vector<T>> makeVectorBuffer(size_t len) {
    auto ret = std::make_shared<std::vector<T>>(std::vector<T>(len));    
    return ret;
  }

  template<typename T> 
  std::shared_ptr<T> makeBuffer() {
    return std::make_shared<T>(new T()); 
  }

  template<typename T> 
  std::shared_ptr<T> makeBuffer(T * d) {
    return std::make_shared<T>(d); 
  }
  
  typedef std::shared_ptr<std::vector<std::complex<float>>> CFBuf;
  CFBuf makeCFBuf(size_t len);
  
  typedef std::shared_ptr<std::vector<std::complex<double>>> CDBuf;
  CDBuf makeCDBuf(size_t len);
  
  typedef std::shared_ptr<std::vector<float>> FBuf;
  FBuf makeFBuf(size_t len);
  
  typedef std::shared_ptr<std::vector<double>> DBuf;
  DBuf makeDBuf(size_t len);
}
