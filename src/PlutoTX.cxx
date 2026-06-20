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

#include "PlutoTX.hxx"
#include <SoDa/Format.hxx>
#include <algorithm>
#include <cstring>

namespace SoDa {

  static std::string buildPlutoURITX(const std::string & args) {
    if (args.find(':') != std::string::npos) return args;
    if (!args.empty())                        return "ip:" + args;
    return "ip:192.168.2.1";
  }

  // ---------------------------------------------------------------
  // Constructor
  // ---------------------------------------------------------------

  PlutoTX::PlutoTX(ParamsPtr params)
    : RadioTX(params, "PlutoTX"),
      ctx(nullptr), dev(nullptr),
      tx_i_chan(nullptr), tx_q_chan(nullptr),
      txbuf(nullptr)
  {
    std::string uri = buildPlutoURITX(params->getRadioArgs());

    // Build the 625 kSPS → 2.5 MSPS interpolator.
    // ts=0.0480 → in=29780, out=119120.  Incoming CBufs are 30000 samples,
    // so the accumulator usually has enough for one push per put() call.
    uint32_t rf_buf_size  = params->getRFBufferSize();             // 30000

    hw_resampler = SoDa::ReSampler::make(625000.0f, 2500000.0f, rf_buf_size);
    rs_in_size   = hw_resampler->getInputBufferSize();   // 29780
    hw_buf_size  = hw_resampler->getOutputBufferSize();  // 119120

    rs_in.resize(rs_in_size);
    hw_cf.resize(hw_buf_size);

    ctx = iio_create_context_from_uri(uri.c_str());
    if (!ctx)
      throw SDR::Exception(
        SoDa::Format("PlutoTX: cannot open IIO context at [%0]\n").addS(uri),
        self.lock());

    dev = iio_context_find_device(ctx, "cf-ad9361-dds-core-lpc");
    if (!dev) {
      iio_context_destroy(ctx); ctx = nullptr;
      throw SDR::Exception(
        SoDa::Format("PlutoTX: cf-ad9361-dds-core-lpc not found at [%0]\n").addS(uri),
        self.lock());
    }

    // Disable all DDS tone generators so streamed samples reach the DAC.
    for (unsigned int i = 0; i < iio_device_get_channels_count(dev); i++) {
      iio_channel * ch = iio_device_get_channel(dev, i);
      if (iio_channel_is_output(ch)) {
        const char * id = iio_channel_get_id(ch);
        if (id && std::strstr(id, "altvoltage"))
          iio_channel_attr_write(ch, "raw", "0");
      }
    }

    tx_i_chan = iio_device_find_channel(dev, "voltage0", true);
    tx_q_chan = iio_device_find_channel(dev, "voltage1", true);
    if (!tx_i_chan || !tx_q_chan) {
      iio_context_destroy(ctx); ctx = nullptr;
      throw SDR::Exception(
        SoDa::Format("PlutoTX: voltage0/voltage1 output channels not found\n"),
        self.lock());
    }

    iio_channel_enable(tx_i_chan);
    iio_channel_enable(tx_q_chan);

    // Pipeline N_KERNEL_BUFS kernel DMA buffers so the DAC is always fed.
    // iio_buffer_push() blocks only when all slots are full, meaning
    // (N_KERNEL_BUFS-1) buffers worth of headroom before any underrun.
    iio_device_set_kernel_buffers_count(dev, N_KERNEL_BUFS);

    txbuf = iio_device_create_buffer(dev, hw_buf_size, false);
    if (!txbuf) {
      iio_context_destroy(ctx); ctx = nullptr;
      throw SDR::Exception(
        SoDa::Format("PlutoTX: failed to create IIO TX buffer (size=%0)\n")
          .addI((int)hw_buf_size),
        self.lock());
    }

    debugMsg(SoDa::Format("PlutoTX: connected to %0, rs_in=%1 hw_buf=%2 kbufs=%3\n")
             .addS(uri).addI((int)rs_in_size).addI((int)hw_buf_size).addI((int)N_KERNEL_BUFS));
  }

  PlutoTX::~PlutoTX()
  {
    if (txbuf) { iio_buffer_destroy(txbuf); txbuf = nullptr; }
    if (ctx)   { iio_context_destroy(ctx);  ctx   = nullptr; }
  }

  // ---------------------------------------------------------------
  // Private helpers
  // ---------------------------------------------------------------

  void PlutoTX::pushToHW()
  {
    // hw_cf holds hw_buf_size complex<float> samples at 2.5 MSPS.
    // Convert to SC16 interleaved in the IIO buffer and push to the DAC.
    char     * i_ptr = (char *)iio_buffer_first(txbuf, tx_i_chan);
    char     * q_ptr = (char *)iio_buffer_first(txbuf, tx_q_chan);
    ptrdiff_t  step  = iio_buffer_step(txbuf);

    for (unsigned int idx = 0; idx < hw_buf_size; idx++) {
      *(int16_t *)(i_ptr + idx * step) = (int16_t)(hw_cf[idx].real() * 32767.0f);
      *(int16_t *)(q_ptr + idx * step) = (int16_t)(hw_cf[idx].imag() * 32767.0f);
    }
    iio_buffer_push(txbuf);
  }

  void PlutoTX::flushZeros()
  {
    // Pad accumulator to exactly one resampler block with zeros, then push.
    accu.resize(rs_in_size, {0.0f, 0.0f});
    std::copy(accu.begin(), accu.begin() + rs_in_size, rs_in.begin());
    hw_resampler->apply(rs_in, hw_cf);
    pushToHW();
    accu.clear();
  }

  // ---------------------------------------------------------------
  // TX arm / disarm
  // ---------------------------------------------------------------

  bool PlutoTX::transmitSwitch(bool tx_on)
  {
    if (tx_on) {
      if (tx_enabled) return true;
      tx_enabled = true;
    } else {
      if (!tx_enabled) return false;
      flushZeros();
      tx_enabled = false;
    }
    return tx_enabled;
  }

  // ---------------------------------------------------------------
  // Sample pump — called by RadioTX::run() for each output block
  // ---------------------------------------------------------------

  bool PlutoTX::put(CBufPtr buf)
  {
    const auto & data = buf->getBuf();
    accu.insert(accu.end(), data.begin(), data.end());

    while (accu.size() >= rs_in_size) {
      std::copy(accu.begin(), accu.begin() + rs_in_size, rs_in.begin());
      accu.erase(accu.begin(), accu.begin() + rs_in_size);
      hw_resampler->apply(rs_in, hw_cf);
      pushToHW();
    }

    return true;
  }

} // namespace SoDa
