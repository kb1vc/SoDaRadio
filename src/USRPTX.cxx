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

#include "USRPTX.hxx"
#include "MailBoxRegistry.hxx"

#include <uhd/version.hpp>
#include <uhd/utils/safe_main.hpp>
#if UHD_VERSION < 3110000
#  include <uhd/utils/thread_priority.hpp>
#else 
#  include <uhd/utils/thread.hpp>
#endif
#include <uhd/usrp/multi_usrp.hpp>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

namespace SoDa { 
  USRPTX::USRPTX(Params * params, uhd::usrp::multi_usrp::sptr _usrp) : Thread("USRPTX")
  {
    cmd_stream = NULL;
    tx_stream = NULL;
    cw_env_stream = NULL;
  
    usrp = _usrp; 


    LO_enabled = false;
    LO_configured = false;
    LO_capable = false;
    beacon_mode = false; 

    // create the tx buffer streamers.
    stream_args = new uhd::stream_args_t("fc32", "sc16");
    stream_args->channels.push_back(0);
    if(0 && (usrp->get_tx_num_channels() > 1)) {
      // disable this for now... there appears to be a bug in the b210 support in 3.8.1
      debugMsg("This radio is transverter LO capable");
      // use the second channel as a transverter LO
      stream_args->channels.push_back(1);
      LO_capable = true;
    }
    else {
      debugMsg("This radio is NOT transverter LO capable");
    }

    // find out how to configure the transmitter
    tx_sample_rate = params->getTXRate();
    tx_buffer_size = params->getRFBufferSize();
  
    // 400 Hz is a nice tone
    // but 400 doesn't really work that well.
    // 650 is better.  but all the RX filters
    // are centered near 500... let's do that.
    CW_tone_freq = 500.0;
    setCWFreq(true, CW_tone_freq); 

    // we aren't waiting for anything. 
    waiting_to_run_dry = false; 

    // build the beacon buffer, and the zero buffer.
    beacon_env = makeFBuf(tx_buffer_size); 
    zero_env = makeFBuf(tx_buffer_size); 
    for(unsigned int i = 0; i < tx_buffer_size; i++) {
      (*beacon_env)[i] = 1.0;
      (*zero_env)[i] = 0.0; 
    }

    // build the cwbuffer
    CFBuf cw_buf = makeCFBuf(tx_buffer_size);

    // set the initial envelope amplitude
    cw_env_amplitude = 0.7;  // more or less sqrt2/2
  
    // build the zero buffer and the transverter lo buffer
    zero_buf = makeCFBuf(tx_buffer_size);
    const_buf = makeCFBuf(tx_buffer_size); 
    for(unsigned int i = 0; i < tx_buffer_size; i++) {
      (*zero_buf)[i] = std::complex<float>(0.0, 0.0);
      (*const_buf)[i] = std::complex<float>(1.0, 0.0);
    }

    tx_enabled = false;
  }

  void USRPTX::run()
  {
    if((cmd_stream == NULL) || (tx_stream == NULL) || (cw_env_stream == NULL)) {
      throw Radio::Exception(std::string("Missing a stream connection.\n"), 
			     this);	
    }

    uhd::set_thread_priority_safe(); 
    // now do the event loop.  we watch
    // for commands and responses on the command stream.
    // and we watch for data in the input buffer. 

    // get the tx streamer 
    debugMsg("Creating tx streamer.\n");
    getTXStreamer();  
    debugMsg("Created tx streamer.\n");

    bool exitflag = false;
    CFBuf txbuf;
    FBuf cwenv;
    CmdMsg cmd; 
    std::vector<std::complex<float> *> buffers(LO_capable ? 2 : 1);

    while(!exitflag) {
      bool didwork = false; 
      if(LO_capable && LO_enabled && LO_configured) buffers[1] = const_buf->data();
      else if(LO_capable) buffers[1] = zero_buf->data();
    
      if((cmd = cmd_stream->get(cmd_subs)) != NULL) {
	// process the command.
	execCommand(cmd);
	didwork = true; 
	exitflag |= (cmd->target == Command::STOP); 
      }
      else if(tx_enabled &&
	      tx_bits &&
	      (tx_modulation != Command::CW_L) &&
	      (tx_modulation != Command::CW_U) &&
	      (txbuf = tx_stream->get(tx_subs)) != NULL) {
	// get a buffer and 
	buffers[0] = txbuf->data();
	tx_bits->send(buffers, txbuf->size(), md);
	md.start_of_burst = false; 
	didwork = true; 
      }
      else if(tx_enabled &&
	      tx_bits &&
	      !beacon_mode &&
	      ((tx_modulation == Command::CW_L) ||
	       (tx_modulation == Command::CW_U))) {
	cwenv = cw_env_stream->get(cw_subs);
	if(cwenv != NULL) {
	  // modulate a carrier with a cw message
	  doCW(cw_buf, cwenv);
	  // now send it to the USRP
	  buffers[0] = cw_buf->data();
	  tx_bits->send(buffers, cwenv->size(), md);
	  md.start_of_burst = false; 
	  didwork = true; 
	}
	else {
	  // we have an empty CW buffer -- we've run out of text.
	  doCW(cw_buf, zero_env);
	  buffers[0] = cw_buf->data();
	  tx_bits->send(buffers, tx_buffer_size, md); 
	  // are we supposed to tell anybody about this? 
	  if(waiting_to_run_dry) {
	    cmd_stream->put(Command::make(Command::REP, Command::TX_CW_EMPTY, 0));
	    waiting_to_run_dry = false; 
	  }
	}
      }
      else if(tx_enabled &&
	      tx_bits &&
	      beacon_mode &&
	      ((tx_modulation == Command::CW_L) ||
	       (tx_modulation == Command::CW_U))) {
	// modulate a carrier with a constant envelope
	doCW(cw_buf, beacon_env);
	// now send it to the USRP
	buffers[0] = cw_buf->data();
	tx_bits->send(buffers, tx_buffer_size, md);
	md.start_of_burst = false; 
	didwork = true; 
      }
      else if(tx_enabled && 
	      tx_bits) {
	// all other cases -- we still want to send the LO buffer
	buffers[0] = zero_buf->data();
	tx_bits->send(buffers, tx_buffer_size, md);
	didwork = true; 
      }

      if(!didwork) {
	sleep_us(100);
      }
    }

    debugMsg("Leaving\n");
  }


