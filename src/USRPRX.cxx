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

#include <uhd/utils/safe_main.hpp>
#include <uhd/utils/thread_priority.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <fftw3.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <fstream>

SoDa::USRPRX::USRPRX(Params * params, uhd::usrp::multi_usrp::sptr _usrp,
		     DatMBox * _rx_stream, DatMBox * _if_stream,
		     CmdMBox * _cmd_stream, 
		     bool _transverter_capable) : SoDa::SoDaThread("USRPRX")
{
  cmd_stream = _cmd_stream;
  rx_stream = _rx_stream;
  if_stream = _if_stream; 
  transverter_capable = _transverter_capable; 
  usrp = _usrp; 
  
  // subscribe to the command stream.
  cmd_subs = cmd_stream->subscribe();

  // create the rx buffer streamers.
  uhd::stream_args_t stream_args("fc32", "sc16");
  stream_args.channels.push_back(0);
  if(transverter_capable) {
    stream_args.channels.push_back(1);
  }

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

#if 0
  rf_dumpfile.open("RFDump.dat", std::ios::out | std::ios::binary);
  if_dumpfile.open("IFDump.dat", std::ios::out | std::ios::binary);
#endif
}

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
  uhd::set_thread_priority_safe(); 
  // now do the event loop.  we watch
  // for commands and responses on the command stream.
  // and we watch for data in the input buffer. 

  bool exitflag = false;

  // we need a junk RX buffer to support transverter-enabled widgets. 
  // this just consumes a null bitstream. 
  std::complex<float> * junk_buffer = new std::complex<float>[rx_buffer_size]; 

  while(!exitflag) {
    Command * cmd = cmd_stream->get(cmd_subs);
    if(cmd != NULL) {
      //      std::cerr << "\n\nIn RX loop got command\n\n" << std::endl; 
      // process the command.
      execCommand(cmd);
      exitflag |= (cmd->target == Command::STOP); 
      cmd_stream->free(cmd); 
    }
    else if(audio_rx_stream_enabled) {
      // go get some data
      // get a free buffer.
      SoDaBuf * buf = rx_stream->alloc();
      if(buf == NULL) {
	buf = new SoDaBuf(rx_buffer_size); 
      }

      if(buf == NULL) throw(new SoDa::SoDaException("USRPRX couldn't allocate SoDaBuf object", this)); 
      if(buf->getComplexBuf() == NULL) throw(new SoDa::SoDaException("USRPRX allocated empty SoDaBuf object", this));
      
      unsigned int left = rx_buffer_size;
      unsigned int coll_so_far = 0;
      uhd::rx_metadata_t md;
      std::complex<float> *dbuf = buf->getComplexBuf();
      std::vector< std::complex<float> * > buffs; 
      buffs.push_back(dbuf); 
      if(transverter_capable) {
	buffs.push_back(junk_buffer); 
      }
      while(left != 0) {
	buffs[0] = &(dbuf[coll_so_far]); 
	unsigned int got = rx_bits->recv(buffs, left, md);
	if(got == 0) {
	  debugMsg("****************************************");
	  debugMsg(boost::format("RECV got error -- md = [%s]\n") % md.to_pp_string());
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
	SoDaBuf * if_buf = if_stream->alloc();
	if(if_buf == NULL) {
	  if_buf = new SoDaBuf(rx_buffer_size); 
	}

	if(if_buf->copy(buf)) {
	  if_stream->put(if_buf);
	}
	else {
	  throw new SoDaException("SoDaBuf Copy for IF stream failed", this);
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

void SoDa::USRPRX::doMixer(SoDaBuf * inout)
{
  int i;
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
  debugMsg(boost::format("Changed 3rdLO to freq = %g\n") % IF_tuning);
}

void SoDa::USRPRX::execCommand(Command * cmd)
{
  //  std::cerr << "In USRPRX execCommand" << std::endl;
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
    //  std::cerr << "Starting RX Stream from USRP" << std::endl;
    usrp->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS); // , 0);
    audio_rx_stream_enabled = true; 
  }
}

void SoDa::USRPRX::stopStream()
{
  //  std::cerr << "Stoping RX Stream from USRP" << std::endl; 
  usrp->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS); // , 0);
  audio_rx_stream_enabled = false;
}

void SoDa::USRPRX::execSetCommand(Command * cmd)
{
  //  std::cerr << "In USRPRX execSetCommand" << std::endl;
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
	// Otherwise
	// stop the RX stream
	debugMsg("In TX ON -- stopped stream");	
	stopStream();
      }
      enable_spectrum_report = false;
    }
    if(cmd->iparms[0] == 2) {
      // start the RX stream.
      //usleep(750000);
      debugMsg("In TX OFF -- restart stream");
      startStream();
      enable_spectrum_report = true;
    }
    break; 
  default:
    break; 
  }
}

void SoDa::USRPRX::execGetCommand(Command * cmd)
{
}

void SoDa::USRPRX::execRepCommand(Command * cmd)
{
}

