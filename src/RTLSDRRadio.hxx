#pragma once
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

/**
 * @file RTLSDRRadio.hxx
 *
 * @brief Aggregate all threads for an RTL-SDR dongle.
 *
 * Opens the device handle (RTLSDRDev) once and passes it to both
 * RTLSDRCtrl and RTLSDRRX.  RTLSDRTX is a stub (RX-only hardware).
 *
 * radio_args from Params is the dongle index (e.g. "0").
 *
 * @author M. H. Reilly (kb1vc)
 * @date   June 2026
 */

#include "Radio.hxx"
#include "Params.hxx"
#include "RTLSDRDev.hxx"
#include "RTLSDRCtrl.hxx"
#include "RTLSDRRX.hxx"
#include "RTLSDRTX.hxx"
#include <memory>

namespace SoDa {

  class RTLSDRRadio;
  using RTLSDRRadioPtr = std::shared_ptr<RTLSDRRadio>;

  class RTLSDRRadio : public Radio {
  protected:
    RTLSDRRadio(ParamsPtr params);

  public:
    static RadioPtr make(ParamsPtr params) {
      return std::shared_ptr<Radio>(new RTLSDRRadio(params));
    }

    void init();

  private:
    RTLSDRDevPtr     dev;
    RTLSDRCtrlPtr    ctrl;
    RTLSDRRXPtr      rx;
    RTLSDRTXPtr      tx;
  };

} // namespace SoDa
