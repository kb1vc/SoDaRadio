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
 * @file PlutoRX.hxx
 *
 * @brief IIO receive streaming thread for the ADALM-PLUTO.
 *
 * Owns its own iio_context connection (cf-ad9361-lpc) so that it does
 * not share state with PlutoCtrl or PlutoTX.  The Pluto hardware is
 * run at 2.5 MSPS (4× the SoDaRadio IF rate of 625 kSPS).  Each IIO
 * buffer refill yields ~30258 samples at 625 kSPS after 4:1 decimation
 * via SoDa::ReSampler; an accumulator accumulates until 30000-sample
 * CBufs can be published to BaseBandRX on rx_stream.
 *
 * The IIO buffer is refilled continuously once started (same policy as
 * USRPRX) so that the DMA pipeline never stalls.  Upstream samples are
 * discarded when streaming is disabled (TX half-duplex mode).
 *
 * @author M. H. Reilly (kb1vc)
 * @date   June 2026
 *
 * @note Substantial parts written or modified by Claude Sonnet 4.6 (claude-sonnet-4-6)
 */

#include "RadioRX.hxx"
#include "SoDaBase.hxx"
#include "Params.hxx"
#include "QuadratureOscillator.hxx"
#include <SoDa/ReSampler.hxx>
#include <iio.h>
#include <memory>
#include <vector>
#include <complex>

namespace SoDa {
  class PlutoRX;
  typedef std::shared_ptr<PlutoRX> PlutoRXPtr;

  class PlutoRX : public RadioRX {
  protected:
    PlutoRX(ParamsPtr params);

  public:
    ~PlutoRX();

    static PlutoRXPtr make(ParamsPtr params) {
      auto ret = std::shared_ptr<PlutoRX>(new PlutoRX(params));
      ret->self = ret;
      ret->registerThread(ret);
      return ret;
    }

    /**
     * @brief Update the IF NCO frequency used by the complex mixer.
     * Called in response to SET RX_IF_FREQ from RadioControl.
     */
    void setNCOFreq(double freq) override;

    /**
     * @brief Refill the IIO buffer (2.5 MSPS), resample to 625 kSPS, mix,
     * and publish 30000-sample CBufs to rx_stream.
     * Blocks for approximately one hardware buffer period (~48 ms).
     */
    bool downConvert() override;

    /** @brief Arm the streaming flag so downConvert() publishes samples. */
    void startStream() override;

    /** @brief Disarm the streaming flag; IIO DMA continues draining silently. */
    void stopStream() override;

    bool streamEnabled() override;

    /** @brief Control whether raw RF samples are forwarded to the IF stream. */
    void enableIFStreamer(bool enable) override;

  private:
    void doMixer(std::vector<std::complex<float>> & buf);

    // IIO streaming objects — owned by this thread.
    iio_context * ctx;
    iio_device  * dev;         ///< cf-ad9361-lpc
    iio_channel * rx_i_chan;   ///< voltage0 (input) — I component
    iio_channel * rx_q_chan;   ///< voltage1 (input) — Q component
    iio_buffer  * rxbuf;

    unsigned int hw_buf_size;   ///< IIO buffer size in samples (at 2.5 MSPS)
    unsigned int rs_out_size;   ///< hw_resampler output size (at 625 kSPS)
    unsigned int rf_buf_size;   ///< publish chunk size for BaseBandRX (30000)

    double rx_sample_rate;      ///< 625 kSPS — used for IF NCO phase increment

    bool streaming;             ///< true after startStream(); samples published
    bool if_streaming_enabled;  ///< true → push raw RF copies to if_stream

    SoDa::ReSamplerPtr hw_resampler;            ///< 2.5 MSPS → 625 kSPS

    std::vector<std::complex<float>> hw_cf;     ///< SC16→float conversion (hw_buf_size)
    std::vector<std::complex<float>> rs_out;    ///< resampler output (rs_out_size)
    std::vector<std::complex<float>> accu;      ///< accumulator for 30000-sample publish

    QuadratureOscillator IF_osc;

    std::weak_ptr<PlutoRX> self;
  };
}
