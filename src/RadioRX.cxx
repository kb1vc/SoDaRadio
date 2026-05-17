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
  RadioRX::RadioRX(Params * params) :
    Thread("RadioRX")
  {
  }


  void RadioRX::run()
  {
    if(cmd_stream == NULL) {
      throw Radio::Exception(std::string("Never got command stream subscription\n"), 
			     this);	
    }
    if(rx_stream == NULL) {
      throw Radio::Exception(std::string("Never got rx stream subscription\n"),
			     this);	
    }
    if(if_stream == NULL) {
      throw Radio::Exception(std::string("Never got if stream subscription\n"),
			     this);	
    }
  
    bool exitflag = false;

    while(!exitflag) {
      Command * cmd = cmd_stream->get(cmd_subs);
      bool did_command = false;
      bool did_convert = false; 
      if(cmd != NULL) {
	// process the command.
	execCommand(cmd);
	exitflag |= (cmd->target == Command::STOP); 
	cmd_stream->free(cmd);
	did_command = true; 
      }

      if(convert()) {
	did_convert = true; 
      }

      // if we didn't do any work at all, go to sleep
      if (! (did_command || did_convert)) {
	sleep_us(1000);
      }
    }

    // once we get the stop message, quit. 
    closeStream(); 
  }

  void RadioRX::execCommand(Command * cmd)
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


  void RadioRX::execSetCommand(Command * cmd)
  {
    switch(cmd->target) {
    case Command::RX_NCO_FREQ:
      current_IF_tuning = cmd->dparms[0];
      int unit_selector = int(cmd->dparms[1]);
      setNCOFreq(cmd->dparms[0], unit_selector); 
      break;
    case Command::TX_STATE: // SET TX_ON
      if(cmd->iparms[0] == 3) {
	// stop the RX stream.
	// some radios will ignore this. 
	stopStream();
	
	// we're turning the transmitter on.
	// don't enable the IF streamer unless we're in full duplex
	// mode. (Added for satcom and EME folks.)
	enableIFStreamer(cmd->iparms[1] > 0);
      }
      if(cmd->iparms[0] == 2) {
	// start the RX stream.
	debugMsg("In TX OFF -- restart stream");
	// we never stopped the stream
	// but we need to start it the very first time. 
	startStream();
	enableIFStreamer(true); 
	// tell the baseband unit that it is ready to start. 
	cmd_stream->put(new Command(Command::SET, Command::TX_STATE, 
				    4));
      }
      break; 
    default:
      break; 
    }
  }

  void RadioRX::execGetCommand(Command * cmd)
  {
    (void) cmd; 
  }

  void RadioRX::execRepCommand(Command * cmd)
  {
    (void) cmd; 
  }

  /// implement the subscription method
  void RadioRX::subscribeToMailBox(const std::string & mbox_name, 
				   BaseMBox * mbox_p) {
    if(connectMailBox<CmdMBox>(this, cmd_stream, "CMD", mbox_name, mbox_p)) {
      cmd_subs = cmd_stream->subscribe();
    }
    if(connectMailBox<DatMBox>(this, rx_stream, "RX", mbox_name, mbox_p)) {
      // we don't subscribe -- we publish
    }
    if(connectMailBox<DatMBox>(this, if_stream, "IF", mbox_name, mbox_p)) {
      // we don't subscribe -- we publish
    }
  }
}
