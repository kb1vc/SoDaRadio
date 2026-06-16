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

#include "RTLSDRRX.hxx"
#include "Command.hxx"
#include <SoDa/Format.hxx>
#include <cmath>
#include <algorithm>

namespace SoDa {

  RTLSDRRX::RTLSDRRX(RTLSDRDevPtr dev_in, ParamsPtr _params)
    : RadioRX(_params),
      rtl(dev_in),
      rx_sample_rate(_params->getRXRate()),
      streaming(false),
      if_streaming_enabled(true)
  {
    // Build 2.048 MSPS → 625 kSPS resampler.  ts=0.0485 matches PlutoRX.
    hw_resampler = SoDa::ReSampler::make(HW_RATE, 625000.0f, 0.0485f);
    hw_buf_size  = hw_resampler->getInputBufferSize();
    rs_out_size  = hw_resampler->getOutputBufferSize();
    rf_buf_size  = _params->getRFBufferSize();

    // raw_buf holds interleaved uint8 I/Q; 2 bytes per complex sample.
    // Round up to the nearest 512-byte transfer boundary required by librtlsdr.
    unsigned int raw_bytes = hw_buf_size * 2u;
    if (raw_bytes % 512 != 0)
      raw_bytes = ((raw_bytes / 512) + 1) * 512;
    hw_buf_size = raw_bytes / 2;  // adjust to match rounded size

    raw_buf.resize(raw_bytes);
    hw_cf.resize(hw_buf_size);
    rs_out.resize(rs_out_size);

    debugMsg(SoDa::Format("RTLSDRRX: hw_buf=%0 rs_out=%1 rf_buf=%2\n")
             .addI((int)hw_buf_size)
             .addI((int)rs_out_size)
             .addI((int)rf_buf_size));
  }

  void RTLSDRRX::init()
  {
    rtlsdr_reset_buffer(rtl->dev);
  }

  // ---------------------------------------------------------------
  // IF NCO
  // ---------------------------------------------------------------

  void RTLSDRRX::setNCOFreq(double freq)
  {
    IF_osc.setPhaseIncr(freq * 2.0 * M_PI / rx_sample_rate);
    debugMsg(SoDa::Format("RTLSDRRX: IF NCO set to %0 Hz\n").addF(freq, 'e'));
    cmd_stream->put(Command::make(Command::REP, Command::RX_IF_FREQ, freq));
  }

  // ---------------------------------------------------------------
  // Per-sample IF mixer
  // ---------------------------------------------------------------

  void RTLSDRRX::doMixer(std::vector<std::complex<float>> & buf)
  {
    for (auto & s : buf) s *= IF_osc.stepOscCF();
  }

  // ---------------------------------------------------------------
  // Main sample pump
  // ---------------------------------------------------------------

  bool RTLSDRRX::downConvert()
  {
    // Read one block of uint8 I/Q from the dongle.  This call blocks until
    // the hardware delivers raw_buf.size() bytes (≈ hw_buf_size / HW_RATE seconds).
    int n_read = 0;
    int rc = rtlsdr_read_sync(rtl->dev,
                              raw_buf.data(),
                              (int)raw_buf.size(),
                              &n_read);
    if (rc < 0 || n_read <= 0) {
      debugMsg(SoDa::Format("RTLSDRRX: read_sync error rc=%0\n").addI(rc));
      return false;
    }

    if (!streaming) return false;

    // Convert uint8 interleaved I/Q → complex<float>.
    // RTL-SDR: value 127/128 maps to 0.0; full scale is ±127.5.
    unsigned int n_cf = (unsigned int)n_read / 2;
    if (n_cf > hw_buf_size) n_cf = hw_buf_size;
    for (unsigned int i = 0; i < n_cf; i++) {
      hw_cf[i] = std::complex<float>(
        ((float)raw_buf[2*i]   - 127.5f) / 127.5f,
        ((float)raw_buf[2*i+1] - 127.5f) / 127.5f);
    }

    // Decimate 2.048 MSPS → 625 kSPS.
    hw_resampler->apply(hw_cf, rs_out);

    // Forward pre-mixer snapshot to the spectrum display.
    if (if_streaming_enabled && if_stream->subscriberCount() > 0) {
      CBufPtr if_buf = SoDa::CBuf::make(rs_out_size);
      auto & if_data = if_buf->getBuf();
      std::copy(rs_out.begin(), rs_out.end(), if_data.begin());
      if_stream->put(if_buf);
    }

    // Apply IF NCO mixer.
    doMixer(rs_out);

    // Accumulate resampler output, then publish in rf_buf_size-sample chunks.
    accu.insert(accu.end(), rs_out.begin(), rs_out.end());

    bool published = false;
    while (accu.size() >= rf_buf_size) {
      CBufPtr buf = SoDa::CBuf::make(rf_buf_size);
      auto & data = buf->getBuf();
      std::copy(accu.begin(), accu.begin() + rf_buf_size, data.begin());
      accu.erase(accu.begin(), accu.begin() + rf_buf_size);
      rx_stream->put(buf);
      published = true;
    }

    return published;
  }

} // namespace SoDa