  void USRPTX::doCW(CFBuf out, FBuf envelope)
  {
    unsigned int i;
    std::complex<float> c;
  
    for(i = 0; i < envelope->size(); i++) {
      c = CW_osc.stepOscCF(); 
      (*out)[i] = c * (*envelope)[i] * cw_env_amplitude;
    }
  }

  void USRPTX::setCWFreq(bool usb, double freq)
  {
    // set to - for USB and + for LSB.
    CW_osc.setPhaseIncr((usb ? -1.0 : 1.0) * freq * 2.0 * M_PI / tx_sample_rate);
    // likely to be extremely small... 
  }

  void USRPTX::transmitSwitch(bool tx_on)
  {
    if(tx_on) {
      if(tx_enabled) return;
      waiting_to_run_dry = false;
      md.start_of_burst = true;
      md.end_of_burst = false;
      md.has_time_spec = false; 
      tx_enabled = true; 
    }
    else {
      if(!tx_enabled && !LO_enabled) return;
      if(!LO_enabled) {
	// If LO is enabled, we always send SOMETHING....
	md.end_of_burst = true;
	tx_bits->send(zero_buf->data(), 10, md);
      }
      tx_enabled = false;
      // flush the input stream for us. 
      tx_stream->clear(tx_subs);
    }
  }

  void USRPTX::getTXStreamer()
  {
    tx_bits = usrp->get_tx_stream(*stream_args); 
  }


  void USRPTX::execSetCommand(CmdMsg cmd)
  {
    switch(cmd->target) {
    case Command::TX_MODE:
      tx_modulation = Command::ModulationType(cmd->iparms[0]);
      if(tx_modulation == Command::CW_L) {
	setCWFreq(false, CW_tone_freq); 
      }
      else if(tx_modulation == Command::CW_U) {
	setCWFreq(true, CW_tone_freq); 
      }
      break; 
    case Command::TX_STATE:
      // TX_STATE must be 3 to turn the transmitter on.
      // bit 1 of the command indicates that CTRL has already done the
      // setup for TX <-> RX mode transitions.
      if((cmd->iparms[0] & 0x2) != 0) {  
	transmitSwitch(cmd->iparms[0] == 3);
	cmd_stream->put(Command::make(Command::REP, Command::TX_STATE, tx_enabled ? 1 : 0));
      }
      break;
    case Command::TX_BEACON:
      beacon_mode = (cmd->iparms[0] != 0);
      break;
    case Command::TX_CW_EMPTY:
      waiting_to_run_dry = true; 
      break;
    case Command::TVRT_LO_ENABLE:
      debugMsg("Enable Transverter LO");
      LO_enabled = true; 
      break; 
    case Command::TVRT_LO_DISABLE:
      debugMsg("Disable Transverter LO");
      LO_enabled = false; 
      break;
    default:
      break; 
    }
  }

  void USRPTX::execGetCommand(CmdMsg cmd)
  {
    switch(cmd->target) {
    case Command::TX_STATE:
      cmd_stream->put(Command::make(Command::REP, Command::TX_STATE, tx_enabled ? 1 : 0)); 
      break;
    default:
      break; 
    }
  }

  void USRPTX::execRepCommand(CmdMsg cmd)
  {
    switch(cmd->target) {
    case Command::TVRT_LO_CONFIG:
      debugMsg("LO configured");
      LO_configured = true; 
      break;
    default:
      break;
    }
  }

  /// implement the subscription method
  void USRPTX::subscribe() {

    auto reg = MailBoxRegistry::getRegistrar();
  
    cmd_stream = MailBoxBase::convert<MsgMBox>(reg->get("CMD"));
    cmd_subs = cmd_stream->subscribe();

    tx_stream = MailBoxBase::convert<CFMBox>(reg->get("TX"));
    tx_subs = tx_stream->subscribe();

    cw_env_stream = MailBoxBase::convert<FMBox>(reg->get("CW_ENV"));
    cw_subs = cw_env_stream->subscribe();
  }
}
