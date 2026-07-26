/*
  Copyright (c) 2012, 2025, 2026 Matthew H. Reilly (kb1vc)
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

#include "USRPTX.hxx"

#include <uhd/version.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/utils/thread.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <SoDa/Format.hxx>


namespace SoDa {
  USRPTX::USRPTX(ParamsPtr params, uhd::usrp::multi_usrp::sptr _usrp) : 
    RadioTX(params, "USRPTX")
  {
    usrp = _usrp; 

    // create the tx buffer streamers.
    stream_args = new uhd::stream_args_t("fc32", "sc16");
    stream_args->channels.push_back(0);

    tx_bits = usrp->get_tx_stream(*stream_args);

    uint32_t rf_buf_size = params->getRFBufferSize();
    double hw_rate = params->getHWSampleRate();
    needs_resample = (hw_rate != params->getTXRate());

    if (needs_resample) {
        hw_resampler = SoDa::ReSampler::make((float)params->getTXRate(), (float)hw_rate, rf_buf_size);
        rs_in_size   = hw_resampler->getInputBufferSize();
        hw_buf_size  = hw_resampler->getOutputBufferSize();
        rs_in.resize(rs_in_size);
        hw_cf.resize(hw_buf_size);
        std::cerr << SoDa::Format("USRPTX: 4:1 resampler enabled rs_in=%0 hw_buf=%1\n")
            .addI((int)rs_in_size).addI((int)hw_buf_size);
    }
  }

  bool USRPTX::put(CBufPtr buf) {
    if (!needs_resample) {
        // Direct path — send 625 kS/s samples straight to the USRP
        auto tbuf = buf->getBuf();
        std::vector<std::complex<float> *> buffers(1);
        buffers[0] = tbuf.data();
        tx_bits->send(buffers, tbuf.size(), tx_md);
        tx_md.start_of_burst = false;
        return true;
    }

    // Accumulate then resample 625 kS/s → 2.5 MS/s
    const auto & data = buf->getBuf();
    accu.insert(accu.end(), data.begin(), data.end());

    while (accu.size() >= rs_in_size) {
        std::copy(accu.begin(), accu.begin() + rs_in_size, rs_in.begin());
        accu.erase(accu.begin(), accu.begin() + rs_in_size);
        hw_resampler->apply(rs_in, hw_cf);
        std::vector<std::complex<float> *> buffers(1);
        buffers[0] = hw_cf.data();
        tx_bits->send(buffers, hw_buf_size, tx_md);
        tx_md.start_of_burst = false;
    }
    return true;
  }

  bool USRPTX::transmitSwitch(bool tx_on)
  {
    if (tx_on) {
        if (tx_enabled) return true;
        tx_md.start_of_burst = true;
        tx_md.end_of_burst = false;
        tx_md.has_time_spec = false;
        tx_enabled = true;
        // Prime UHD with ~150 ms of silence. The GUI's audio ring in AudioQt
        // is empty at TX_ON (sleepIn cleared it), so the first real buffer
        // won't reach us until BaseBandTX has accumulated one audio buffer
        // from the socket (~48 ms) and pushed it through the interpolator.
        // Without a cushion, the USB-connected USRP underruns immediately.
        const int prime_bufs = 3;
        for (int i = 0; i < prime_bufs; i++) {
            put(zero_buf);
        }
    } else {
        if (!tx_enabled) return false;
        if (needs_resample && !accu.empty()) {
            // Flush accumulator with zeros so DAC goes quiet cleanly
            accu.resize(rs_in_size, {0.0f, 0.0f});
            std::copy(accu.begin(), accu.begin() + rs_in_size, rs_in.begin());
            hw_resampler->apply(rs_in, hw_cf);
            std::vector<std::complex<float> *> buffers(1);
            buffers[0] = hw_cf.data();
            tx_bits->send(buffers, hw_buf_size, tx_md);
            accu.clear();
        }
        tx_md.end_of_burst = true;
        put(zero_buf);
        tx_enabled = false;
    }
    return tx_enabled;
  }

}
