#pragma once

/*
  Copyright (c) 2020,2024 Matthew H. Reilly (kb1vc)
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

#include "SoDaThread.hxx"
#include "UDSockets.hxx"
#include <string>
#include <mutex>
#include <SoDa/Format.hxx>
#include <iostream>
#include <stdexcept>
// we implement the TX side of things. 
#include "AudioQtRX.hxx"

namespace SoDa {
  /**
   * @class AudioQtTX
   *
   * @brief Qt audio interface that monitors the socket from the GUI, where the
   * audio processing lives.
   *
   * When BasebandTX needs data from the audio interface, it is taken from
   * a buffer received from the GUI on the ..._txa socket.
   *
   * 
   */
  class AudioQtTX : public SoDa::Thread {
  public:
    /**
     * constructor
     * @param _sample_rate in Hz  48000 is a good choice
     * @param _audio_buffer_size  the size of the buffers passed to and from
     *                              the audio device (in samples)
     * @param audio_sock_basename starting string for the unix-domain socket 
     *                            that carries the audio stream from the SoDaServer (radio) process
     */
    AudioQtTX(unsigned int _sample_rate,
	      unsigned int _audio_buffer_size, 
	      std::string audio_sock_basename = std::string("soda_"));

    ~AudioQtTX() {
    }

    /// implement the subscription method
    void subscribeToMailBoxList(CmdMailBoxMap & cmd_boxes,
				DatMailBoxMap & dat_boxes);
    
    /**
     * @brief get a buffer of data from the audio input
     * @param buf buffer of type described by the DataFormat selected at init
     * @param len number of elements in the buffer to get
     * @param when_ready if true, test with sendBufferReady and return 0 if not ready
     * otherwise perform the recv regardless.
     * @return number of elements transferred from the audio input
     */
    int getBuffer(FVecPtr & buf, unsigned int len, bool when_ready = false);

    /**
     * bufferReady -- are there samples waiting in the audio device?
     *                    
     * @return true if  samples are waiting in in the device buffer
     */
    bool bufferReady();

    /**
     * @brief run method -- looks for messages on the tx audio socket
     */
    void run();


    /**
     * @brief empty all waiting buffers from the buffer list
     */
    void clearBufferList();
    
    /**
     * @brief set method -- monitor for TX on or TX off
     *
     * @param cmd the SoDaRadio command packet
     */
    void execSetCommand(CommandPtr cmd); 

    /**
     * get the gain for the input device.
     * @return input gain
     */
    float getGain() { return gain; }

    /**
     * set the gain for the input device.
     * @param _gain -- range from 0 to 1.0
     * @return true if gain was set, false otherwise.
     */
    bool setGain(float _gain) {
      gain = _gain;
      return true; 
    }
    
  protected:
    
    void setupCurrentBuf();
    
    SoDa::UD::ServerSocketPtr audio_tx_socket;
    
    unsigned int audio_buffer_size;
    unsigned int sample_rate; 
    
    CmdMBoxPtr cmd_stream;
    
    bool tx_on; 
    bool cw_mode; 
    float gain;

    std::queue<FVecPtr> buffer_list;
    
    FVecPtr current_buf;
    char * current_buf_end_ptr;
    unsigned int current_buf_bytes_left; 
    

    std::mutex buf_list_mutex; 
  };

}

