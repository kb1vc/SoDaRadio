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
 * @file RTLSDRRX.hxx
 *
 * @brief RadioRX implementation for RTL-SDR dongles.
 *
 * RTL-SDR hardware delivers interleaved uint8 I/Q samples (127 = 0).
 * RTLSDRRX reads synchronously at 2.048 MSPS, converts to complex<float>,
 * resamples 2.048 MSPS → 625 kSPS via SoDa::ReSampler, applies the IF NCO
 * mixer, and publishes 30000-sample CBufs to rx_stream.  Pre-mixer snapshots
 * are forwarded to if_stream for the spectrum display.
 *
 * @author M. H. Reilly (kb1vc)
 * @date   June 2026
 */

#include "RadioRX.hxx"
#include "Params.hxx"
#include "QuadratureOscillator.hxx"
#include "RTLSDRDev.hxx"
#include <SoDa/ReSampler.hxx>
#include <vector>
#include <complex>
#include <memory>

namespace SoDa {

  class RTLSDRRX;
  using RTLSDRRXPtr = std::shared_ptr<RTLSDRRX>;

  class RTLSDRRX : public RadioRX {
  protected:
    RTLSDRRX(RTLSDRDevPtr dev_in, ParamsPtr params);

  public:
    ~RTLSDRRX() = default;

    static RTLSDRRXPtr make(RTLSDRDevPtr dev_in, ParamsPtr params) {
      auto ret = std::shared_ptr<RTLSDRRX>(new RTLSDRRX(dev_in, params));
      ret->self = ret;
      ret->registerThread(ret);
      return ret;
    }

    void init() override;

    void setNCOFreq(double freq) override;
    bool downConvert() override;

    void startStream()    override { streaming = true;  }
    void stopStream()     override { streaming = false; }
    bool streamEnabled()  override { return streaming;  }
    void enableIFStreamer(bool enable) override { if_streaming_enabled = enable; }

  private:
    void doMixer(std::vector<std::complex<float>> & buf);

    RTLSDRDevPtr rtl;

    // Hardware sample rate (set to 2.048 MSPS).
    static constexpr float HW_RATE = 2048000.0f;

    unsigned int hw_buf_size;   ///< resampler input size (at 2.048 MSPS)
    unsigned int rs_out_size;   ///< resampler output size (at 625 kSPS)
    unsigned int rf_buf_size;   ///< publish chunk size (30000)

    double rx_sample_rate;      ///< 625 kSPS — used for IF NCO phase increment

    bool streaming;
    bool if_streaming_enabled;

    SoDa::ReSamplerPtr hw_resampler;                    ///< 2.048 MSPS → 625 kSPS

    std::vector<uint8_t>              raw_buf;   ///< uint8 I/Q from librtlsdr
    std::vector<std::complex<float>>  hw_cf;     ///< converted float I/Q (hw_buf_size)
    std::vector<std::complex<float>>  rs_out;    ///< resampler output (rs_out_size)
    std::vector<std::complex<float>>  accu;      ///< accumulator for 30000-sample publish

    QuadratureOscillator IF_osc;

    std::weak_ptr<RTLSDRRX> self;
  };

} // namespace SoDa
