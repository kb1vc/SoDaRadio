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
// Substantial parts written or modified by Claude Sonnet 4.6 (claude-sonnet-4-6)

#include "RTLSDRRadio.hxx"
#include <SoDa/Format.hxx>
#include <stdexcept>

namespace SoDa {

  RTLSDRRadio::RTLSDRRadio(ParamsPtr params) : Radio(params)
  {
    // Parse device index from radio_args (default 0).
    std::string args = params->getRadioArgs();
    uint32_t index = 0;
    if (!args.empty()) {
      try { index = (uint32_t)std::stoul(args); }
      catch (...) { index = 0; }
    }

    if (rtlsdr_get_device_count() == 0)
      throw std::runtime_error("RTLSDRRadio: no RTL-SDR devices found");

    dev = std::make_shared<RTLSDRDev>(index);
    if (!dev->dev)
      throw std::runtime_error(
        SoDa::Format("RTLSDRRadio: failed to open device index %0").addI((int)index).str());

    ctrl = RTLSDRCtrl::make(dev, params);
    rx   = RTLSDRRX::make(dev, params);
    tx   = RTLSDRTX::make(params);
  }

  void RTLSDRRadio::init()
  {
    ctrl->init();
    rx->init();
    tx->init();
  }

} // namespace SoDa
