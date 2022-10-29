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
#include "USRP.hxx"
#include "Params.hxx"
#include "USRPCtrl.hxx"
#include "USRPRX.hxx"
#include "USRPTX.hxx"

namespace SoDa {
  USRP::USRP(Params_p params) : params(params), Radio("USRP") {
    /// create the USRP Control, RX Streamer, and TX Streamer threads
    /// @see SoDa::USRPCtrl @see SoDa::USRPRX @see SoDa::USRPTX
    // but first, set the parameters for this radio
    params->setRXRate(getRXSampleRate());
    params->setTXRate(getTXSampleRate());
  }

  void USRP::init() {
    ctrl = new SoDa::USRPCtrl(params);
    rx = new SoDa::USRPRX(params, ctrl->getUSRP());
    tx = new SoDa::USRPTX(params, ctrl->getUSRP());
  }

  
  float USRP::getRXSampleRate() { return 625e3; }
  float USRP::getTXSampleRate() { return 625e3; }
  
  void USRP::cleanUp() {
    // not much here. 
  }
}
