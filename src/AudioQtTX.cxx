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
  }


  int AudioQtTX::recv(FVecPtr & buf, unsigned int len, bool when_ready) {
    if(when_ready && !recvBufferReady(len)) return 0;

    // do we have anything in the buffer list?
    if(!buffer_list.empty()) {
      auto rdy_buf = buffer_list.front();
      buffer_list.pop_front();

      // copy the data from rdy_buf to buf
      *buf = *rdy_buf;
      // free the buffer
      rdy_buf = nullptr; 
    }
    else {
      return 0; 
    }
  }


  void AudioQtTX::run() {
    // look for incoming data from the TX audio socket.
    // if we are "on" push the data onto the buffer list.

    // but first, tell the GUI how big our audio buffer needs to be
    cmd_stream->put(Command::make(Command::SET, Command::AUDIO_BUF_SIZE, audio_buffer_size));
    // and the sample rate
    cmd_stream->put(Command::make(Command::SET, Command::AUDIO_SAMPLE_RATE, sample_rate));    

    auto ab_bytes = audio_buffer_size * sizeof(float);     
    auto current_buf = makeFVec(audio_buffer_size);
    
    bool exitflag = false; 
    
    while(!exitflag) {
      if((cmd = cmd_stream->get(this)) != nullptr) {
	exitflag |= (cmd->target == Command::STOP);
	execCommand(cmd);
	cmd = nullptr; 
      }
      
      // are there any audio buffers coming in?
      // are we awake?  if so, pend them to the buffer list
      if(audio_tx_socket->isReady()) {
	int stat = audio_tx_socket->get(current_buf->data(), ab_bytes);
	if((stat == ab_bytes) && tx_on) {
	  // pend the incoming buffer onto the list
	  buffer_list.push_back(current_buf);
	  current_buf = makeFVec(audio_buffer_size);
	}
	else if (stat <= 0) {
	  // it was an empty socket. 
	}
	else {
	  // the buffer was the wrong size
	  std::cerr << SoDa::Format("OH NO.  Bad QtTX buffer size! was %0 should have been %1\n")
	    .addI(stat)
	    .addI(audio_buffer_size * sizeof(float))
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
    while(!buffer_list.empty()) {
      auto bp = buffer_list.front();
      buffer_list.pop_front();
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
