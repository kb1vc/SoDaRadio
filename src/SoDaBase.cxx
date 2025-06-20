/*
  Copyright (c) 2014, 2025 Matthew H. Reilly (kb1vc)
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

namespace SoDa {
  std::map<std::string, BasePtr> Base::object_directory; 

  Base::Base(const std::string & oname)
  {
    objname = oname;
  }

  void Base::registerSelf(BasePtr me) {
    if(object_directory.find(objname) == object_directory.end()) {
      object_directory[objname] = me;
    }
    
    self = me; 
  }
  
  BasePtr Base::findSoDaObject(const std::string & oname) {
    std::map<std::string, BasePtr>::iterator mi;
    mi = object_directory.find(oname);
    if(mi != object_directory.end()) {
      return mi->second;
    }
    else {
      return NULL; 
    }
  }

  double Base::getTime() {
    struct timespec tp; 
    clock_gettime(CLOCK_MONOTONIC, &tp); // 60nS average in tight loops, 160nS cold.
    double ret = ((double) tp.tv_sec) + (1.0e-9 * ((double) tp.tv_nsec)); 
    if(first_time) {
      base_first_time = ret; 
      first_time = false; 
    }
    return ret - base_first_time; 
  }


  bool Base::first_time = true;
  double Base::base_first_time;
}
