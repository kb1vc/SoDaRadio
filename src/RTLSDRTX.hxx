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
 * @file RTLSDRTX.hxx
 *
 * @brief Stub RadioTX for RTL-SDR dongles (RX-only hardware).
 *
 * The RTL-SDR has no transmit capability.  This class satisfies the
 * RadioTX interface so the rest of SoDaServer can treat RTL-SDR like
 * any other radio model.  All TX operations are silent no-ops.
 *
 * @author M. H. Reilly (kb1vc)
 * @date   June 2026
 */

#include "RadioTX.hxx"
#include "Params.hxx"
#include <memory>

namespace SoDa {

  class RTLSDRTX;
  using RTLSDRTXPtr = std::shared_ptr<RTLSDRTX>;

  class RTLSDRTX : public RadioTX {
  protected:
    RTLSDRTX(ParamsPtr params) : RadioTX(params, "RTLSDRTX") { }

  public:
    ~RTLSDRTX() = default;

    static RTLSDRTXPtr make(ParamsPtr params) {
      auto ret = std::shared_ptr<RTLSDRTX>(new RTLSDRTX(params));
      ret->self = ret;
      ret->registerThread(ret);
      return ret;
    }

    void init() override { }

    /// RTL-SDR cannot transmit — always report TX off.
    bool transmitSwitch(bool) override { return false; }

    /// Accept but discard all TX sample buffers.
    bool put(CBufPtr) override { return true; }

    RadioTXPtr getSelfPtr() override { return self.lock(); }

  private:
    std::weak_ptr<RTLSDRTX> self;
  };

} // namespace SoDa
