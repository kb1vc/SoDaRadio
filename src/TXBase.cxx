/*
  Copyright (c) 2022, Matthew H. Reilly (kb1vc)
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

#include "TXBase.hxx"
#include "MailBoxRegistry.hxx"
#include "MailBoxTypes.hxx"
#include "Radio.hxx"
#include <SoDa/Format.hxx>

// Mac OSX doesn't have a clock_gettime, it has
// the microsecond resolution gettimeofday. 

namespace SoDa {

  TXBase::TXBase(const std::string & name) : Thread(name)
  {
    cmd_stream = nullptr;
    tx_stream = nullptr;
    cw_env_stream = nullptr;
    
    is_initialized = false;
  }

  void TXBase::run()
  {
    if(cmd_stream == nullptr) {
      throw Radio::Exception(SoDa::Format("Never got command stream subscription\n").str(), 
				   this);	
    }
    if(tx_stream == nullptr) {
      throw Radio::Exception(std::string("Never got tx stream subscription\n"),
			     this);	
    }
    if(cw_env_stream == nullptr) {
      throw Radio::Exception(std::string("Never got cw envelope stream subscription\n"),
			     this);	
    }
   
    if(!is_initialized) {
      init();
      is_initialized = true; 
    }
    
    bool exitflag = false;
    unsigned int cmds_processed = 0;
    unsigned int loopcount = 0; 
    while(!exitflag) {
      loopcount++; 
      CmdMsg cmd = cmd_stream->get(cmd_subs);
      if(cmd != nullptr) {
	// process the command.
	if((cmds_processed & 0xff) == 0) {
	  debugMsg(SoDa::Format("TXBase processed %0 commands").addI(cmds_processed));
	}
	cmds_processed++; 
	execCommand(cmd);
	exitflag |= (cmd->target == Command::STOP); 
      }
      
      if(!runTick()) {
	sleep_us(getSleepDuration());
      }
    }
  }

  void TXBase::execCommand(CmdMsg cmd)
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

  /// implement the subscription method
  void TXBase::subscribe() {

    auto reg = MailBoxRegistry::getRegistrar();
    cmd_stream = MailBoxBase::convert<MsgMBox>(reg->get("CMD"));
    cmd_subs = cmd_stream->subscribe();

    tx_stream = MailBoxBase::convert<CFMBox>(reg->get("TX"));
    tx_subs = tx_stream->subscribe();

    cw_env_stream = MailBoxBase::convert<FMBox>(reg->get("CW_ENV"));
    cw_subs = cw_env_stream->subscribe();
  }
}
