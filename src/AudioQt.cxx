/*
  Copyright (c) 2012, 2025, 2026 Matthew H. Reilly (kb1vc)
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
#include "AudioQt.hxx"
#include "SoDaBase.hxx"
#include "SoDaThread.hxx"

#define _USE_MATH_DEFINES
#include <cmath>


namespace SoDa {
  AudioQt::AudioQt(unsigned int _sample_rate,
		   unsigned int _sample_count_hint, 
		   std::string audio_sock_basename) :
    AudioIfc(_sample_rate, _sample_count_hint, "AudioQt Qt Interface"),
    Thread("AudioQt")
  {

    setupNetwork(audio_sock_basename); 

    setupBuffers(_sample_count_hint);
    
    ang = 0.0; 
    ang_incr = 2.0 * M_PI / 48.0; 

    cmd_stream = nullptr;

    ignore_tx_data = true; 
  }

  FloatVecPtr AudioQt::makeBuffer(unsigned int bsize) {
      return std::shared_ptr<std::vector<float>>(new std::vector<float>(bsize));    
  }

  void AudioQt::setupBuffers(unsigned int bsize) {
    for(int i = 0; i < initial_pool_size; i++) {
      auto nbufp = makeBuffer(bsize);
      free_audio_bufs.push(nbufp);
    }
  }

  void AudioQt::setupNetwork(std::string audio_sock_basename) 
  {
    // both sockets are servers.. makes things simpler that way. 
    std::string rx_sockname = audio_sock_basename + "_rxa";
    audio_rx_socket = std::shared_ptr<UD::ServerSocket>(new UD::ServerSocket(rx_sockname));
    
    std::string tx_sockname = audio_sock_basename + "_txa";
    audio_tx_socket = UD::ServerSocket::make(tx_sockname); 
  }

  void AudioQt::sleepIn() {
    std::lock_guard<std::mutex> tx_data_lock(tx_data_mutex);
    ignore_tx_data = true; 
    while(incoming_audio_bufs.size() != 0) {
      free_audio_bufs.push(incoming_audio_bufs.front());
      incoming_audio_bufs.pop();
    }
  }

  void AudioQt::wakeIn() {
    std::lock_guard<std::mutex> tx_data_lock(tx_data_mutex);
    ignore_tx_data = false; 
  }
  bool AudioQt::sendBufferReady(unsigned int len)  {
    return true; 
  }

  bool AudioQt::recvBufferReady(unsigned int len) {
    std::lock_guard<std::mutex> tx_data_lock(tx_data_mutex);    
    return incoming_audio_bufs.size() != 0;
  }

  int AudioQt::recv(std::vector<float> & buf, bool when_ready) {
    std::lock_guard<std::mutex> tx_data_lock(tx_data_mutex);
    // we only return one size of buffer
    if(incoming_audio_bufs.size() == 0) return 0;
    
    // read a buffer from the incoming queue
    auto abufp = incoming_audio_bufs.front();
    incoming_audio_bufs.pop();
    // copy the buffer. sigh. 
    buf = *abufp;
    // put the old buffer on the free list
    free_audio_bufs.push(abufp);
    
    return buf.size();
  }

  int AudioQt::send(void * buf, unsigned int len, bool when_ready) {
    int ret;
    ret = audio_rx_socket->put(buf, len, false);
    return ret; 
  }

  void AudioQt::subscribeToMailBoxes(const std::vector<MailBoxBasePtr> & mailboxes)
  {
    for(auto mbox_p : mailboxes) {
      MailBoxBase::connect<MailBox<CommandPtr>>(mbox_p,
						"CMDstream",
						cmd_stream); 
    }

    if(cmd_stream == nullptr) {
      throw MissingMailBox("CMD", getSelfPtr());    
    }
    else {
      cmd_subs = cmd_stream->subscribe();
    }
  }

  void AudioQt::run()
  {
    bool exit_flag = false;
    CommandPtr cmd; 
    FloatVecPtr cur_buf_ptr = nullptr;
    if(cmd_stream == nullptr) {
      throw SoDa::SDR::Exception(std::string("Missing a stream connection.\n"),
			     getSelfPtr());	
    }
    
    // now poll various places
    // track progress in bytes (floats are 4 bytes each)
    unsigned int bytes_left = 0;
    unsigned int bytes_so_far = 0;

    while(!exit_flag) {
      if(cmd_stream->get(cmd_subs, cmd)) {
	exit_flag |= (cmd->target == Command::STOP);
      }
      else {
	// we don't want to block on an empty input
	// from the audio source, so we test
	if(audio_tx_socket->isReady()) {
	  if(cur_buf_ptr == nullptr) {
	    cur_buf_ptr = getFreeBuffer();
	    cur_buf_ptr->resize(sample_count_hint);
	    bytes_left = sample_count_hint * sizeof(float);
	    bytes_so_far = 0;
	  }
	  // now get a buffer full.
	  // bstart is a byte pointer into the float buffer
	  auto bstart = reinterpret_cast<uint8_t*>(cur_buf_ptr->data()) + bytes_so_far;
	  int stat = audio_tx_socket->get(bstart, bytes_left, false);
	  if(stat > 0) {
	    bytes_so_far += stat;
	    bytes_left -= stat;
	    if(bytes_left == 0) {
	      std::lock_guard<std::mutex> tx_data_lock(tx_data_mutex);
	      incoming_audio_bufs.push(cur_buf_ptr);
	      cur_buf_ptr = nullptr;
	    }
	  }
	}
      }
    }
  }
  
  FloatVecPtr AudioQt::getFreeBuffer() {
    std::lock_guard<std::mutex> tx_data_lock(tx_data_mutex);
    if(free_audio_bufs.size() == 0) {
      for(int i = 0; i < 10; i++) {
	free_audio_bufs.push(makeBuffer(sample_count_hint));
      }
    }

    auto ret = free_audio_bufs.front();
    free_audio_bufs.pop();
    return ret; 
  }
}
