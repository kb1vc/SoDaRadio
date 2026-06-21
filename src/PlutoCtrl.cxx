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

#include "PlutoCtrl.hxx"
#include "SoDaBase.hxx"
#include <SoDa/Format.hxx>
#include <algorithm>
#include <cmath>
#include <sys/time.h>

namespace SoDa {

  // C++11 out-of-class definitions required when constexpr members are ODR-used
  constexpr double PlutoCtrl::RX_GAIN_MAX;
  constexpr double PlutoCtrl::TX_GAIN_MIN;
  constexpr double PlutoCtrl::FREQ_MIN;
  constexpr double PlutoCtrl::FREQ_MAX;
  constexpr float  PlutoCtrl::PLUTO_SAMPLE_RATE;

  // ---------------------------------------------------------------
  // Constructor
  // ---------------------------------------------------------------

  PlutoCtrl::PlutoCtrl(ParamsPtr _params)
    : RadioControl(_params, "PlutoCtrl"),
      ctx(nullptr), phy(nullptr),
      rx_lo_chan(nullptr), tx_lo_chan(nullptr),
      rx_phy_chan(nullptr), tx_phy_chan(nullptr),
      rx_lo_freq(0.0), tx_lo_freq(0.0),
      rx_samp_rate_hw(625000.0f), tx_samp_rate_hw(625000.0f)
  {
    // Build the IIO URI from radio_args.
    // Accept a bare IP ("192.168.2.1"), a full URI ("ip:192.168.2.1", "usb:"),
    // or empty (use Pluto factory default).
    std::string args = _params->getRadioArgs();
    std::string uri;
    if (args.find(':') != std::string::npos) {
      uri = args;
    } else if (!args.empty()) {
      uri = "ip:" + args;
    } else {
      uri = "ip:192.168.2.1";
    }

    ctx = iio_create_context_from_uri(uri.c_str());
    if (ctx == nullptr) {
      throw SDR::Exception(
        SoDa::Format("PlutoCtrl: cannot open IIO context at [%0]\n").addS(uri),
        self.lock());
    }

    phy = iio_context_find_device(ctx, "ad9361-phy");
    if (phy == nullptr) {
      iio_context_destroy(ctx);
      ctx = nullptr;
      throw SDR::Exception(
        SoDa::Format("PlutoCtrl: ad9361-phy not found at [%0]\n").addS(uri),
        self.lock());
    }

    // altvoltage0 = RX LO (IIO "output" because it drives the mixer)
    rx_lo_chan = iio_device_find_channel(phy, "altvoltage0", true);
    // altvoltage1 = TX LO
    tx_lo_chan = iio_device_find_channel(phy, "altvoltage1", true);
    // voltage0 input = RX sample-path configuration
    rx_phy_chan = iio_device_find_channel(phy, "voltage0", false);
    // voltage0 output = TX sample-path configuration
    tx_phy_chan = iio_device_find_channel(phy, "voltage0", true);

    if (!rx_lo_chan || !tx_lo_chan || !rx_phy_chan || !tx_phy_chan) {
      iio_context_destroy(ctx);
      ctx = nullptr;
      throw SDR::Exception(
        SoDa::Format("PlutoCtrl: one or more required IIO channels missing on ad9361-phy\n"),
        self.lock());
    }

    // Force manual gain control so SoDaRadio owns the RX gain value.
    iio_channel_attr_write(rx_phy_chan, "gain_control_mode", "manual");

    // Mute TX at startup (maximum attenuation = minimum power).
    writeHWTXGain(TX_GAIN_MIN);

    // The Pluto has a single SMA port per direction.
    rx_antenna_name = "RX";
    tx_antenna_name = "TX";
    motherboard_name = "ADALM-PLUTO";

    hw_description = SoDa::Format("ADALM-PLUTO (AD9361) @ %0  %1 – %2 MHz")
      .addS(uri)
      .addF(FREQ_MIN * 1e-6, 'f', 0, 0)
      .addF(FREQ_MAX * 1e-6, 'f', 0, 0)
      .str();

    first_gettime = 0.0;
    first_gettime = std::floor(getTime());

    debugMsg(SoDa::Format("PlutoCtrl: connected to %0\n").addS(uri));
  }

