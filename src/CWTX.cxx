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

#include "CWTX.hxx"
#include "CWGenerator.hxx"

namespace SoDa {
  
  CWTX::CWTX(ParamsPtr params) : Thread("CWTX")
  {
    cwtxt_stream = NULL;
    cw_env_stream = NULL;
    cmd_stream = NULL;

    // get the controlling audio parameters
    // like the audio sample rate and buffer size
    rf_sample_rate = params->getTXRate();
    rf_buffer_size = params->getRFBufferSize();
  
  
    sent_char_count = 0;
  }

  void CWTX::run()
  {
    bool exitflag = false;
    CommandPtr cmd, txtcmd; 

    if((cmd_stream == NULL) || (cw_env_stream == NULL) || (cwtxt_stream == NULL)) {
      throw Radio::Exception(std::string("Missing a stream connection.\n"), 
			     getSelfPtr());	
    }

    // setup the CW generator unit
    cwgen = CWGenerator::make(cw_env_stream, rf_sample_rate, rf_buffer_size);
  
    while(!exitflag) {
      bool workdone = false; 
      while(cmd_stream->get(cmd_subs, cmd)) {
	// process the command.
	execCommand(cmd);
	exitflag |= (cmd->target == Command::STOP); 
	workdone = true; 
      }

      while(cwtxt_stream->get(cwtxt_subs, txtcmd)) {
	// pend the text to the text queue
	execCommand(txtcmd);
	exitflag |= (txtcmd->target == Command::STOP); 
	workdone = true; 
      }

      bool dochar = cwgen->readyForMore();

      // if we've got text in the queue and we're ready
      // for more outbound envelope, send the character
      if(dochar) {
	workdone |= sendAvailChar();
      }

      if(!workdone) {
	usleep(1000); 
      }
    }
  }

  bool CWTX::sendAvailChar()
  {
    // if we are in CW mode
    if(!txmode_is_cw) return false;
    if(!tx_on) return false;

    char outchar;
    bool sent_char = false; 
    // if there are any characters to send
    // and the generator is not stuffed
    int itercount = 0; 
    char tbuf[2];
    tbuf[1] = '\000';
    while (cwgen->readyForMore()) {
      itercount++; 
      if(1) {
	std::lock_guard<std::mutex> mt_lock(text_mutex);
	if(text_queue.empty()) return false;

	outchar = text_queue.front(); text_queue.pop();
      }
      if(outchar != '\003') {
	cwgen->sendChar(outchar);
	sent_char = true;
	tbuf[0] = outchar; 
	cmd_stream->put(Command::make(Command::REP,
				      Command::CW_CHAR_SENT,
				      tbuf, sent_char_count++));
      }
      else {
	cmd_stream->put(Command::make(Command::SET,
				      Command::TX_CW_EMPTY,
				      0));
      }
    }

    return sent_char; 
  }

  void CWTX::execGetCommand(CommandPtr cmd)
  {
    switch(cmd->target) {
    case Command::TX_STATE:
      break; 
    case Command::TX_BEACON:
      break; 
    case Command::TX_MODE:
      break;
    case Command::TX_CW_SPEED:
      break;
    case Command::TX_CW_TEXT:
      break; 
    case Command::TX_CW_FLUSHTEXT:
      break; 
    default:
      break; 
    }
  }

  void CWTX::execSetCommand(CommandPtr cmd)
  {
    Command::ModulationType txmode;
  
    switch(cmd->target) {
    case Command::TX_STATE:
      if(cmd->iparms[0] == 3) tx_on = true;
      else tx_on = false; 
      break; 
    case Command::TX_BEACON:
      if(cmd->iparms[0] == 1) {
	old_txmode_is_cw = txmode_is_cw; 
	txmode_is_cw = false;
      }
      else {
	txmode_is_cw = old_txmode_is_cw; 
      }
      break; 
    case Command::TX_MODE:
      txmode = Command::ModulationType(cmd->iparms[0]);
      if((txmode == Command::CW_L) || (txmode == Command::CW_U)) {
	txmode_is_cw = true;
	old_txmode_is_cw = true; 
      }
      else {
	txmode_is_cw = false; 
	old_txmode_is_cw = false;
      }
      break;
    case Command::TX_CW_SPEED:
      cwgen->setCWSpeed(cmd->iparms[0]); 
      break;
    case Command::TX_CW_TEXT:
      enqueueText(cmd->sparm); 
      break;
    case Command::TX_CW_MARKER:
      // set the ETX marker. 
      enqueueText("\003");
      // pend the iparm to the break notification queue
      if(1) {
	std::lock_guard<std::mutex> lock(break_id_mutex);
	break_notification_id_queue.push(cmd->iparms[0]);
      }
      break; 
    case Command::TX_CW_FLUSHTEXT:
      clearTextQueue(); 
      break; 
    default:
      break; 
    }
  }

  void CWTX::enqueueText(const char * buf)
  {
    std::lock_guard<std::mutex> lock(text_mutex);
  
    const char * cp = buf;
    int i;
    for(i = 0; i < Command::getMaxStringLen(); i++) {
      if(*cp == '\000') break;
      // enqueue a character. 
      text_queue.push(*cp);
      cp++; 
    }
  }

  void CWTX::clearTextQueue()
  {
    // empty the text_queue
    if(1) {
      std::lock_guard<std::mutex> lock(text_mutex);
  
      int deleted_count = text_queue.size();
  
      text_queue = std::queue<char>(); 
      sent_char_count += deleted_count; 
    }
    CommandPtr ncmd = Command::make(Command::REP,
				    Command::TX_CW_FLUSHTEXT,
				    sent_char_count);
    cmd_stream->put(ncmd); 
  
  }

  void CWTX::execRepCommand(CommandPtr cmd)
  {
    switch(cmd->target) {
    case Command::TX_CW_EMPTY:
      // The CW generator has run out of things to send. 
      {
	std::lock_guard<std::mutex> mt_lock(break_id_mutex);
	// we got an ETX marker -- get the next completion code from
	// the break queue and send it back in a REPORT
	if(!break_notification_id_queue.empty()) {
	  int bkid = break_notification_id_queue.front(); break_notification_id_queue.pop();
	  CommandPtr ncmd = Command::make(Command::REP,
					  Command::TX_CW_MARKER,
					  bkid);
	  cmd_stream->put(ncmd); 	  
	}
      }
      break;
    default:
      break;
    }
  }


  /// implement the subscription method
  void CWTX::subscribeToMailBoxes(const std::vector<MailBoxBasePtr> & mailboxes)
  {
    for(auto mbox_p : mailboxes) {
      cmd_stream = MailBoxBase::convert<MailBox<CommandPtr>>(mbox_p, "CMDstream");
      if(cmd_stream != nullptr) {
	cmd_subs = cmd_stream->subscribe();
      }
      cwtxt_stream = MailBoxBase::convert<MailBox<CommandPtr>>(mbox_p, "RXstream");
      if(cwtxt_stream != nullptr) {
	cwtxt_subs = cwtxt_stream->subscribe();
      }
      cw_env_stream = MailBoxBase::convert<MailBox<FBufPtr>>(mbox_p, "CWstream");
    }

    if(cmd_stream == nullptr) {
      throw MissingMailBox("CMD", getSelfPtr());
    }
    if(cwtxt_stream == nullptr) {
      throw MissingMailBox("RX", getSelfPtr());
    }
    if(cw_env_stream == nullptr) {
      throw MissingMailBox("RX", getSelfPtr());
    }
  }
}
