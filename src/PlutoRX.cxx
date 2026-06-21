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

#include "PlutoRX.hxx"
#include <SoDa/Format.hxx>
#include <cmath>
#include <cstring>
#include <algorithm>

namespace SoDa {

  // ---------------------------------------------------------------
  // Helpers
  // ---------------------------------------------------------------

  static std::string buildPlutoURI(const std::string & args) {
    if (args.find(':') != std::string::npos) return args;
    if (!args.empty())                        return "ip:" + args;
    return "ip:192.168.2.1";
  }

  // ---------------------------------------------------------------
  // Constructor
  // ---------------------------------------------------------------

  PlutoRX::PlutoRX(ParamsPtr params)
    : RadioRX(params),
      ctx(nullptr), dev(nullptr),
      rx_i_chan(nullptr), rx_q_chan(nullptr),
      rxbuf(nullptr),
      rx_sample_rate(params->getRXRate()),
      streaming(false),
      if_streaming_enabled(true)
  {
    std::string uri = buildPlutoURI(params->getRadioArgs());

    // Build the 2.5 MSPS → 625 kSPS resampler.
    rf_buf_size  = params->getRFBufferSize();             // 30000
    uint32_t sample_bufsize = uint32_t(rf_buf_size * 2.5e6 / 0.625e6);
    hw_resampler = SoDa::ReSampler::make(2500000.0f, 625000.0f, sample_bufsize);
    
    hw_buf_size  = hw_resampler->getInputBufferSize();   // samples at 2.5 MSPS
    rs_out_size  = hw_resampler->getOutputBufferSize();  // samples at 625 kSPS

    hw_cf.resize(hw_buf_size);
    rs_out.resize(rs_out_size);

    ctx = iio_create_context_from_uri(uri.c_str());
    if (!ctx)
      throw SDR::Exception(
        SoDa::Format("PlutoRX: cannot open IIO context at [%0]\n").addS(uri),
        self.lock());

    dev = iio_context_find_device(ctx, "cf-ad9361-lpc");
    if (!dev) {
      iio_context_destroy(ctx); ctx = nullptr;
      throw SDR::Exception(
        SoDa::Format("PlutoRX: cf-ad9361-lpc not found at [%0]\n").addS(uri),
        self.lock());
    }

    rx_i_chan = iio_device_find_channel(dev, "voltage0", false);
    rx_q_chan = iio_device_find_channel(dev, "voltage1", false);
    if (!rx_i_chan || !rx_q_chan) {
      iio_context_destroy(ctx); ctx = nullptr;
      throw SDR::Exception(
        SoDa::Format("PlutoRX: voltage0/voltage1 input channels not found\n"),
        self.lock());
    }

    iio_channel_enable(rx_i_chan);
    iio_channel_enable(rx_q_chan);

    // Non-cyclic buffer at 2.5 MSPS — sized to match the resampler's input.
    rxbuf = iio_device_create_buffer(dev, hw_buf_size, false);
    if (!rxbuf) {
      iio_context_destroy(ctx); ctx = nullptr;
      throw SDR::Exception(
        SoDa::Format("PlutoRX: failed to create IIO RX buffer (size=%0)\n")
          .addI((int)hw_buf_size),
        self.lock());
    }

    debugMsg(SoDa::Format("PlutoRX: connected to %0, hw_buf=%1 rs_out=%2 rf_buf=%3\n")
             .addS(uri).addI((int)hw_buf_size)
             .addI((int)rs_out_size).addI((int)rf_buf_size));
  }

  PlutoRX::~PlutoRX()
  {
    if (rxbuf)    { iio_buffer_destroy(rxbuf);       rxbuf    = nullptr; }
    if (rx_i_chan){ iio_channel_disable(rx_i_chan);   rx_i_chan = nullptr; }
    if (rx_q_chan){ iio_channel_disable(rx_q_chan);   rx_q_chan = nullptr; }
    if (ctx)      { iio_context_destroy(ctx);         ctx      = nullptr; }
  }

  // ---------------------------------------------------------------
  // IF NCO
  // ---------------------------------------------------------------

  void PlutoRX::setNCOFreq(double freq)
  {
    IF_osc.setPhaseIncr(freq * 2.0 * M_PI / rx_sample_rate);
    debugMsg(SoDa::Format("PlutoRX: IF NCO set to %0 Hz\n").addF(freq, 'e'));
    cmd_stream->put(Command::make(Command::REP, Command::RX_IF_FREQ, freq));
  }

  // ---------------------------------------------------------------
  // Streaming control
  // ---------------------------------------------------------------

  void PlutoRX::startStream()  { streaming = true;  }
  void PlutoRX::stopStream()   { streaming = false; }
  bool PlutoRX::streamEnabled(){ return streaming;  }

  void PlutoRX::enableIFStreamer(bool enable) { if_streaming_enabled = enable; }

  // ---------------------------------------------------------------
  // Per-sample IF mixer
  // ---------------------------------------------------------------

  void PlutoRX::doMixer(std::vector<std::complex<float>> & buf)
  {
    for (auto & samp : buf)
      samp *= IF_osc.stepOscCF();
  }

  // ---------------------------------------------------------------
  // Main sample pump
  // ---------------------------------------------------------------

  bool PlutoRX::downConvert()
  {
    // Always drain the IIO DMA even when not publishing, to prevent backlog.
    ssize_t nbytes = iio_buffer_refill(rxbuf);
    if (nbytes < 0) {
      debugMsg(SoDa::Format("PlutoRX: iio_buffer_refill error %0\n").addI((int)nbytes));
      return false;
    }

    if (!streaming) return false;

    // Convert sc16 interleaved samples → complex<float> at 2.5 MSPS.
    char     * i_ptr = (char *)iio_buffer_first(rxbuf, rx_i_chan);
    char     * q_ptr = (char *)iio_buffer_first(rxbuf, rx_q_chan);
    ptrdiff_t  step  = iio_buffer_step(rxbuf);

    for (unsigned int idx = 0; idx < hw_buf_size; idx++) {
      char * p = i_ptr + idx * step;
      hw_cf[idx] = std::complex<float>(
        *(const int16_t *)p              / 32768.0f,
        *(const int16_t *)(q_ptr + idx * step) / 32768.0f);
    }

    // Decimate 4:1 to 625 kSPS.
    hw_resampler->apply(hw_cf, rs_out);

    // Forward pre-mixer IF snapshot to the spectrogram stream.
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
