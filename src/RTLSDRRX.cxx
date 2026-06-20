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
      if_streaming_enabled(true),
      diag_resamp_runs(0),
      diag_published(0)
  {
    rf_buf_size  = _params->getRFBufferSize();

    uint32_t hi_buf_size = uint32_t((HW_RATE / rx_sample_rate) * rf_buf_size);

    hw_resampler = SoDa::ReSampler::make(HW_RATE, 625000.0f, hi_buf_size);  // 2.048 MSPS → 625 kSPS
    rs_in_size   = hw_resampler->getInputBufferSize();
    rs_out_size  = hw_resampler->getOutputBufferSize();

    rs_in.resize(rs_in_size);
    rs_out.resize(rs_out_size);

    debugMsg(SoDa::Format("RTLSDRRX: rs_in=%0 rs_out=%1 rf_buf=%2 read_bytes=%3\n")
             .addI((int)rs_in_size)
             .addI((int)rs_out_size)
             .addI((int)rf_buf_size)
             .addI((int)READ_BYTES));
  }

  void RTLSDRRX::init()
  {
    // rtlsdr_reset_buffer and streaming start are handled in startStream().
  }

  // ---------------------------------------------------------------
  // Async USB collection thread
  // ---------------------------------------------------------------

  void RTLSDRRX::asyncCallback(unsigned char* buf, uint32_t len, void* ctx)
  {
    RTLSDRRX* self = static_cast<RTLSDRRX*>(ctx);
    if (!self->streaming) return;

    unsigned int n_cf = len / 2;

    std::lock_guard<std::mutex> lock(self->raw_accu_mutex);

    // Drop oldest block if buffer is growing too large (protects against
    // latency runaway if downConvert() falls behind).
    if (self->raw_accu.size() > self->rs_in_size * RAW_ACCU_MAX) {
      self->raw_accu.erase(self->raw_accu.begin(),
                           self->raw_accu.begin() + self->rs_in_size);
    }

    for (unsigned int i = 0; i < n_cf; i++) {
      self->raw_accu.emplace_back(
        ((float)buf[2*i]   - 127.5f) / 127.5f,
        ((float)buf[2*i+1] - 127.5f) / 127.5f);
    }

    self->diag_reads++;
  }

  void RTLSDRRX::startStream()
  {
    // Stop any existing async thread cleanly before restarting.
    if (async_rx_thread.joinable()) {
      streaming = false;
      rtlsdr_cancel_async(rtl->dev);
      async_rx_thread.join();
    }

    {
      std::lock_guard<std::mutex> lock(raw_accu_mutex);
      raw_accu.clear();
    }

    rtlsdr_reset_buffer(rtl->dev);
    streaming = true;

    async_rx_thread = std::thread([this]() {
      rtlsdr_read_async(rtl->dev, asyncCallback, this, 0, READ_BYTES);
    });
  }

  void RTLSDRRX::stopStream()
  {
    streaming = false;
    rtlsdr_cancel_async(rtl->dev);
    if (async_rx_thread.joinable()) async_rx_thread.join();
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

  void RTLSDRRX::doMixer(std::vector<std::complex<float>> & buf)
  {
    for (auto & s : buf) s *= IF_osc.stepOscCF();
  }

  // ---------------------------------------------------------------
  // Processing
  // ---------------------------------------------------------------

  bool RTLSDRRX::downConvert()
  {
    if (!streaming) return false;

    // Grab one resampler-sized block from the async accumulator.
    {
      std::lock_guard<std::mutex> lock(raw_accu_mutex);
      if (raw_accu.size() < rs_in_size) return false;

      std::copy(raw_accu.begin(), raw_accu.begin() + rs_in_size, rs_in.begin());
      raw_accu.erase(raw_accu.begin(), raw_accu.begin() + rs_in_size);
    }

    // Periodic status dump every 10000 resampler runs (~8 minutes at 625 kSPS/30000).
    if ((diag_resamp_runs % 10000) == 0) {
      size_t accu_snap;
      {
        std::lock_guard<std::mutex> lock(raw_accu_mutex);
        accu_snap = raw_accu.size();
      }
      debugMsg(SoDa::Format("RTLSDRRX diag: callbacks=%0 accu=%1 resamp_runs=%2 published=%3\n")
               .addI((int)diag_reads.load())
               .addI((int)accu_snap)
               .addI((int)diag_resamp_runs)
               .addI((int)diag_published));
    }

    // Decimate 2.048 MSPS → 625 kSPS.
    hw_resampler->apply(rs_in, rs_out);
    diag_resamp_runs++;

    // Forward pre-mixer snapshot to spectrum display.
    if (if_streaming_enabled && if_stream->subscriberCount() > 0) {
      CBufPtr if_buf = SoDa::CBuf::make(rs_out_size);
      auto & if_data = if_buf->getBuf();
      std::copy(rs_out.begin(), rs_out.end(), if_data.begin());
      if_stream->put(if_buf);
    }

    // Apply IF NCO mixer.
    doMixer(rs_out);

    // Accumulate resampled output; publish in rf_buf_size-sample chunks.
    accu.insert(accu.end(), rs_out.begin(), rs_out.end());

    bool published = false;
    while (accu.size() >= rf_buf_size) {
      CBufPtr buf = SoDa::CBuf::make(rf_buf_size);
      auto & data = buf->getBuf();
      std::copy(accu.begin(), accu.begin() + rf_buf_size, data.begin());
      accu.erase(accu.begin(), accu.begin() + rf_buf_size);
      rx_stream->put(buf);
      diag_published++;
      published = true;
    }

    return published;
  }

} // namespace SoDa
