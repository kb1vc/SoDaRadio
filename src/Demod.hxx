/*
  Copyright (c) 2015, Matthew H. Reilly (kb1vc)
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

#ifndef DEMOD_HDR
#define DEMOD_HDR
#include "SoDaBase.hxx"
#include "Debug.hxx"

namespace SoDa {
  /**
   * @class Demod
   * @brief Demodulator base class -- all others build on this.
   */
  class Demod : public Debug, public SoDaBase { 
  public:
    Demod(const std::string & demod_name, bool _is_audio) : SoDaBase(demod_name) {
      is_audio = _is_audio; 
      name = demod_name;
      demodulator_map[name] = this;
    }

    static Demod * getDemodulator(const std::string & mod_type) {
      if(demodulator_map.find(mod_type) != demodulator_map.end()) {
	return demodulator_map[mod_type]; 
      }
      else {
	return NULL; 
      }
    }

    virtual bool demodAudio(SoDaBuf * in, float * out, float gain) {
      return false; 
    }
    virtual bool demodDigital(SoDaBuf * in, std::vector<char> out) {
      return false;
    }

  protected:
    std::string name; 
    bool is_audio; 
  private:
    static std::map<std::string, Demod *> demodulator_map; 

  }; 

  class DemodSSB : public Demod {
  public:
    DemodSSB(bool _is_usb) : Demod(_is_usb ? "USB" : "LSB", true) {
      is_usb = _is_usb; 
    }
  protected:
    bool is_usb; 
  };

  class DemodUSB : public DemodSSB {
  public:
    DemodUSB() : DemodSSB(true) { }

    bool demodAudio(SoDaBuf * in, float * out, float gain) {
      std::cerr << "Here in DemodUSB" << std::endl;
      return true;      
    }
  }; 

  class DemodLSB : public DemodSSB {
  public:
    DemodLSB() : DemodSSB(false) { }
    
    bool demodAudio(SoDaBuf * in, float * out, float gain) {
      std::cerr << "Here in DemodLSB" << std::endl; 
      return true;
    }
  }; 
}

#endif
