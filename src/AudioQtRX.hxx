#pragma once

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

#include "SoDaBase.hxx"
#include "AudioIfc.hxx"
#include "UDSockets.hxx"
#include "Params.hxx"
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
   * AudioQt implements the interface specified by AudioIfc.  It should
   * be interchangable with other audio interface handlers.  When BasebandRX
   * posts data to the audio interface, it is transmitted via a unix domain
   * socket to the GUI where the audio output device is managed. 
   *
   * For now, the transmit side is via ALSA.  That will change in time. 
   *
   * The first several generations of SoDa used a Port Audio interface, but
   * the PA library tends to spew a lot of extraneous "informational" messages
   * on the console.  Though it is quite simple, the noise that PA creates makes
   * it not my (kb1vc) interface of choice.
   *
   * ALSA on the other hand is documented poorly, organized oddly, and
   * nearly inscrutable. But it is fast, and doesn't make a lot of noise on the
   * console. 
   *
   * On the other hand, ALSA doesn't support asynchronous callbacks on the
   * audio output channel.  Because of this it really really stinks 
   * when it comes to dealing with real-time problems.  Yes there is a documented
   * "register a callback" function, but here's a surprise: it isn't supported 
   * in certain cases that are common for Fedora, Ubuntu, and others.  
   *
   * Qt, on the other hand, is well documented, nicely written, and 
   * works pretty much all the time.  
   * 
   */
  class AudioQtRX : public AudioIfc, public Debug {
  public:
    /**
     * constructor
     * @param params All settings are taken from the params object. 
     * @param name the name for this object/thread
     */
    AudioQtRX(Params_p params, const std::string & name = "AudioQtRX");

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
     * recv -- get a buffer of data from the audio input -- Always returns a buffer
     * of 0.0.  AudioQtRXTX over-rides this method. 
     * 
     * @param buf buffer of type described by the DataFormat selected at init
     * @param len number of elements in the buffer to get
     * @param when_ready if true, test with recvBufferReady and return 0 if not ready
     * otherwise perform the recv regardless.
     * @return number of elements transferred from the audio input
     */
    virtual int recv(void * buf, unsigned int len, bool when_ready = false) { 
      std::ignore = buf;
      std::ignore = when_ready;
      float *bp = (float*) buf;
      for(int i = 0; i < len; i++) { bp[i] = 0.0; }
      return len; 
    }

    /**
     * recvBufferReady -- are there samples waiting in the audio device?
     * Always returns true. 
     *                    
     * @param len the number of samples that we wish to get
     * @return true if len samples are waiting in in the device buffer
     */
    bool recvBufferReady(unsigned int len) { return true; }

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
    
  protected:
    /**
     * setup the network sockets for the audio link to the user interface.
     */
    void setupNetwork(std::string audio_sock_basename) ;
    
    

  private:

  private:
    SoDa::UD::ServerSocket * audio_rx_socket; 

    // debug assistance
    float ang; 
    float ang_incr; 
  };

}

