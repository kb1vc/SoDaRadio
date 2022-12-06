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
#include "RTLsdr.hxx"
#include "Params.hxx"
#include "RTLsdrCtrl.hxx"
#include "RTLsdrRX.hxx"
#include "RTLsdrTX.hxx"

namespace SoDa {
  RTLsdr::RTLsdr(Params_p params) : params(params), Radio("RTLsdr") {
    // set the parameters for this radio
    params->setRXRate(getRXSampleRate());
    params->setTXRate(getTXSampleRate());
    
  }

  void RTLsdr::init() {
    ctrl = new SoDa::RTLsdrCtrl(params);
    rx = new SoDa::RTLsdrRX(params);
    tx = new SoDa::RTLsdrTX(params);
  }

  static Radio * makeRTLsdr(Params_p parms) { return new RTLsdr(parms); }
  
  float RTLsdr::getRXSampleRate() { return 1e6; }
  float RTLsdr::getTXSampleRate() { return 1e6; }
  
  void RTLsdr::cleanUp() {
    // not much here. 
  }
}
