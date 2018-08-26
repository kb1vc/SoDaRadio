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

#include "CWTX.hxx"
#include "CWGenerator.hxx"

SoDa::CWTX::CWTX(Params * params, CmdMBox * _cwtxt_stream, DatMBox * _cw_env_stream, CmdMBox * _cmd_stream) : SoDa::SoDaThread("CWTX")
{
  cwtxt_stream = _cwtxt_stream;
  cwtxt_subs = cwtxt_stream->subscribe();

  cw_env_stream = _cw_env_stream;

  cmd_stream = _cmd_stream;
  cmd_subs = cmd_stream->subscribe();


  // get the controlling audio parameters
  // like the audio sample rate and buffer size
  rf_sample_rate = params->getTXRate();
  rf_buffer_size = params->getRFBufferSize();
  
  // setup the CW generator unit
  cwgen = new SoDa::CWGenerator(_cw_env_stream, rf_sample_rate, rf_buffer_size);
  
  sent_char_count = 0;
}

void SoDa::CWTX::run()
{
  bool exitflag = false;
  Command * cmd, *txtcmd; 
  
  while(!exitflag) {
    bool workdone = false; 
    while((cmd = cmd_stream->get(cmd_subs)) != NULL) {
      // process the command.
      execCommand(cmd);
      exitflag |= (cmd->target == Command::STOP); 
      cmd_stream->free(cmd);
      workdone = true; 
    }

    while((txtcmd = cwtxt_stream->get(cwtxt_subs)) != NULL) {
      // pend the text to the text queue
      execCommand(txtcmd);
      exitflag |= (txtcmd->target == Command::STOP); 
      cwtxt_stream->free(txtcmd);
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

bool SoDa::CWTX::sendAvailChar()
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
      cmd_stream->put(new SoDa::Command(SoDa::Command::REP,
					SoDa::Command::CW_CHAR_SENT,
					tbuf, sent_char_count++));
    }
    else {
      cmd_stream->put(new SoDa::Command(SoDa::Command::SET,
					SoDa::Command::TX_CW_EMPTY,
					0));
    }
  }

  return sent_char; 
}

void SoDa::CWTX::execGetCommand(Command * cmd)
{
  switch(cmd->target) {
  case SoDa::Command::TX_STATE:
    break; 
  case SoDa::Command::TX_BEACON:
    break; 
  case SoDa::Command::TX_MODE:
    break;
  case SoDa::Command::TX_CW_SPEED:
    break;
  case SoDa::Command::TX_CW_TEXT:
    break; 
  case SoDa::Command::TX_CW_FLUSHTEXT:
    break; 
  default:
    break; 
  }
}

void SoDa::CWTX::execSetCommand(Command * cmd)
{
  SoDa::Command::ModulationType txmode;
  
  switch(cmd->target) {
  case SoDa::Command::TX_STATE:
    if(cmd->iparms[0] == 3) tx_on = true;
    else tx_on = false; 
    break; 
  case SoDa::Command::TX_BEACON:
    if(cmd->iparms[0] == 1) {
      old_txmode_is_cw = txmode_is_cw; 
      txmode_is_cw = false;
    }
    else {
      txmode_is_cw = old_txmode_is_cw; 
    }
    break; 
  case SoDa::Command::TX_MODE:
    txmode = SoDa::Command::ModulationType(cmd->iparms[0]);
    if((txmode == SoDa::Command::CW_L) || (txmode == SoDa::Command::CW_U)) {
      txmode_is_cw = true;
      old_txmode_is_cw = true; 
    }
    else {
      txmode_is_cw = false; 
      old_txmode_is_cw = false;
    }
    break;
  case SoDa::Command::TX_CW_SPEED:
    cwgen->setCWSpeed(cmd->iparms[0]); 
    break;
  case SoDa::Command::TX_CW_TEXT:
    enqueueText(cmd->sparm); 
    break;
  case SoDa::Command::TX_CW_MARKER:
    // set the ETX marker. 
    enqueueText("\003");
    // pend the iparm to the break notification queue
    if(1) {
      std::lock_guard<std::mutex> lock(break_id_mutex);
      break_notification_id_queue.push(cmd->iparms[0]);
    }
    break; 
  case SoDa::Command::TX_CW_FLUSHTEXT:
    clearTextQueue(); 
    break; 
  default:
    break; 
  }
}

void SoDa::CWTX::enqueueText(const char * buf)
{
  std::lock_guard<std::mutex> lock(text_mutex);
  
  const char * cp = buf;
  int i;
  for(i = 0; i < SoDa::Command::getMaxStringLen(); i++) {
    if(*cp == '\000') break;
    // enqueue a character. 
    text_queue.push(*cp);
    cp++; 
  }
}

void SoDa::CWTX::clearTextQueue()
{
  // empty the text_queue
  if(1) {
    std::lock_guard<std::mutex> lock(text_mutex);
  
    int deleted_count = text_queue.size();
  
    text_queue = std::queue<char>(); 
    sent_char_count += deleted_count; 
  }
  SoDa::Command * ncmd = new SoDa::Command(SoDa::Command::REP,
					   SoDa::Command::TX_CW_FLUSHTEXT,
					   sent_char_count);
  cmd_stream->put(ncmd); 
  
}

void SoDa::CWTX::execRepCommand(Command * cmd)
{
  switch(cmd->target) {
  case SoDa::Command::TX_CW_EMPTY:
    // The CW generator has run out of things to send. 
    {
      std::lock_guard<std::mutex> mt_lock(break_id_mutex);
      // we got an ETX marker -- get the next completion code from
      // the break queue and send it back in a REPORT
      if(!break_notification_id_queue.empty()) {
	int bkid = break_notification_id_queue.front(); break_notification_id_queue.pop();
	SoDa::Command * ncmd = new SoDa::Command(SoDa::Command::REP,
						 SoDa::Command::TX_CW_MARKER,
						 bkid);
	cmd_stream->put(ncmd); 	  
      }
    }
    break;
  default:
    break;
  }
}
