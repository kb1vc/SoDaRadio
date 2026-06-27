/*
  Copyright (c) 2012, 2026 Matthew H. Reilly (kb1vc)
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

#include "USRPRX.hxx"
#include "QuadratureOscillator.hxx"
#include <algorithm>

#include <uhd/version.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/utils/thread.hpp>

#include <uhd/usrp/multi_usrp.hpp>
#include <fftw3.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <SoDa/Format.hxx>

#include <fstream>

namespace SoDa {
  USRPRX::USRPRX(ParamsPtr params, uhd::usrp::multi_usrp::sptr _usrp) : 
    RadioRX(params)
  {

    usrp = _usrp; 
  

    // create the rx buffer streamers.
    uhd::stream_args_t stream_args("fc32", "sc16");
    std::vector<size_t> channel_nums;
    channel_nums.push_back(0);
    stream_args.channels = channel_nums;
    rx_bits = usrp->get_rx_stream(stream_args);

    usrp->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS);
  
    // no UI listening for spectrum dumps yet.
    ui = NULL; 

    rx_sample_rate = params->getRXRate();
    rx_buffer_size = params->getRFBufferSize();

    rf_buf_size = params->getRFBufferSize();  // 30000

    double hw_rate = params->getHWSampleRate();
    needs_resample = (hw_rate != params->getRXRate());

    if (needs_resample) {
        uint32_t sample_bufsize = uint32_t(rf_buf_size * hw_rate / params->getRXRate());
        hw_resampler = SoDa::ReSampler::make((float)hw_rate, (float)params->getRXRate(), sample_bufsize);
        hw_buf_size  = hw_resampler->getInputBufferSize();
        rs_out_size  = hw_resampler->getOutputBufferSize();
        hw_cf.resize(hw_buf_size);
        rs_out.resize(rs_out_size);
        std::cerr << SoDa::Format("USRPRX: 4:1 resampler enabled hw_buf=%0 rs_out=%1 rf=%2\n")
            .addI((int)hw_buf_size).addI((int)rs_out_size).addI((int)rf_buf_size);
    } else {
        hw_buf_size = rx_buffer_size;  // existing path, no resampler
    }

    // we aren't receiving yet.
    audio_rx_stream_enabled = false;

    // wake up in USB mode
    rx_modulation = Command::USB;

    // setup debug hooks
    // outf[0] = creat("RF_premix.dat", 0666);
    // outf[1] = creat("RF_postmix.dat", 0666);
    scount = 0;

    // enable spectrum reporting at startup
    enable_spectrum_report = true; 

    rx_stream = NULL;
    if_stream = NULL;
    cmd_stream = NULL;

    uhd::set_thread_priority_safe(); 
    // now do the event loop.  we watch
    // for commands and responses on the command stream.
    // and we watch for data in the input buffer. 
  }


  bool USRPRX::downConvert()
  {
    bool did_work = false;

    if (!audio_rx_stream_enabled) return false;

    if (needs_resample) {
        // Recv hw_buf_size samples at 2.5 MS/s
        unsigned int left = hw_buf_size;
        unsigned int coll = 0;
        uhd::rx_metadata_t md;
        while (left != 0) {
            unsigned int got = rx_bits->recv(&hw_cf[coll], left, md);
            if (got == 0) {
                debugMsg(Format("USRPRX: recv got 0 -- md=[%0]\n").addS(md.to_pp_string()));
            }
            coll += got;
            left -= got;
        }

        // Decimate 4:1 to 625 kS/s
        hw_resampler->apply(hw_cf, rs_out);

        // Forward pre-mixer IF snapshot
        if (enable_spectrum_report && (if_stream->subscriberCount() > 0)) {
            CBufPtr if_buf = SoDa::CBuf::make(rs_out_size);
            auto & if_data = if_buf->getBuf();
            std::copy(rs_out.begin(), rs_out.end(), if_data.begin());
            if_stream->put(if_buf);
        }

        // Apply IF NCO mixer
        for (auto & s : rs_out) s *= IF_osc.stepOscCF();

        // Accumulate and publish in rf_buf_size chunks
        accu.insert(accu.end(), rs_out.begin(), rs_out.end());
        while (accu.size() >= rf_buf_size) {
            CBufPtr buf = SoDa::CBuf::make(rf_buf_size);
            auto & data = buf->getBuf();
            std::copy(accu.begin(), accu.begin() + rf_buf_size, data.begin());
            accu.erase(accu.begin(), accu.begin() + rf_buf_size);
            rx_stream->put(buf);
            did_work = true;
        }
    } else {
        // Original path — recv directly at 625 kS/s
        SoDa::CBufPtr buf = SoDa::CBuf::make(rx_buffer_size);
        if (buf == nullptr) throw SDR::Exception("USRPRX couldn't allocate SoDa::Buf object", self.lock());
        if (buf->size() == 0) throw SDR::Exception("USRPRX allocated empty SoDa::Buf object", self.lock());

        unsigned int left = rx_buffer_size;
        unsigned int coll_so_far = 0;
        uhd::rx_metadata_t md;
        std::complex<float> *dbuf = buf->getBuf().data();
        while (left != 0) {
            unsigned int got = rx_bits->recv(&(dbuf[coll_so_far]), left, md);
            if (got == 0) {
                debugMsg("****************************************");
                debugMsg(Format("RECV got error -- md = [%0]\n").addS(md.to_pp_string()));
                debugMsg("****************************************");
            }
            coll_so_far += got;
            left -= got;
        }

        if (enable_spectrum_report && (if_stream->subscriberCount() > 0)) {
            auto if_buf = SoDa::CBuf::make(rx_buffer_size);
            if (if_buf->copy(buf)) {
                if_stream->put(if_buf);
            } else {
                throw SDR::Exception("SoDa::Buf Copy for IF stream failed", self.lock());
            }
        }

        scount++;
        doMixer(buf);
        rx_stream->put(buf);
        did_work = true;
    }

    return did_work;
  }

  void USRPRX::doMixer(SoDa::CBufPtr inout)
  {
    unsigned int i;
    std::complex<float> o;
    for(i = 0; i < inout->size(); i++) {
      o = IF_osc.stepOscCF();
      (*inout)[i] = (*inout)[i] * o; 
    }
  }

  void USRPRX::setNCOFreq(double IF_tuning) 
  {
    // calculate the advance of phase for the IF
    // oscilator in terms of radians per sample
    IF_osc.setPhaseIncr(IF_tuning * 2.0 * M_PI / rx_sample_rate);
    debugMsg(Format("Changed 3rdLO to freq = %0\n")
	     .addF(IF_tuning, 'e', 10, 6));
    // send a message back.
    cmd_stream->put(Command::make(Command::REP, Command::RX_IF_FREQ, IF_tuning));
  }


  void USRPRX::startStream()
  {
    if(!audio_rx_stream_enabled) {
      usrp->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS, 0);
      audio_rx_stream_enabled = true; 
    }
  }

  bool USRPRX::streamEnabled() {
    return audio_rx_stream_enabled; 
  }

  void USRPRX::stopStream()
  {
    // we never stop the stream for a USRP
  }

  void USRPRX::closeStream()
  {
    usrp->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS, 0);
    audio_rx_stream_enabled = false;
  }

  void USRPRX::enableIFStreamer(bool enable)
  {
    enable_spectrum_report = enable;
  }

}