  void PlutoCtrl::init()
  {
    // Set 2.5 MSPS on the hardware NOW, before PlutoTX::init() creates the
    // IIO DMA buffer.  If the buffer is created before the sample rate is
    // written to ad9361-phy, iio_buffer_push() silently fails.
    setSampleRate(params->getRXRate(), SoDa::RX);
    setSampleRate(params->getTXRate(), SoDa::TX);
    std::cerr << SoDa::Format("PlutoCtrl::init() set sample rate RX=%0 TX=%1\n")
      .addF(params->getRXRate(), 'g').addF(params->getTXRate(), 'g');
  }

  PlutoCtrl::~PlutoCtrl()
  {
    if (tx_phy_chan) writeHWTXGain(TX_GAIN_MIN);
    if (rx_lo_chan)  iio_channel_attr_write(rx_lo_chan, "powerdown", "1");
    if (tx_lo_chan)  iio_channel_attr_write(tx_lo_chan, "powerdown", "1");
    if (ctx) {
      iio_context_destroy(ctx);
      ctx = nullptr;
    }
  }

  // ---------------------------------------------------------------
  // Private helpers
  // ---------------------------------------------------------------

  void PlutoCtrl::writeHWTXGain(double gain_db)
  {
    gain_db = std::max(TX_GAIN_MIN, std::min(0.0, gain_db));
    int ret = iio_channel_attr_write_double(tx_phy_chan, "hardwaregain", gain_db);
    if (ret < 0) {
      std::cerr << SoDa::Format("PlutoCtrl: writeHWTXGain(%0) FAILED ret=%1\n")
        .addF(gain_db, 'f', 2).addI(ret);
    }
    double actual = gain_db;
    iio_channel_attr_read_double(tx_phy_chan, "hardwaregain", &actual);
    std::cerr << SoDa::Format("PlutoCtrl: TX hardwaregain set=%0  actual=%1\n")
      .addF(gain_db, 'f', 2).addF(actual, 'f', 2);
  }

  void PlutoCtrl::writeHWRXGain(double gain_db)
  {
    gain_db = std::max(0.0, std::min(RX_GAIN_MAX, gain_db));
    iio_channel_attr_write_double(rx_phy_chan, "hardwaregain", gain_db);
  }

  // ---------------------------------------------------------------
  // LO frequency
  // ---------------------------------------------------------------

  double PlutoCtrl::setLOFreq(double freq, SoDa::RXTX rxtx)
  {
    freq = std::max(FREQ_MIN, std::min(FREQ_MAX, freq));
    long long ifreq = (long long)std::round(freq);
    long long actual = ifreq;

    if (rxtx == SoDa::RX) {
      iio_channel_attr_write_longlong(rx_lo_chan, "frequency", ifreq);
      iio_channel_attr_read_longlong(rx_lo_chan,  "frequency", &actual);
      rx_lo_freq = (double)actual;
      debugMsg(SoDa::Format("PlutoCtrl: RX LO set to %0 actual %1\n")
               .addF(freq, 'e').addF(rx_lo_freq, 'e'));
      return rx_lo_freq;
    }
    else {
      iio_channel_attr_write_longlong(tx_lo_chan, "frequency", ifreq);
      iio_channel_attr_read_longlong(tx_lo_chan,  "frequency", &actual);
      tx_lo_freq = (double)actual;
      debugMsg(SoDa::Format("PlutoCtrl: TX LO set to %0 actual %1\n")
               .addF(freq, 'e').addF(tx_lo_freq, 'e'));
      return tx_lo_freq;
    }
  }

  double PlutoCtrl::getLOFreq(SoDa::RXTX rxtx)
  {
    long long actual = 0;
    if (rxtx == SoDa::RX) {
      iio_channel_attr_read_longlong(rx_lo_chan, "frequency", &actual);
    } else {
      iio_channel_attr_read_longlong(tx_lo_chan, "frequency", &actual);
    }
    return (double)actual;
  }

  bool PlutoCtrl::isLOLocked(SoDa::RXTX /* rxtx */)
  {
    // The AD9361 locks within microseconds; there is no standard IIO
    // "lo_locked" sensor attribute in the common Pluto firmware.
    // If setLOFreq succeeded the LO is considered locked.
    return true;
  }

  // ---------------------------------------------------------------
  // Gain
  //
  // SoDaRadio convention (from RadioControl.hxx):
  //   gain is in dB, range -30..0.  0 = maximum (full power or max RX gain).
  //
  // AD9361 RX hardware gain: 0..73 dB absolute.
  //   Mapping: hw_gain = RX_GAIN_MAX + soda_gain
  //
  // AD9361 TX hardware gain: -89.75..0 dB (0 = maximum power, negative = attenuation).
  //   Mapping: hw_gain = soda_gain  (identical scale, different physical meaning)
  // ---------------------------------------------------------------

