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
 * @file PlutoTX.hxx
 *
 * @brief IIO transmit streaming thread for the ADALM-PLUTO.
 *
 * Owns its own iio_context connection (cf-ad9361-dds-core-lpc) separate
 * from PlutoCtrl and PlutoRX.  The Pluto hardware runs at 2.5 MSPS (4×
 * the SoDaRadio IF rate of 625 kSPS).  Incoming 625 kSPS CBufs from
 * RadioTX::run() are accumulated in a vector; once enough samples are
 * available, SoDa::ReSampler interpolates 4:1 to 2.5 MSPS and the result
 * is pushed to the Pluto DAC via iio_buffer_push().
 *
 * The DDS tone generators on cf-ad9361-dds-core-lpc are disabled at
 * construction so that our streamed samples reach the DAC unmodified.
 *
 * @author M. H. Reilly (kb1vc)
 * @date   June 2026
 *
 * @note Substantial parts written or modified by Claude Sonnet 4.6 (claude-sonnet-4-6)
 */

#include "RadioTX.hxx"
#include "SoDaBase.hxx"
#include "Params.hxx"
#include <SoDa/ReSampler.hxx>
#include <iio.h>
#include <memory>
#include <vector>
#include <complex>

namespace SoDa {
  class PlutoTX;
  typedef std::shared_ptr<PlutoTX> PlutoTXPtr;

  class PlutoTX : public RadioTX {
  protected:
    PlutoTX(ParamsPtr params);

  public:
    ~PlutoTX();

    static PlutoTXPtr make(ParamsPtr params) {
      auto ret = std::shared_ptr<PlutoTX>(new PlutoTX(params));
      ret->self = ret;
      ret->registerThread(ret);
      return ret;
    }

    void init() override { }

    /**
     * @brief Arm or disarm the IIO TX stream.
     * transmitSwitch(false) flushes the accumulator with zeros so the DAC
     * goes quiet cleanly.
     * @return the new tx_enabled state.
     */
    bool transmitSwitch(bool tx_on) override;

    /**
     * @brief Accumulate complex<float> samples; when a full resampler block
     * is available, interpolate 4:1 and push to the Pluto DAC.
     * @return true always.
     */
    bool put(CBufPtr buf) override;

    RadioTXPtr getSelfPtr() override { return self.lock(); }

  private:
    void pushToHW();
    void flushZeros();

    // IIO streaming objects — owned by this thread.
    iio_context * ctx;
    iio_device  * dev;         ///< cf-ad9361-dds-core-lpc
    iio_channel * tx_i_chan;   ///< voltage0 (output) — I component
    iio_channel * tx_q_chan;   ///< voltage1 (output) — Q component
    iio_buffer  * txbuf;

    unsigned int hw_buf_size;  ///< IIO buffer size = resampler output (at 2.5 MSPS)
    unsigned int rs_in_size;   ///< resampler input size (at 625 kSPS)

    SoDa::ReSamplerPtr hw_resampler;            ///< 625 kSPS → 2.5 MSPS

    std::vector<std::complex<float>> accu;      ///< accumulator for 625 kSPS input
    std::vector<std::complex<float>> rs_in;     ///< one resampler input block
    std::vector<std::complex<float>> hw_cf;     ///< resampler output (at 2.5 MSPS)

    std::weak_ptr<PlutoTX> self;
  };
}
