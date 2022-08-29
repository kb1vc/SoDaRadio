/*
Copyright (c) 2022, Matthew H. Reilly (kb1vc)
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

#include "RadioRegistry.hxx"
#include <iostream>

namespace SoDa {
  RadioRegistry::RadioRegistry() { }

  void RadioRegistry::add(const std::string & name, RadioBuilderFunc builder) {
    builders[name] = builder; 
  }

  RadioRegistry::RadioBuilderFunc & RadioRegistry::get(const std::string & name) {
    if(builders.find(name) == builders.end()) {
      throw ModelNotFound(name, supportedRadios());
    }
    return builders[name];
  }

  Radio * RadioRegistry::make(const std::string & name, 
			      Params_p parms) {
    auto mkr = get(name); 
    return mkr(parms);
  }

  std::string RadioRegistry::supportedRadios() {
    std::string ret; 
    std::string sep("");
    for(auto e : builders) {
      ret = ret + sep + e.first; 
      sep = ", "; 
    }
    return ret; 
  }
}
