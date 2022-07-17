/*
  Copyright (c) 2012,2022 Matthew H. Reilly (kb1vc)
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
#include "Radio.hxx"
#include "QuadratureOscillator.hxx"

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

  USRPRX::USRPRX(Params * params, uhd::usrp::multi_usrp::sptr _usrp) : 
    RXBase("USRPRX")
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

    rx_sample_rate = 625e3;
    rx_buffer_size = params->getRFBufferSize(); 

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

  }

  static void doFFTandDump(int fd, std::complex<float> * in, int len) __attribute__ ((unused));

  static void doFFTandDump(int fd, std::complex<float> * in, int len)
  {
    std::complex<float> out[len];
    // create plan
    fftwf_plan tplan = fftwf_plan_dft_1d(len, (fftwf_complex*) in, (fftwf_complex*) out,
					 FFTW_FORWARD, FFTW_ESTIMATE | FFTW_UNALIGNED);

    fftwf_execute(tplan);
    write(fd, out, sizeof(std::complex<float>) * len); 
    fftwf_destroy_plan(tplan); 
  }

  void USRPRX::init() {
    uhd::set_thread_priority_safe(); 
  }

  void USRPRX::cleanUp() {
    stopStream();     
  }

  double USRPRX::sampleRate() {
    return rx_sample_rate;
  }
  
  bool USRPRX::runTick()
  {
    // we watch for data in the input buffer. 
    if(audio_rx_stream_enabled) {
      // go get some data
      // get a free buffer.
      CFBuf buf = makeCFBuf(rx_buffer_size); 

      if(buf == nullptr) {
	throw Radio::Exception("USRPRX couldn't allocate Buf object", this); 
      }
      
      unsigned int left = rx_buffer_size;
      unsigned int coll_so_far = 0;
      uhd::rx_metadata_t md;
      std::complex<float> *dbuf = buf->data();
      while(left != 0) {
	unsigned int got = rx_bits->recv(&(dbuf[coll_so_far]), left, md);
	if(got == 0) {
	  debugMsg("****************************************");
	  debugMsg(Format("RECV got error -- md = [%0]\n").addS(md.to_pp_string()));
	  debugMsg("****************************************");	  
	}
	coll_so_far += got;
	left -= got;
      }

      // If the anybody cares, send the IF buffer out.
      // If the UI is listening, it will do an FFT on the buffer
      // and send the positive spectrum via the UI to any listener.
      // the UI does the FFT then puts it on its own ring.
      if(enable_spectrum_report) {
	// clone a buffer, cause we're going to modify
	// it before the send is complete. 
	CFBuf if_buf = makeCFBuf(rx_buffer_size); 

	*if_buf = *buf;
	if_stream->put(if_buf);
      }


      // support debug... 
      scount++;

      // tune it down with the IF oscillator
      doMixer(buf); 
      // now put the baseband signal on the ring.
      rx_stream->put(buf);

      // we may have more work to do. 
      return false; 
    }
    else {
      return true; 
    }
    
  }

  void USRPRX::doMixer(CFBuf inout)
  {
    unsigned int i;
    std::complex<float> o;
    for(i = 0; i < inout->size(); i++) {
      o = IF_osc.stepOscCF();
      (*inout)[i] = (*inout)[i] * o; 
    }
  }

  void USRPRX::set3rdLOFreq(double IF_tuning)
  {
    // calculate the advance of phase for the IF
    // oscilator in terms of radians per sample
    IF_osc.setPhaseIncr(IF_tuning * 2.0 * M_PI / rx_sample_rate);
    debugMsg(Format("Changed 3rdLO to freq = %0\n")
	     .addF(IF_tuning, 'e', 12, 6));
  }


  void USRPRX::startStream()
  {
    if(!audio_rx_stream_enabled) {
      usrp->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS, 0);
      audio_rx_stream_enabled = true; 
    }
  }

  void USRPRX::stopStream()
  {
    usrp->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS, 0);
    audio_rx_stream_enabled = false;
  }

  void USRPRX::execSetCommand(CmdMsg cmd)
  {
    switch(cmd->target) {
    case Command::RX_MODE:
      rx_modulation = Command::ModulationType(cmd->iparms[0]); 
      break; 
    case Command::RX_LO3_FREQ:
      current_IF_tuning = cmd->dparms[0];
      set3rdLOFreq(cmd->dparms[0]); 
      break;
    case Command::TX_STATE: // SET TX_ON
      if(cmd->iparms[0] == 3) {
	if((rx_modulation == Command::CW_L) || (rx_modulation == Command::CW_U)) {
	  // If we're in a CW mode, set the RF gain to zip.
	  // this is already done in the USRPCtrl thread.
	  // and adjust the AF gain.
	  debugMsg("In TX ON -- stream continues");
	}
	else {
	  // We used to stop the RX stream, but we don't anymore. 
	  debugMsg("In TX ON -- stopped stream");	
	  // stopStream();
	}
	enable_spectrum_report = (cmd->iparms[1] > 0);
      }
      if(cmd->iparms[0] == 2) {
	// start the RX stream.
	//usleep(750000);
	debugMsg("In TX OFF -- restart stream");
	// we never stopped the stream
	// but we need to start it the very first time. 
	startStream();
	enable_spectrum_report = true;
	// tell the baseband unit that it is ready to start. 
	cmd_stream->put(Command::make(Command::SET, Command::TX_STATE, 
				      4));
      }
      break; 
    default:
      break; 
    }
  }

  void USRPRX::execGetCommand(CmdMsg cmd)
  {
    (void) cmd; 
  }

  void USRPRX::execRepCommand(CmdMsg cmd)
  {
    (void) cmd; 
  }
}
