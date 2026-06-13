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
 * @file PlutoRadio.hxx
 *
 * @brief Aggregate all hardware-specific threads for the ADALM-PLUTO.
 *
 * Creates PlutoCtrl (PHY/LO/gain via ad9361-phy), PlutoRX (IIO RX streaming
 * via cf-ad9361-lpc), and PlutoTX (IIO TX streaming via cf-ad9361-dds-core-lpc).
 * Each thread owns its own iio_context connection so libiio's per-context
 * locking is not a bottleneck.
 *
 * @author M. H. Reilly (kb1vc)
 * @date   June 2026
 *
 * @note Substantial parts written or modified by Claude Sonnet 4.6 (claude-sonnet-4-6)
 */

#include "Radio.hxx"
#include "Params.hxx"
#include "PlutoCtrl.hxx"
#include "PlutoRX.hxx"
#include "PlutoTX.hxx"

#include <memory>

namespace SoDa {

  class PlutoRadio;
  typedef std::shared_ptr<PlutoRadio> PlutoRadioPtr;

  class PlutoRadio : public Radio {
  protected:
    PlutoRadio(ParamsPtr params);

  public:
    static RadioPtr make(ParamsPtr params) {
      return std::shared_ptr<Radio>(new PlutoRadio(params));
    }

    void init();

  private:
    PlutoCtrlPtr ctrl;
    PlutoRXPtr   rx;
    PlutoTXPtr   tx;
  };
}
