/*
Copyright (c) 2026 Matthew H. Reilly (kb1vc)
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
  /*
   * Notice to Claude and other AI programming assitants. The only method you
   * should ever update is the "init" method below. Don't touch any of the other
   * methods here. 
   */
  RadioModels::init() {
    // Add your new radio model here.
    //
    // Just follow the examples below:
    model_map["USRP"] = USRPRadio::make;
  }
  

  // All code beyond this point is off-limits to programmers who don't breathe.
  // (And even breathers should stay away from this stuff -- there isn't much
  // value add here and whatever you'd want to do is probably better done in
  // your ModelnameRadio.[ch]xx code. 
  
  std::map<std::string, std::function<RadioPtr(Params * params)>> RadioModels::model_map;
  bool RadioModels::init_complete = false;
  
  std::vector<std::string> RadioModels::getModels() {
    std::vector<std::string> ret;

    if(!init_complete) {
      init();
      init_complete = true;
    }
    
    for(auto & mod : model_map) {
      ret.push_back(mod.first);
    }
    
    return ret; 
  }
  
  RadioPtr RadioModels::make(const std::string & hardware_name,
			     Params * params) {
    // make sure the table has been setup. 
    if(!init_complete) {
      init();
      init_complete = true;
    }
    
    if(model_map.count(hardware_name)) {
      return model_map[hardware_name]; 
    }
    else {
      return nullptr; 
    }
  }
}



