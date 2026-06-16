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

#include "RTLSDRCtrl.hxx"
#include <SoDa/Format.hxx>
#include <algorithm>
#include <cmath>
#include <climits>
#include <sys/time.h>

namespace SoDa {

  RTLSDRCtrl::RTLSDRCtrl(RTLSDRDevPtr dev_in, ParamsPtr _params)
    : RadioControl(_params, "RTLSDRCtrl"),
      rtl(dev_in),
      rx_lo_freq(0.0),
      rx_sample_rate(2048000.0f)
  {
    if (!rtl || !rtl->dev)
      throw SDR::Exception(
        SoDa::Format("RTLSDRCtrl: device handle is null\n"), self.lock());

    // Manual tuner gain mode — SoDaRadio owns the gain value.
    rtlsdr_set_tuner_gain_mode(rtl->dev, 1);
    // Disable RTL AGC.
    rtlsdr_set_agc_mode(rtl->dev, 0);

    // Enumerate the discrete gain steps supported by this tuner.
    int n = rtlsdr_get_tuner_gains(rtl->dev, nullptr);
    if (n > 0) {
      gain_steps.resize(n);
      rtlsdr_get_tuner_gains(rtl->dev, gain_steps.data());
    }

    // Set default sample rate.
    rtlsdr_set_sample_rate(rtl->dev, (uint32_t)rx_sample_rate);
    rx_sample_rate = (float)rtlsdr_get_sample_rate(rtl->dev);

    motherboard_name = rtl->name;
    hw_description = SoDa::Format("RTL-SDR [%0] (RX only)").addS(rtl->name).str();

    first_gettime = 0.0;
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    first_gettime = (double)tv.tv_sec;

    debugMsg(SoDa::Format("RTLSDRCtrl: opened [%0] sample_rate=%1\n")
             .addS(rtl->name).addF(rx_sample_rate, 'e'));
  }

  // ---------------------------------------------------------------
  // Private helpers
  // ---------------------------------------------------------------

  int RTLSDRCtrl::sodaGainToRTL(float soda_gain) const
  {
    // SoDa gain: -30..0 dB (0 = maximum).
    // RTL gains: list of values in tenths of dB (e.g. 496 = 49.6 dB).
    // Map: soda 0 → highest RTL gain, soda -30 → lowest RTL gain.
    if (gain_steps.empty()) return 0;

    int max_rtl = *std::max_element(gain_steps.begin(), gain_steps.end());
    int min_rtl = *std::min_element(gain_steps.begin(), gain_steps.end());
    float rtl_range = (float)(max_rtl - min_rtl);

    // Linear map from [-30,0] → [min_rtl, max_rtl].
    float target = (float)min_rtl + (soda_gain + 30.0f) / 30.0f * rtl_range;

    // Find closest available step.
    int best = gain_steps[0];
    float best_dist = std::abs(target - (float)best);
    for (int g : gain_steps) {
      float d = std::abs(target - (float)g);
      if (d < best_dist) { best_dist = d; best = g; }
    }
    return best;
  }

  // ---------------------------------------------------------------
  // LO frequency
  // ---------------------------------------------------------------

  double RTLSDRCtrl::setLOFreq(double freq, SoDa::RXTX rxtx)
  {
    if (rxtx != SoDa::RX) return 0.0;  // no TX

    rtlsdr_set_center_freq(rtl->dev, (uint32_t)std::round(freq));
    rx_lo_freq = (double)rtlsdr_get_center_freq(rtl->dev);
    debugMsg(SoDa::Format("RTLSDRCtrl: RX LO set to %0 actual %1\n")
             .addF(freq, 'e').addF(rx_lo_freq, 'e'));
    return rx_lo_freq;
  }

  double RTLSDRCtrl::getLOFreq(SoDa::RXTX rxtx)
  {
    if (rxtx != SoDa::RX) return 0.0;
    return (double)rtlsdr_get_center_freq(rtl->dev);
  }

  bool RTLSDRCtrl::isLOLocked(SoDa::RXTX /* rxtx */)
  {
    // The RTL2832 PLL locks within a few milliseconds.  There is no
    // programmatic lock-detect API in librtlsdr; treat it as always locked.
    return true;
  }

  // ---------------------------------------------------------------
  // Gain
  // ---------------------------------------------------------------

  float RTLSDRCtrl::setRFGain(float gain, SoDa::RXTX rxtx)
  {
    if (rxtx != SoDa::RX) return 0.0f;

    int rtl_gain = sodaGainToRTL(gain);
    rtlsdr_set_tuner_gain(rtl->dev, rtl_gain);
    int actual = rtlsdr_get_tuner_gain(rtl->dev);
    rx_rf_gain = gain;
    debugMsg(SoDa::Format("RTLSDRCtrl: RX gain set to %0 (tenths dB) requested %1\n")
             .addI(actual).addI(rtl_gain));
    // Return actual gain in the SoDa convention (dB relative to max).
    if (gain_steps.empty()) return gain;
    int max_rtl = *std::max_element(gain_steps.begin(), gain_steps.end());
    int min_rtl = *std::min_element(gain_steps.begin(), gain_steps.end());
    float rtl_range = (float)(max_rtl - min_rtl);
    return (rtl_range > 0.0f)
      ? -30.0f + ((float)(actual - min_rtl) / rtl_range) * 30.0f
      : gain;
  }

  // ---------------------------------------------------------------
  // Sample rate
  // ---------------------------------------------------------------

  void RTLSDRCtrl::setSampleRate(float rate, SoDa::RXTX rxtx)
  {
    if (rxtx != SoDa::RX) return;
    rtlsdr_set_sample_rate(rtl->dev, (uint32_t)std::round(rate));
    rx_sample_rate = (float)rtlsdr_get_sample_rate(rtl->dev);
  }

  float RTLSDRCtrl::getSampleRate(SoDa::RXTX rxtx)
  {
    if (rxtx != SoDa::RX) return 0.0f;
    return rx_sample_rate;
  }

  // ---------------------------------------------------------------
  // Antennas
  // ---------------------------------------------------------------

  std::vector<std::string> RTLSDRCtrl::listAntennas(SoDa::RXTX rxtx)
  {
    if (rxtx == SoDa::RX) return { "RX" };
    return { };
  }

  void RTLSDRCtrl::setAntenna(const std::string & /* ant */, SoDa::RXTX /* rxtx */) { }

  std::string RTLSDRCtrl::getAntenna(SoDa::RXTX rxtx)
  {
    return (rxtx == SoDa::RX) ? "RX" : "";
  }

  // ---------------------------------------------------------------
  // Hardware description
  // ---------------------------------------------------------------

  std::string RTLSDRCtrl::getHardwareDescription() { return hw_description; }

} // namespace SoDa
