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
 * @file PlutoCtrl.hxx
 *
 * @brief Thread class that owns the ADALM-PLUTO / AD9361 control channel.
 *
 * Implements the RadioControl interface using the libiio / ad9361-phy
 * IIO device.  The radio_args parameter (from Params) is interpreted as
 * an IIO URI:
 *   - bare IP address: "192.168.2.1"  → prepended to give "ip:192.168.2.1"
 *   - full URI:        "ip:192.168.2.1", "usb:", "local:", etc.
 *   - empty string:    falls back to "ip:192.168.2.1" (Pluto default)
 *
 * Frequency range (with AD9361 software extension): 70 MHz – 6 GHz.
 *
 * @note A Pluto-only build (-DNO_USRP=ON) is supported: TRControl.cxx,
 *       N200Control.cxx, and B200Control.cxx are compiled only when
 *       HAVE_UHD is set, and PlutoCtrl does not include TRControl.hxx.
 *
 * @author M. H. Reilly (kb1vc)
 * @date   June 2026
 *
 * @note Substantial parts written or modified by Claude Sonnet 4.6 (claude-sonnet-4-6)
 */

#include "SoDaBase.hxx"
#include "SoDaThread.hxx"
#include "Command.hxx"
#include "Params.hxx"
#include "RadioControl.hxx"
#include <iio.h>
#include <memory>
#include <string>
#include <vector>

namespace SoDa {
  class PlutoCtrl;
  typedef std::shared_ptr<PlutoCtrl> PlutoCtrlPtr;

  /**
   * @class PlutoCtrl
   *
   * Concrete RadioControl for the ADALM-PLUTO SDR (AD9361 transceiver).
   *
   * All frequency, gain, and sample-rate commands arrive on the command
   * stream and are dispatched by RadioControl::run().  This class provides
   * the hardware-specific implementations of those virtual methods.
   *
   * TX/RX switching is handled by asserting maximum TX attenuation when the
   * transmitter is disabled — the Pluto has no external relay GPIO.
   */
  class PlutoCtrl : public SoDa::RadioControl {
  protected:
    PlutoCtrl(ParamsPtr params);

  public:
    ~PlutoCtrl();

    static PlutoCtrlPtr make(ParamsPtr params) {
      auto ret = std::shared_ptr<PlutoCtrl>(new PlutoCtrl(params));
      ret->self = ret;
      ret->registerThread(ret);
      return ret;
    }

    RadioControlPtr getSelfPtr() override { return self.lock(); }

    /**
     * @brief Set hardware sample rate before PlutoTX/PlutoRX create their DMA buffers.
     *
     * iio_buffer_push() silently fails if the DMA buffer is created before the
     * ad9361-phy sampling_frequency attribute is written.  This override ensures
     * the rate is written during PlutoRadio::init() (ctrl→rx→tx order) so that
     * PlutoTX::init() sees a correctly configured DMA engine.
     */
    void init() override;

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

    bool getTXEna() override;
    bool getTXRelayOn() override;
    void setTXEna(bool enable, bool full_duplex) override;

  private:
    // IIO objects
    iio_context * ctx;         ///< IIO context (network, USB, or local)
    iio_device  * phy;         ///< ad9361-phy control device

    iio_channel * rx_lo_chan;  ///< altvoltage0: RX PLL oscillator
    iio_channel * tx_lo_chan;  ///< altvoltage1: TX PLL oscillator
    iio_channel * rx_phy_chan; ///< voltage0 (input direction):  RX path config
    iio_channel * tx_phy_chan; ///< voltage0 (output direction): TX path config

    // Cached hardware state
    double rx_lo_freq;         ///< last RX LO written to hardware (Hz)
    double tx_lo_freq;         ///< last TX LO written to hardware (Hz)
    float  rx_samp_rate_hw;    ///< RX sample rate read back from hardware
    float  tx_samp_rate_hw;    ///< TX sample rate read back from hardware

    std::string rx_antenna_name;
    std::string tx_antenna_name;

    std::string hw_description; ///< cached string for getHardwareDescription()

    // AD9361 limits
    static constexpr double RX_GAIN_MAX = 73.0;    ///< dB maximum RX hardware gain
    static constexpr double TX_GAIN_MIN = -89.75;  ///< dB most-negative TX gain (= max atten)
    static constexpr double FREQ_MIN    = 70.0e6;  ///< Hz (software-extended lower limit)
    static constexpr double FREQ_MAX    =  6.0e9;  ///< Hz (software-extended upper limit)

    // PlutoRX and PlutoTX each decimate/interpolate 4:1, so the Pluto hardware
    // always runs at 4 * 625k = 2.5 MSPS regardless of what Params requests.
    static constexpr float PLUTO_SAMPLE_RATE = 2500000.0f;

    std::weak_ptr<PlutoCtrl> self;

    /// Write TX hardware gain (dB, range TX_GAIN_MIN..0) to IIO, clamping first.
    void writeHWTXGain(double gain_db);
    /// Write RX hardware gain (dB, range 0..RX_GAIN_MAX) to IIO, clamping first.
    void writeHWRXGain(double gain_db);
  };
}
