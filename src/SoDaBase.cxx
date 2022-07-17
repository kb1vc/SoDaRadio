/*
  Copyright (c) 2014,2022 Matthew H. Reilly (kb1vc)
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

#include "SoDaBase.hxx"
#include <string>
#include <map>
#include <chrono>

namespace SoDa {
  std::map<std::string, SoDa::Base *> SoDa::Base::ObjectDirectory;

  SoDa::Base::Base(const std::string & oname)
  {
    objname = oname;
    if(ObjectDirectory.find(oname) == ObjectDirectory.end()) {
      ObjectDirectory[oname] = this;
    }
  }

  SoDa::Base * SoDa::Base::findSoDaObject(const std::string & oname) {
    std::map<std::string, SoDa::Base *>::iterator mi;
    mi = ObjectDirectory.find(oname);
    if(mi != ObjectDirectory.end()) {
      return mi->second;
    }
    else {
      return NULL; 
    }
  }

  double SoDa::Base::getTime() {
    auto st = std::chrono::system_clock::now().time_since_epoch();
    auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(st).count();
    double ret = (double) nsec;
    
    if(first_time) {
      base_first_time = ret; 
      first_time = false; 
    }
    return ret - base_first_time; 
  }
}

bool SoDa::Base::first_time = true;
double SoDa::Base::base_first_time;
