/*
  Copyright (c) 2026, Matthew H. Reilly (kb1vc)
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

#include "RadioRX.hxx"
#include <SoDa/Format.hxx>

#include <fstream>

namespace SoDa {
  RadioRX::RadioRX(ParamsPtr params) :
    Thread("RadioRX")
  {
    current_IF_tuning = 0.0;
  }


  void RadioRX::run()
  {
    if(cmd_stream == NULL) {
      throw SDR::Exception(std::string("Never got command stream subscription\n"),
			     getSelfPtr());
    }
    if(rx_stream == NULL) {
      throw SDR::Exception(std::string("Never got rx stream subscription\n"),
			     getSelfPtr());
    }
    if(if_stream == NULL) {
      throw SDR::Exception(std::string("Never got if stream subscription\n"),
			     getSelfPtr());
    }
  
    bool exitflag = false;

    while(!exitflag) {
      CommandPtr cmd;
      bool did_command = false;
      bool did_convert = false;
      if(cmd_stream->get(cmd_subs, cmd)) {
	execCommand(cmd);
	exitflag |= (cmd->target == Command::STOP);
	did_command = true;
      }

      if(downConvert()) {
	did_convert = true;
      }

      // if we didn't do any work at all, go to sleep
      if (! (did_command || did_convert)) {
	sleep_us(1000);
      }
    }

    // once we get the stop message, quit. 
    stopStream();
  }

  void RadioRX::execSetCommand(CommandPtr cmd)
  {
    switch(cmd->target) {
    case Command::RX_IF_FREQ:
      current_IF_tuning = cmd->dparms[0];
      setNCOFreq(cmd->dparms[0]);
      break;
    case Command::TX_STATE: // SET TX_ON
      if(cmd->iparms[0] == Command::TX_ON_1) {
	// stop the RX stream.
	// some radios will ignore this. 
	stopStream();
	
	// we're turning the transmitter on.
	// don't enable the IF streamer unless we're in full duplex
	// mode. (Added for satcom and EME folks.)
	enableIFStreamer(cmd->iparms[1] > 0);

	// tell the transmitter it can turn itself on now. 
	cmd_stream->put(Command::make(Command::SET, Command::TX_STATE,
				    Command::TX_ON_2, cmd->iparms[1]));
      }
      if(cmd->iparms[0] == Command::TX_OFF_1) {
	// start the RX stream.
	debugMsg("In TX OFF -- restart stream");
	// we never stopped the stream
	// but we need to start it the very first time. 
	startStream();
	enableIFStreamer(true); 
	// tell the baseband unit that it is ready to start. 
	cmd_stream->put(Command::make(Command::SET, Command::TX_STATE, 
				    Command::TX_OFF_2));
      }
      break; 
    default:
      break; 
    }
  }

  void RadioRX::execGetCommand(CommandPtr cmd)
  {
    (void) cmd; 
  }

  void RadioRX::execRepCommand(CommandPtr cmd)
  {
    (void) cmd; 
  }

  /// implement the subscription method

  /// implement the subscription method
  void RadioRX::subscribeToMailBoxes(const std::vector<MailBoxBasePtr> & mailboxes) {
    for(auto mbox_p : mailboxes) {
      MailBoxBase::connect<MailBox<CommandPtr>>(mbox_p,
                                                "CMDstream",
                                                cmd_stream); 
      MailBoxBase::connect<MailBox<CBufPtr>>(mbox_p,
                                             "RXstream",
                                             rx_stream); 
      MailBoxBase::connect<MailBox<CBufPtr>>(mbox_p,
                                             "IFstream",
                                             if_stream); 
    }

    if(cmd_stream == nullptr) {
      throw MissingMailBox("CMD", getSelfPtr());    
    }
    else {
      cmd_subs = cmd_stream->subscribe();
    }

    // we publish on these streams. 
    if(rx_stream == nullptr) {
      throw MissingMailBox("RX", getSelfPtr());
    }
    if(if_stream == nullptr) {
      throw MissingMailBox("IF", getSelfPtr());
    }
  }
}
