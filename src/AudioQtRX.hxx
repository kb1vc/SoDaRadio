#pragma once

/*
  Copyright (c) 2012,2024 Matthew H. Reilly (kb1vc)
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

#include "SoDaBase.hxx"
#include "UDSockets.hxx"
#include <string>
#include <mutex>
#include <iostream>
#include <stdexcept>

#include <tuple>

namespace SoDa {
  /**
   * @class AudioQtRX
   *
   * @brief Qt audio interface class
   *
   * AudioQt implements the interface specified by AudioIfc. When BasebandRX
   * posts data to the audio interface, it is transmitted via a unix domain
   * socket to the GUI where the audio output device is managed. 
   * 
   */
  class AudioQtRX : public Debug, public Base {
  public:
    /**
     * constructor
     * @param _sample_rate in Hz  48000 is a good choice
     * @param _sample_count_hint  the size of the buffers passed to and from
     *                              the audio device (in samples)
     * @param audio_sock_basename starting string for the unix-domain socket 
     *                            that carries the audio stream from the SoDaServer (radio) process
     */
    AudioQtRX(unsigned int _sample_rate,
	      unsigned int _sample_count_hint = 1024,
	      std::string audio_sock_basename = std::string("soda_"));

    ~AudioQtRX() {
      delete audio_rx_socket;
    }
    
    /**
     * send -- send a buffer to the audio output
     * @param buf buffer of type described by the DataFormat selected at init
     * @param len number of elements in the buffer to send
     * @param when_ready if true, test with sendBufferReady and return 0 if not ready
     * otherwise perform the send regardless.
     * @return number of elements transferred to the audio output
     */
    int send(void * buf, unsigned int len, bool when_ready = false);

    /**
     * sendBufferReady -- is there enough space in the audio device
     *                    send buffer for a call from send?
     * @param len the number of samples that we wish to send
     * @return true if there is sufficient space. 
     */
    bool sendBufferReady(unsigned int len);

    /**
     * stop the output stream so that we don't encounter a buffer underflow
     * while the reciever is muted.
     */
    void sleepOut() {
    }
    /**
     * start the output stream
     */
    void wakeOut() {
    }
        
    /**
     * stop the input stream so that we don't encounter a buffer overflow
     * while the transmitter is inactive.
     */
    virtual void sleepIn() { }
    /**
     * start the input stream
     */
    virtual void wakeIn() { }

    std::string currentPlaybackState() {
      return std::string("Fabulous");
    }

    virtual std::string currentCaptureState() {
      return std::string("NOT IMPLEMENTED");
    }

    /**
     * set the gain for the output device.
     * @param _gain -- range from 0 to 1.0
     * @return true if gain was set, false otherwise.
     */
    virtual bool setOutGain(float _gain) {
      gain = _gain;
      return true; 
    }


    /**
     * get the gain for the output device.
     * @return the gain; 
     */
    virtual float getOutGain() { return gain; }


    
  protected:
    /**
     * setup the network sockets for the audio link to the user interface.
     */
    void setupNetwork(std::string audio_sock_basename) ;
    
  private:
    SoDa::UD::ServerSocket * audio_rx_socket; 

    unsigned int sample_rate;
    unsigned int sample_count_hint;
    
    float gain; 
  };

}

