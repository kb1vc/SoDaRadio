/*
  Copyright (c) 2012, 2023 Matthew H. Reilly (kb1vc)
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
#include "AudioQtRX.hxx"

#define _USE_MATH_DEFINES
#include <cmath>


namespace SoDa {
  AudioQtRX::AudioQtRX(unsigned int _sample_rate,
		   unsigned int _sample_count_hint, 
		   std::string audio_sock_basename, 
		   std::string audio_port_name) :
    AudioIfc(_sample_rate, _sample_count_hint, "AudioQtRX Qt Interface") {

    std::cerr << "Creating AudioQtRX\n";
    
    setupNetwork(audio_sock_basename); 

    ang = 0.0; 
    ang_incr = 2.0 * M_PI / 48.0; 
  }

  void AudioQtRX::setupNetwork(std::string audio_sock_basename) 
  {
    std::string sockname = audio_sock_basename + "_rxa";
    audio_rx_socket = new SoDa::UD::ServerSocket(sockname);
    audio_rx_socket->setDebug(true);
  }


  bool AudioQtRX::sendBufferReady(unsigned int len)  {
    return true; 
  }


  int AudioQtRX::send(void * buf, unsigned int len, bool when_ready) {
    int ret;
    ret = audio_rx_socket->put(buf, len, false);
    return ret; 
  }

}
