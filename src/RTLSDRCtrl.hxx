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
 * @file RTLSDRCtrl.hxx
 *
 * @brief RadioControl implementation for RTL-SDR dongles (librtlsdr).
 *
 * The RTL-SDR is receive-only.  All TX-related virtual methods are
 * implemented as harmless stubs.  Frequency range depends on the
 * specific tuner chip (R820T: ~24 MHz – 1.8 GHz is typical).
 *
 * radio_args from Params is interpreted as a decimal device index
 * (e.g. "0" for the first dongle).  An empty string also selects index 0.
 *
 * @author M. H. Reilly (kb1vc)
 * @date   June 2026
 */

#include "RadioControl.hxx"
#include "Params.hxx"
#include "RTLSDRDev.hxx"
#include <vector>
#include <string>

namespace SoDa {

  class RTLSDRCtrl;
  using RTLSDRCtrlPtr = std::shared_ptr<RTLSDRCtrl>;

  class RTLSDRCtrl : public RadioControl {
  protected:
    RTLSDRCtrl(RTLSDRDevPtr dev_in, ParamsPtr params);

  public:
    ~RTLSDRCtrl() = default;

    static RTLSDRCtrlPtr make(RTLSDRDevPtr dev_in, ParamsPtr params) {
      auto ret = std::shared_ptr<RTLSDRCtrl>(new RTLSDRCtrl(dev_in, params));
      ret->self = ret;
      ret->registerThread(ret);
      return ret;
    }

    RadioControlPtr getSelfPtr() override { return self.lock(); }

    // ---------------------------------------------------------------
    // RadioControl pure-virtual interface
    // ---------------------------------------------------------------

    bool isLOLocked(SoDa::RXTX rxtx) override;

    double setLOFreq(double freq, SoDa::RXTX rxtx) override;
    double getLOFreq(SoDa::RXTX rxtx) override;

    float setRFGain(float gain, SoDa::RXTX rxtx) override;

    void  setSampleRate(float rate, SoDa::RXTX rxtx) override;
    float getSampleRate(SoDa::RXTX rxtx) override;

    std::string getHardwareDescription() override;

    std::vector<std::string> listAntennas(SoDa::RXTX rxtx) override;
    void        setAntenna(const std::string & ant, SoDa::RXTX rxtx) override;
    std::string getAntenna(SoDa::RXTX rxtx) override;

    // TX is not supported — stubs only.
    bool getTXEna() override     { return false; }
    bool getTXRelayOn() override { return false; }
    void setTXEna(bool, bool) override { }

  private:
    static constexpr float HW_RATE = 2048000.0f;  ///< RTL2832U actual USB delivery rate (2.048 MSPS)

    RTLSDRDevPtr rtl;

    double rx_lo_freq;         ///< last RX center frequency written (Hz)
    float  rx_sample_rate;     ///< current sample rate (Hz)

    std::vector<int> gain_steps; ///< valid tuner gains in tenths of dB
    std::string hw_description;

    std::weak_ptr<RTLSDRCtrl> self;

    /// Map SoDaRadio gain (-30..0 dB) to the nearest discrete RTL gain step.
    int sodaGainToRTL(float soda_gain) const;
  };

} // namespace SoDa
