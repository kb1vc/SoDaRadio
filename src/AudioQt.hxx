#pragma once

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

#include "SoDaBase.hxx"
#include "SoDaThread.hxx"
#include "AudioIfc.hxx"
#include "UDSockets.hxx"
#include <string>
#include <mutex>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <tuple>

namespace SoDa {

  class AudioQt;
  typedef std::shared_ptr<AudioQt> AudioQtPtr;
  typedef std::shared_ptr<std::vector<float>> FloatVecPtr;
  
  /**
   * @class AudioQt
   *
   * @brief Qt audio interface class
   *
   * AudioQt implements the interface specified by AudioIfc.  It should
   * be interchangable with other audio interface handlers.
   *
   * When BasebandRX posts data to the audio interface, it is
   * transmitted via a unix domain socket to the GUI where the audio
   * output device is managed. In the standard SoDaRadio configuration,
   * this is turned into a QtAudio output stream.
   *
   * BasebandTX watches for incoming data on its audio input mailbox. This
   * arrives from the GUI (a QtAudio stream) via a unix domain socket.
   *
   * The AudioQt object runs as a thread, solely to listen in on the
   * incoming (transmit) audio socket. As samples arrive, they are
   * appended to a dequeue of buffers to be passed back to the
   * BasebandTX object when it needs data. The listener thread
   * also subscribes to the cmd mailbox ring: it throws away
   * incoming and buffered samples while the radio is in TX
   * mode. This allows the process sourcing the audio stream to
   * be ignorant of the current state of the transceiver. 
   * 
   */
  class AudioQt : public AudioIfc, public Thread {
  protected:
    /**
     * constructor
     * @param _sample_rate in Hz  48000 is a good choice
     * @param _sample_count_hint  the size of the buffers passed to and from
     *                              the audio device (in samples)
     * @param audio_sock_basename starting string for the unix-domain sockets
     *                            that carry the RX audio stream from the SoDaServer (radio) process
     *                            and the TX audio stream from the Qt GUI.
     */
    AudioQt(unsigned int _sample_rate,
	    unsigned int _sample_count_hint = 1024,
	    std::string audio_sock_basename = std::string("soda_"));

  public:
    static AudioQtPtr make(unsigned int _sample_rate,
			   unsigned int _sample_count_hint = 1024,
			   std::string audio_sock_basename = std::string("soda_")) {
      auto ret = std::shared_ptr<AudioQt>(new AudioQt(_sample_rate,
						      _sample_count_hint,
						      audio_sock_basename));
      ret->registerThread(ret);
      return ret; 
    }
    
    ~AudioQt() {
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
     * recv -- get a buffer of data from the audio input
     * 
     * @param buf float vector of samples coming from the audio input device
     * @param when_ready if true, test with recvBufferReady and return 0 if not ready
     * otherwise perform the recv regardless.
     * @return number of elements transferred from the audio input
     */
    virtual int recv(std::vector<float> & buf, bool when_ready = false);

    /**
     * recvBufferReady -- are there samples waiting in the audio device?
     *                    
     * @param len the number of samples that we wish to get
     * @return true if len samples are waiting in in the device buffer
     */
    bool recvBufferReady(unsigned int len);

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
     * while the transmitter is inactive. This will also empty the incoming
     * transmit buffer. 
     */
    virtual void sleepIn();
    /**
     * start the input stream
     */
    virtual void wakeIn();

    std::string currentPlaybackState() {
      return std::string("Fabulous");
    }

    virtual std::string currentCaptureState() {
      return std::string("NOT IMPLEMENTED");
    }
    /**
     * @brief the run method -- maintains incoming audio sample buffers
     */
    void run();

    void subscribeToMailBoxes(const std::vector<MailBoxBasePtr> & mailboxes);
    
  protected:
    /**
     * setup the network sockets for the audio link to the user interface.
     */
    void setupNetwork(std::string audio_sock_basename) ;

    /**
     * @brief create buffers to hold incoming samples. Put them in the free pool
     */ 
    void setupBuffers(unsigned int bsize);

    /**
     * @brief create a new buffer to be added to the free pool
     *
     * @param bsize number of samples in the buffer
     * @return a pointer to a float vector. 
     */
    FloatVecPtr makeBuffer(unsigned int bsize);
    
    /**
     * @brief gets a buffer from the free pool
     */
    FloatVecPtr getFreeBuffer();
    
  private:
    std::shared_ptr<SoDa::UD::ServerSocket> audio_rx_socket;
    std::shared_ptr<SoDa::UD::ServerSocket> audio_tx_socket;

    // the TX process monitors the command bus for TX on/off. 
    CmdMBoxPtr cmd_stream; ///< command stream from UI and other units
    CmdMBox::Subscription cmd_subs; ///< subscription ID for command stream

    bool ignore_tx_data;
    
    // the transmit audio buffer ring
    const int initial_pool_size = 42; 
    std::queue<FloatVecPtr> incoming_audio_bufs;
    std::queue<FloatVecPtr> free_audio_bufs;
    // the mutex for managing it
    std::mutex tx_data_mutex; 

    // debug assistance
    float ang; 
    float ang_incr;

  };
}

