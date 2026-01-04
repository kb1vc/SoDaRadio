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

#include "Debug.hxx"
#include "AudioQt.hxx"

#define _USE_MATH_DEFINES
#include <cmath>


namespace SoDa {
  AudioQt::AudioQt(unsigned int _sample_rate,
		   unsigned int _sample_count_hint, 
		   std::string audio_sock_basename, 
		   std::string audio_port_name) :
    AudioIfc(_sample_rate, _sample_count_hint, "AudioQt Qt Interface") {

    setupNetwork(audio_sock_basename); 

    ang = 0.0; 
    ang_incr = 2.0 * M_PI / 48.0; 
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
    // call recv until we get an empty buffer
    const int buf_size = 1024;
    char junk_buffer[buf_size];
    while(recv(junk_buffer, buf_size) != 0) {
      // do nothing.  we're just going to throw away the bits.
    }
  }
  
  bool AudioQt::sendBufferReady(unsigned int len)  {
    return true; 
  }

  int AudioQt::recvBufferReady(unsigned int len) {
    return true; 
  }

  int AudioQt::recv(void * buf, unsigned int len, bool when_ready) {
    // read a buffer from the tx_socket.
    auto got_bytes = tx_socket->get(buf, len);
    if(got_bytes < 0) {
      if((errno != EWOULDBLOCK) && (errno != EAGAIN)) {
	// we've got a problem.  Wonder what it is? 
      }
      got_bytes = 0; 
    }
    std::ignore = when_ready;
    return got_bytes; 
  }

  int AudioQt::send(void * buf, unsigned int len, bool when_ready) {
    int ret;
    ret = audio_rx_socket->put(buf, len, false);
    return ret; 
  }
}
