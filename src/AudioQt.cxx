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
#include <unistd.h>


namespace SoDa {
  AudioQt::AudioQt(unsigned int _sample_rate,
		   unsigned int _sample_count_hint,
		   std::string audio_sock_basename) :
    AudioIfc(_sample_rate, _sample_count_hint, "AudioQt Qt Interface"),
    Thread("AudioQt")
  {

    setupNetwork(audio_sock_basename);

    // 2 seconds of float32 mono audio — absorbs QAudioSource bursts and
    // feeds BaseBandTX at a steady rate without starvation gaps.
    audio_cbuffer_p = new SoDa::CircularBuffer<char>(_sample_rate * sizeof(float) * 2);

    ang = 0.0;
    ang_incr = 2.0 * M_PI / 48.0;

    cmd_stream = nullptr;

    ignore_tx_data = true;
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
    ignore_tx_data = true;
    audio_cbuffer_p->clear();
  }

  void AudioQt::wakeIn() {
    ignore_tx_data = false;
  }

  bool AudioQt::sendBufferReady(unsigned int len)  {
    return true;
  }

  bool AudioQt::recvBufferReady(unsigned int len) {
    return audio_cbuffer_p->numElements() >= (size_t)(len * sizeof(float));
  }

  int AudioQt::recv(std::vector<float> & buf, bool when_ready) {
    size_t bytes_needed = sample_count_hint * sizeof(float);
    if(audio_cbuffer_p->numElements() < bytes_needed) return 0;
    buf.resize(sample_count_hint);
    audio_cbuffer_p->get(reinterpret_cast<char*>(buf.data()), bytes_needed);
    return (int)sample_count_hint;
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
    if(cmd_stream == nullptr) {
      throw SoDa::SDR::Exception(std::string("Missing a stream connection.\n"),
			     getSelfPtr());
    }

    // Read in chunks matching the consumer's buffer size for efficiency.
    const size_t READ_CHUNK = sample_count_hint * sizeof(float);
    std::vector<uint8_t> tmp_buf(READ_CHUNK);

    while(!exit_flag) {
      bool didwork = false;
      if(cmd_stream->get(cmd_subs, cmd)) {
	exit_flag |= (cmd->target == Command::STOP);
	didwork = true;
      }
      if(!exit_flag && audio_tx_socket->isReady()) {
	int stat = audio_tx_socket->get(tmp_buf.data(), READ_CHUNK, false);
	if(stat > 0) {
	  audio_cbuffer_p->put(reinterpret_cast<char*>(tmp_buf.data()), stat);
	  didwork = true;
	}
      }
      if(!didwork) usleep(500);
    }
  }
}
