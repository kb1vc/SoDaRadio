/*
  Copyright (c) 2012, 2025 Matthew H. Reilly (kb1vc)
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

#include "RadioTX.hxx"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

SoDa::RadioTX::RadioTX(ParamsPtr params, const std::string & name) : SoDa::Thread(name) 
{
  cmd_stream = NULL;
  tx_stream = NULL;
  cw_env_stream = NULL;

  beacon_mode = false; 

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
  beacon_env = SoDa::FBuf::make(tx_buffer_size); 
  zero_env = SoDa::FBuf::make(tx_buffer_size); 
  for(unsigned int i = 0; i < tx_buffer_size; i++) {
    (*beacon_env)[i] = 1.0;
    (*zero_env)[i] = 0.0; 
  }

  // build the cwbuffer
  cw_buf = SoDa::CBuf::make(tx_buffer_size); 

  // set the initial envelope amplitude
  cw_env_amplitude = 0.7;  // more or less sqrt2/2
  
  // build the zero buffer
  zero_buf = SoDa::CBuf::make(tx_buffer_size);
  for(unsigned int i = 0; i < tx_buffer_size; i++) {
    (*zero_buf)[i] = std::complex<float>(0.0, 0.0);
  }

  // build the zero envelope
  for(unsigned int i = 0; i < tx_buffer_size; i++) {
    (*zero_env)[i] = 0;
  }
  
  tx_enabled = false;
}

void SoDa::RadioTX::run()
{
  if((cmd_stream == NULL) || (tx_stream == NULL) || (cw_env_stream == NULL)) {

    throw SoDa::SDR::Exception(std::string("Missing a stream connection.\n"),
			       getSelfPtr());
  }

  bool exitflag = false;
  unsigned int beacon_push_count = 0;
  unsigned int cw_push_count = 0;
  while(!exitflag) {
    bool didwork = false;
    CommandPtr cmd;
    if(cmd_stream->get(cmd_subs, cmd)) {
      execCommand(cmd);
      didwork = true;
      exitflag |= (cmd->target == Command::STOP);
    }
    else if(tx_enabled && beacon_mode) {
      // Carrier beacon: send full-amplitude signal regardless of modulation mode
      if(beacon_push_count == 0) {
        std::cerr << SoDa::Format("RadioTX: beacon first push  tx_mod=%0 tx_enabled=%1\n")
          .addI((int)tx_modulation).addI((int)tx_enabled);
      }
      beacon_push_count++;
      doCW(cw_buf, beacon_env);
      put(cw_buf);
      didwork = true;
    }
    else if(tx_enabled &&
	    (tx_modulation != SoDa::Command::CW_L) &&
	    (tx_modulation != SoDa::Command::CW_U)) {
      CBufPtr txbuf;
      if(tx_stream->get(tx_subs, txbuf)) {
	put(txbuf);
	didwork = true;
      }
    }
    else if(tx_enabled &&
	    ((tx_modulation == SoDa::Command::CW_L) ||
	     (tx_modulation == SoDa::Command::CW_U))) {
      FBufPtr cwenv;
      if(cw_env_stream->get(cw_subs, cwenv)) {
	if(cw_push_count == 0) {
	  std::cerr << SoDa::Format("RadioTX: CW first envelope  tx_mod=%0\n")
	    .addI((int)tx_modulation);
	}
	cw_push_count++;
	doCW(cw_buf, cwenv);
	put(cw_buf);
	didwork = true;
      }
      else {
	// No envelope yet — keep TX stream alive with silence.
	doCW(cw_buf, zero_env);
	put(cw_buf);
	didwork = true;
	if(waiting_to_run_dry) {
	  cmd_stream->put(Command::make(Command::REP, Command::TX_CW_EMPTY, 0));
	  waiting_to_run_dry = false;
	}
      }
    }
    else if(tx_enabled) {
      put(zero_buf);
      didwork = true;
    }

    if(!didwork) {
      sleep_us(100);
    }
  }

  debugMsg("Leaving\n");
}


void SoDa::RadioTX::doCW(SoDa::CBufPtr out, SoDa::FBufPtr envelope)
{
  unsigned int i;
  std::complex<float> c;

  
  for(i = 0; i < envelope->size(); i++) {
    c = CW_osc.stepOscCF(); 
    (*out)[i] = c * (*envelope)[i] * cw_env_amplitude;
  }
}

void SoDa::RadioTX::setCWFreq(bool usb, double freq)
{
  // set to - for USB and + for LSB.
  CW_osc.setPhaseIncr((usb ? -1.0 : 1.0) * freq * 2.0 * M_PI / tx_sample_rate);
  // likely to be extremely small... 
}

void SoDa::RadioTX::execSetCommand(CommandPtr cmd)
{
  switch(cmd->target) {
  case SoDa::Command::TX_MODE:
    tx_modulation = SoDa::Command::ModulationType(cmd->iparms[0]);
    if(tx_modulation == SoDa::Command::CW_L) {
      setCWFreq(false, CW_tone_freq); 
    }
    else if(tx_modulation == SoDa::Command::CW_U) {
      setCWFreq(true, CW_tone_freq); 
    }
    break; 
  case Command::TX_STATE:
    // TX_STATE must be 3 to turn the transmitter on.
    // bit 1 of the command indicates that CTRL has already done the
    // setup for TX <-> RX mode transitions.

    if(cmd->iparms[0] == Command::TX_ON_2) {
	// fiddle this when we move it over to RadioTX. 
      transmitSwitch(true);
      cmd_stream->put(Command::make(Command::REP, Command::TX_STATE,
				    Command::TX_ON_2));
    }
    if(cmd->iparms[0] == Command::TX_OFF_0) {
      transmitSwitch(false); 
      cmd_stream->put(Command::make(Command::REP, Command::TX_STATE,
				    Command::TX_OFF_0));
    }
    break;
  case Command::TX_BEACON:
    beacon_mode = (cmd->iparms[0] != 0);
    if(beacon_mode &&
       (tx_modulation != Command::CW_L) &&
       (tx_modulation != Command::CW_U)) {
      // Non-CW mode beacon: prime the CW oscillator so beacon tone is
      // offset from LO and distinguishable from LO leakthrough.
      setCWFreq(true, CW_tone_freq);
    }
    break;
  case Command::TX_CW_EMPTY:
    waiting_to_run_dry = true; 
    break;
  default:
    break; 
  }
}

void SoDa::RadioTX::execGetCommand(CommandPtr cmd)
{
  switch(cmd->target) {
  case Command::TX_STATE:
    cmd_stream->put(Command::make(Command::REP, Command::TX_STATE, tx_enabled ? 1 : 0)); 
    break;
  default:
    break; 
  }
}

void SoDa::RadioTX::execRepCommand(CommandPtr cmd)
{
  switch(cmd->target) {
  default:
    break;
  }
}

/// implement the subscription method
void SoDa::RadioTX::subscribeToMailBoxes(const std::vector<MailBoxBasePtr> & mailboxes)
{
  for(auto mbox_p : mailboxes) {
    SoDa::MailBoxBase::connect<SoDa::MailBox<CommandPtr>>(mbox_p, "CMDstream",
							  cmd_stream);
    SoDa::MailBoxBase::connect<SoDa::MailBox<CBufPtr>>(mbox_p, "TXstream",
						       tx_stream);
    SoDa::MailBoxBase::connect<SoDa::MailBox<FBufPtr>>(mbox_p, "CWstream",
						       cw_env_stream);
  }

  if(cmd_stream == nullptr) {
    throw SoDa::MissingMailBox("CMD", getSelfPtr());
  }
  else {
    cmd_subs = cmd_stream->subscribe();    
  }
  if(tx_stream == nullptr) {
    throw SoDa::MissingMailBox("TX", getSelfPtr());
  }
  else {
    tx_subs = tx_stream->subscribe();    
  }
  if(cw_env_stream == nullptr) {
    throw SoDa::MissingMailBox("CW", getSelfPtr());
  }
  else {
    cw_subs = cw_env_stream->subscribe();    
  }
}