  float PlutoCtrl::setRFGain(float gain, SoDa::RXTX rxtx)
  {
    if (rxtx == SoDa::RX) {
      double hw_gain = RX_GAIN_MAX + (double)gain;
      writeHWRXGain(hw_gain);
      double actual = hw_gain;
      iio_channel_attr_read_double(rx_phy_chan, "hardwaregain", &actual);
      rx_rf_gain = gain;
      return (float)actual;
    }
    else {
      double hw_gain = (double)gain;  // already in -89.75..0 range
      writeHWTXGain(hw_gain);
      double actual = hw_gain;
      iio_channel_attr_read_double(tx_phy_chan, "hardwaregain", &actual);
      tx_rf_gain = gain;
      return (float)actual;
    }
  }

  // ---------------------------------------------------------------
  // Sample rate
  // ---------------------------------------------------------------

  void PlutoCtrl::setSampleRate(float /* rate */, SoDa::RXTX rxtx)
  {
    // PlutoRX and PlutoTX resample 4:1, so the hardware always runs at 2.5 MSPS.
    long long lrate = (long long)PLUTO_SAMPLE_RATE;
    if (rxtx == SoDa::RX) {
      iio_channel_attr_write_longlong(rx_phy_chan, "sampling_frequency", lrate);
      iio_channel_attr_write_longlong(rx_phy_chan, "rf_bandwidth",        lrate);
      long long actual = lrate;
      iio_channel_attr_read_longlong(rx_phy_chan, "sampling_frequency", &actual);
      rx_samp_rate_hw = (float)actual;
    }
    else {
      iio_channel_attr_write_longlong(tx_phy_chan, "sampling_frequency", lrate);
      iio_channel_attr_write_longlong(tx_phy_chan, "rf_bandwidth",        lrate);
      long long actual = lrate;
      iio_channel_attr_read_longlong(tx_phy_chan, "sampling_frequency", &actual);
      tx_samp_rate_hw = (float)actual;
    }
  }

  float PlutoCtrl::getSampleRate(SoDa::RXTX rxtx)
  {
    return (rxtx == SoDa::RX) ? rx_samp_rate_hw : tx_samp_rate_hw;
  }

  // ---------------------------------------------------------------
  // TX enable / disable
  //
  // The Pluto has no relay GPIO.  TX is "disabled" by slamming the TX
  // attenuation to its maximum; it is "enabled" by restoring tx_rf_gain.
  // ---------------------------------------------------------------

  void PlutoCtrl::setTXEna(bool enable, bool /* full_duplex */)
  {
    tx_on = enable;
    if (enable) {
      writeHWTXGain((double)tx_rf_gain);
    } else {
      writeHWTXGain(TX_GAIN_MIN);
    }
  }

  bool PlutoCtrl::getTXEna()
  {
    return tx_on;
  }

  bool PlutoCtrl::getTXRelayOn()
  {
    // No relay sense input on the Pluto — mirror the software state.
    return tx_on;
  }

  // ---------------------------------------------------------------
  // Antennas
  //
  // The Pluto exposes a single RX SMA and a single TX SMA.  There is
  // no software antenna selection in the IIO interface; we just track
  // the name the caller requested and echo it back.
  // ---------------------------------------------------------------

  std::vector<std::string> PlutoCtrl::listAntennas(SoDa::RXTX rxtx)
  {
    if (rxtx == SoDa::RX) return { "RX" };
    return { "TX" };
  }

  void PlutoCtrl::setAntenna(const std::string & ant, SoDa::RXTX rxtx)
  {
    // Single fixed port per direction; just remember the name.
    if (rxtx == SoDa::RX) rx_antenna_name = ant;
    else                    tx_antenna_name = ant;
  }

  std::string PlutoCtrl::getAntenna(SoDa::RXTX rxtx)
  {
    return (rxtx == SoDa::RX) ? rx_antenna_name : tx_antenna_name;
  }

  // ---------------------------------------------------------------
  // Hardware description
  // ---------------------------------------------------------------

  std::string PlutoCtrl::getHardwareDescription()
  {
    return hw_description;
  }

} // namespace SoDa
