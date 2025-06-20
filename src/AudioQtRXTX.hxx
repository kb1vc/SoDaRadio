#pragma once

/*
  Copyright (c) 2020, 2025 Matthew H. Reilly (kb1vc)
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
#include <string>
#include <memory>

#include <mutex>
// Only works if we have ALSA
#include <alsa/asoundlib.h>
#include <SoDa/Format.hxx>
#include <iostream>
#include <stdexcept>
// we implement the TX side of things. 
#include "AudioQtRX.hxx"

namespace SoDa {
  /**
   * @class AudioQtRXTX
   *
   * @brief Qt audio interface class for transmit via ALSA, inherits 
   * from AudioQtRX for the Qt receive path. 
   *
   * AudioQtRXTX implements the interface specified by AudioIfc.  It should
   * be interchangable with other audio interface handlers.  When BasebandRX
   * posts data to the audio interface, it is transmitted via a unix domain
   * socket to the GUI where the audio output device is managed. 
   *
   * For now, the transmit side is via ALSA.  That will change in time. 
   *
   * 
   */
  class AudioQtRXTX;
  typedef std::shared_ptr<AudioQtRXTX> AudioQtRXTXPtr;
  
  class AudioQtRXTX : public AudioQtRX {
  protected:
    /**
     * constructor
     * @param _sample_rate in Hz  48000 is a good choice
     * @param _sample_count_hint  the size of the buffers passed to and from
     *                              the audio device (in samples)
     * @param audio_sock_basename starting string for the unix-domain socket 
     *                            that carries the audio stream from the SoDaServer (radio) process
     * @param audio_port_name  which ALSA device are we connecting to?
     */
    AudioQtRXTX(unsigned int _sample_rate,
	    unsigned int _sample_count_hint = 1024,
	    std::string audio_sock_basename = std::string("soda_"),
	    std::string audio_port_name = std::string("default"));

  public:
    static AudioQtRXTXPtr make(unsigned int _sample_rate,
			       unsigned int _sample_count_hint = 1024,
			       std::string audio_sock_basename = std::string("soda_"),
			       std::string audio_port_name = std::string("default")) {
      auto ret = std::shared_ptr<AudioQtRXTX>(new AudioQtRXTX(_sample_rate,
							      _sample_count_hint,
							      audio_sock_basename,
							      audio_port_name));
      ret->registerSelf(ret);
      return ret; 
    }
  
    ~AudioQtRXTX() {
    }

    
    /**
     * recv -- get a buffer of data from the audio input
     * @param buf buffer of type described by the DataFormat selected at init
     * @param len number of elements in the buffer to get
     * @param when_ready if true, test with sendBufferReady and return 0 if not ready
     * otherwise perform the recv regardless.
     * @return number of elements transferred from the audio input
     */
    int recv(void * buf, unsigned int len, bool when_ready = false);

    /**
     * recvBufferReady -- are there samples waiting in the audio device?
     *                    
     * @param len the number of samples that we wish to get
     * @return true if len samples are waiting in in the device buffer
     */
    bool recvBufferReady(unsigned int len);

    /**
     * stop the input stream so that we don't encounter a buffer overflow
     * while the transmitter is inactive.
     */
    void sleepIn();

    /**
     * start the input stream
     */
    void wakeIn();

    std::string currentCaptureState();

  protected:
    snd_pcm_t * pcm_in;  ///< The capture (input) handle. 
    snd_pcm_hw_params_t * hw_in_params;  ///< the input parameter list

    /**
     *
     */
    std::string currentState(snd_pcm_t * dev);
    /**
     * setup the capture handle and features.
     */
    void setupCapture(std::string audio_port_name); 

    /**
     * setup the parameters for a PCM device
     * @param dev the device handle
     * @param hw_params (out parameter) a pointer to a device parameter block
     */
    void setupParams(snd_pcm_t * dev, snd_pcm_hw_params_t * & hw_params);
    
    /**
     * checkStatus check to see if the return status from an alsa call was OK
     * @param err -- the error number
     * @param exp -- why are we here
     * @param fatal -- if true, throw an exception, otherwise print an error to std::cerr
     */
    void checkStatus(int err, const std::string & exp, bool fatal = false) {
      if (err < 0) {
	if(fatal) throw SoDa::Radio::Exception(SoDa::Format("%0 %1")
					       .addS(exp)
					       .addS(snd_strerror(err)), getSelfPtr());
	else std::cerr << SoDa::Format("%0 %1 %2\n")
	       .addS(getObjName())
	       .addS(exp)
	       .addS(snd_strerror(err));
      }
    }

  private:
    /**
     * recvBufferReady_priv -- are there samples waiting in the audio device?
     *                    
     * actual implementation of ready check, but without protecting mutex. 
     *
     * @param len the number of samples that we wish to get
     * @return true if len samples are waiting in in the device buffer
     */
    bool recvBufferReady_priv(unsigned int len);

  private:
    std::mutex alsa_mutex;

    // debug assistance
    float ang; 
    float ang_incr; 
  };

}

