/*
  Copyright (c) 2012, Matthew H. Reilly (kb1vc)
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

#include <uhd/version.hpp>
#include <uhd/utils/safe_main.hpp>
#if UHD_VERSION < 3110000
#  include <uhd/utils/thread_priority.hpp>
#else 
#  include <uhd/utils/thread.hpp>
#endif
#include <uhd/usrp/multi_usrp.hpp>
#include <fftw3.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <SoDa/Format.hxx>

#include <fstream>

SoDa::USRPRX::USRPRX(Params * params, uhd::usrp::multi_usrp::sptr _usrp) : 
  SoDa::Thread("USRPRX")
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

  // we aren't receiving yet. 
  audio_rx_stream_enabled = false;

  // wake up in USB mode
  rx_modulation = SoDa::Command::USB;

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

void SoDa::USRPRX::run()
{
  if(cmd_stream == NULL) {
    throw SoDa::Exception(std::string("Never got command stream subscription\n"), 
			  this);	
  }
  if(rx_stream == NULL) {
    throw SoDa::Exception(std::string("Never got rx stream subscription\n"),
			  this);	
  }
  if(if_stream == NULL) {
    throw SoDa::Exception(std::string("Never got if stream subscription\n"),
			  this);	
  }
  
  uhd::set_thread_priority_safe(); 
  // now do the event loop.  we watch
  // for commands and responses on the command stream.
  // and we watch for data in the input buffer. 

  bool exitflag = false;

  while(!exitflag) {
    Command * cmd = cmd_stream->get(cmd_subs);
    if(cmd != NULL) {
      // process the command.
      execCommand(cmd);
      exitflag |= (cmd->target == Command::STOP); 
      cmd_stream->free(cmd); 
    }
    else if(audio_rx_stream_enabled) {
      // go get some data
      // get a free buffer.
      SoDa::Buf * buf = rx_stream->alloc();
      if(buf == NULL) {
	buf = new SoDa::Buf(rx_buffer_size); 
      }

      if(buf == NULL) throw SoDa::Exception("USRPRX couldn't allocate SoDa::Buf object", this); 
      if(buf->getComplexBuf() == NULL) throw SoDa::Exception("USRPRX allocated empty SoDa::Buf object", this);
      
      unsigned int left = rx_buffer_size;
      unsigned int coll_so_far = 0;
      uhd::rx_metadata_t md;
      std::complex<float> *dbuf = buf->getComplexBuf();
      while(left != 0) {
	unsigned int got = rx_bits->recv(&(dbuf[coll_so_far]), left, md);
	if(got == 0) {
	  debugMsg("****************************************");
	  debugMsg(SoDa::Format("RECV got error -- md = [%0]\n").addS(md.to_pp_string()));
	  debugMsg("****************************************");	  
	}
	coll_so_far += got;
	left -= got;
      }

      // If the anybody cares, send the IF buffer out.
      // If the UI is listening, it will do an FFT on the buffer
      // and send the positive spectrum via the UI to any listener.
      // the UI does the FFT then puts it on its own ring.
      if(enable_spectrum_report && (if_stream->getSubscriberCount() > 0)) {
	// clone a buffer, cause we're going to modify
	// it before the send is complete. 
	SoDa::Buf * if_buf = if_stream->alloc();
	if(if_buf == NULL) {
	  if_buf = new SoDa::Buf(rx_buffer_size); 
	}

	if(if_buf->copy(buf)) {
	  if_stream->put(if_buf);
	}
	else {
	  throw SoDa::Exception("SoDa::Buf Copy for IF stream failed", this);
	}
      }


      // support debug... 
      scount++;

      // tune it down with the IF oscillator
      doMixer(buf); 
      // now put the baseband signal on the ring.
      rx_stream->put(buf);

      // write the buffer output
    }
    else {
      usleep(1000);
    }
  }

  stopStream(); 
}

void SoDa::USRPRX::doMixer(SoDa::Buf * inout)
{
  unsigned int i;
  std::complex<float> o;
  std::complex<float> * ioa = inout->getComplexBuf();
  for(i = 0; i < inout->getComplexMaxLen(); i++) {
    o = IF_osc.stepOscCF();
    ioa[i] = ioa[i] * o; 
  }
}

void SoDa::USRPRX::set3rdLOFreq(double IF_tuning)
{
  // calculate the advance of phase for the IF
  // oscilator in terms of radians per sample
  IF_osc.setPhaseIncr(IF_tuning * 2.0 * M_PI / rx_sample_rate);
  debugMsg(SoDa::Format("Changed 3rdLO to freq = %0\n")
	   .addF(IF_tuning, 10, 6, 'e'));
}

void SoDa::USRPRX::execCommand(Command * cmd)
{
  switch (cmd->cmd) {
  case Command::GET:
    execGetCommand(cmd); 
    break;
  case Command::SET:
    execSetCommand(cmd); 
    break; 
  case Command::REP:
    execRepCommand(cmd); 
    break;
  default:
    break; 
  }
}

void SoDa::USRPRX::startStream()
{
  if(!audio_rx_stream_enabled) {
    usrp->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS, 0);
    audio_rx_stream_enabled = true; 
  }
}

void SoDa::USRPRX::stopStream()
{
  usrp->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS, 0);
  audio_rx_stream_enabled = false;
}

void SoDa::USRPRX::execSetCommand(Command * cmd)
{
  switch(cmd->target) {
  case SoDa::Command::RX_MODE:
    rx_modulation = SoDa::Command::ModulationType(cmd->iparms[0]); 
    break; 
  case Command::RX_LO3_FREQ:
    current_IF_tuning = cmd->dparms[0];
    set3rdLOFreq(cmd->dparms[0]); 
    break;
  case SoDa::Command::TX_STATE: // SET TX_ON
    if(cmd->iparms[0] == 3) {
      if((rx_modulation == SoDa::Command::CW_L) || (rx_modulation == SoDa::Command::CW_U)) {
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
      cmd_stream->put(new Command(Command::SET, Command::TX_STATE, 
				  4));
    }
    break; 
  default:
    break; 
  }
}

void SoDa::USRPRX::execGetCommand(Command * cmd)
{
  (void) cmd; 
}

void SoDa::USRPRX::execRepCommand(Command * cmd)
{
  (void) cmd; 
}

/// implement the subscription method
void SoDa::USRPRX::subscribeToMailBox(const std::string & mbox_name, 
					SoDa::BaseMBox * mbox_p) {
  if(SoDa::connectMailBox<SoDa::CmdMBox>(this, cmd_stream, "CMD", mbox_name, mbox_p)) {
    cmd_subs = cmd_stream->subscribe();
  }
  if(SoDa::connectMailBox<SoDa::DatMBox>(this, rx_stream, "RX", mbox_name, mbox_p)) {
    // we don't subscribe -- we publish
  }
  if(SoDa::connectMailBox<SoDa::DatMBox>(this, if_stream, "IF", mbox_name, mbox_p)) {
    // we don't subscribe -- we publish
  }
}
