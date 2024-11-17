/*
  Copyright (c) 2012, 2024 Matthew H. Reilly (kb1vc)
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

#include "Debug.hxx"
#include "AudioQtTX.hxx"

#include <SoDa/Format.hxx>

#define _USE_MATH_DEFINES
#include <cmath>


namespace SoDa {
  AudioQtTX::AudioQtTX(unsigned int _sample_rate,
		       unsigned int _audio_buffer_size,
		       std::string audio_sock_basename) :

    Thread("AudioQtTX Interface") {

    sample_rate = _sample_rate;
    audio_buffer_size = _audio_buffer_size;

    tx_on = false;
    cw_mode = false; 
    
    std::cerr << "Creating AudioQtTX\n";    
    
    // setup socket
    audio_tx_socket = new SoDa::UD::ServerSocket(audio_sock_basename + "_audioTX");
    
  }


  int AudioQtTX::getBuffer(FVecPtr & buf, unsigned int len, bool when_ready) {
    if(when_ready && !bufferReady()) return 0;

    // do we have anything in the buffer list?
    if(bufferReady()) {
      auto rdy_buf = buffer_list.front();
      buffer_list.pop();

      // copy the data from rdy_buf to buf
      *buf = *rdy_buf;
      // free the buffer
      rdy_buf = nullptr; 
    }
    else {
      return 0; 
    }
    return buf->size();
  }

  bool AudioQtTX::bufferReady() {
    std::lock_guard<std::mutex> lock(buf_list_mutex);
    return !buffer_list.empty();
  }

  void AudioQtTX::setupCurrentBuf() {
    current_buf = makeFVec(audio_buffer_size);
    current_buf_end_ptr = (char*) (current_buf.get()->data());
    current_buf_bytes_left = audio_buffer_size; 
  }
  
  void AudioQtTX::run() {
    // look for incoming data from the TX audio socket.
    // if we are "on" push the data onto the buffer list.

    // but first, tell the GUI how big our audio buffer needs to be
    cmd_stream->put(Command::make(Command::SET, Command::AUDIO_BUF_SIZE, (int) audio_buffer_size));
    // and the sample rate
    cmd_stream->put(Command::make(Command::SET, Command::AUDIO_SAMPLE_RATE, (int) sample_rate));    

    setupCurrentBuf(); 
  
    
    bool exitflag = false; 
    
    CommandPtr cmd; 
    
    while(!exitflag) {
      if((cmd = cmd_stream->get(this)) != nullptr) {
	exitflag |= (cmd->target == Command::STOP);
	execCommand(cmd);
	cmd = nullptr; 
      }
      
      // are there any audio buffers coming in?
      // are we awake?  if so, pend them to the buffer list
      while(audio_tx_socket->isReady()) {
	// fill the current buffer, and start a new one when
	// we need to.  This allows the QtAudio side to send buffers
	// that aren't necessarily the size we've planned on.
	int stat = audio_tx_socket->get(current_buf_end_ptr, current_buf_bytes_left);
	if(stat >= 0) {
	  current_buf_bytes_left -= stat;
	  current_buf_end_ptr += stat;
	  if(current_buf_bytes_left == 0) {
	    // pend the buffer if tx is on and we have filled the current buffer. 
	    if(tx_on) {
	      std::lock_guard<std::mutex> lock(buf_list_mutex);	  
	      buffer_list.push(current_buf);
	    }

	    setupCurrentBuf(); 
	  }
	}
	else if (stat < 0) {
	  // the socket is broken
	  std::cerr << SoDa::Format("OH NO. Audio TX socket error: get stat = %0\n")
	    .addI(stat)
	    ;
	}
      }
    }
  }

  void AudioQtTX::execSetCommand(CommandPtr cmd) {
    
    switch (cmd->target) {
    case Command::TX_MODE:
      {
	// we only care about CW mode or NOT CW mode.
	auto tx_mode = Command::ModulationType(cmd->iparms[0]);
	if((tx_mode == Command::CW_L) || (tx_mode == Command::CW_U)) {
	  cw_mode = true; 
	}
	else {
	  cw_mode = false; 
	}
      }
      break; 
    case Command::TX_STATE:
      if(cmd->iparms[0] == Command::TX_ON) {
	// don't turn on audio if we're in cw mode
	if(!cw_mode) {
	  tx_on = true; 
	}
      }
      else if(cmd->iparms[0] == Command::RX_READY) {
	tx_on = false;
	// clear buffer list
	clearBufferList();
      }
    default:
      break; 
    }
  }

  void AudioQtTX::clearBufferList() {
    std::lock_guard<std::mutex> lock(buf_list_mutex);    
    while(!buffer_list.empty()) {
      auto bp = buffer_list.front();
      buffer_list.pop();
      bp = nullptr; 
    }
  }

  
  /// implement the subscription method
  void AudioQtTX::subscribeToMailBoxList(MailBoxMap & mailboxes)
  {
    // we need the cmd mailbox so that we can tell the GUI what
    // the audio buffer size should be.
    cmd_stream = connectMailBox<CmdMBox>(this, "CMD", mailboxes);    
  }

}
