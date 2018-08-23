#ifndef ALSA_PCM_HDR
#define ALSA_PCM_HDR
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
#include <string>
#include <alsa/asoundlib.h>
#include <boost/format.hpp>
#include <boost/thread/mutex.hpp>
#include <iostream>
#include <stdexcept>

namespace SoDa {
  /**
   * @class AudioALSA
   *
   * @brief ALSA audio interface class
   *
   * AudioALSA implements the interface specified by AudioIfc.  It should
   * be interchangable with other audio interface handlers.
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
   */
  class AudioALSA : public AudioIfc, public Debug {
  public:
    /**
     * constructor
     * @param _sample_rate in Hz  48000 is a good choice
     * @param _sample_count_hint  the size of the buffers passed to and from
     *                              the audio device (in samples)
     * @param audio_port_name  which ALSA device are we connecting to?
     */
    AudioALSA(unsigned int _sample_rate,
	      unsigned int _sample_count_hint = 1024,
	      std::string audio_port_name = std::string("default"));

    ~AudioALSA() {
      boost::mutex::scoped_lock lock(alsa_lock);          
      snd_pcm_close(pcm_out);
    }


    /**
     * @brief buffer allocator. 
     * @param audio_buffer_size number of elements in the audio buffer (duh.)
     * @return a buffer
     */
    float * getBuffer(unsigned int audio_buffer_size);

    /** 
     * @brief flush outstanding RX buffers (we're about to switch to TX...)
     *
     */
    void flushRXBuffers();
    
    /**
     * send -- send a buffer to the audio output
     * @param buf buffer of type described by the DataFormat selected at init
     * @param len number of elements in the buffer to send
     * @return number of elements transferred to the audio output
     */
    int send(void * buf, unsigned int len)  ;

    /**
     * recv -- get a buffer of data from the audio input
     * @param buf buffer of type described by the DataFormat selected at init
     * @param len number of elements in the buffer to get
     * @param when_ready if true, test with sendBufferReady and return 0 if not ready
     * otherwise perform the recv regardless.
     * @return number of elements transferred from the audio input
     */
    int recv(void * buf, unsigned int len, bool when_ready = false)  ; 

    /**
     * recvBufferReady -- are there samples waiting in the audio device?
     *                    
     * @param len the number of samples that we wish to get
     * @return true if len samples are waiting in in the device buffer
     */
    bool recvBufferReady(unsigned int len)  ;

    /**
     * stop the output stream so that we don't encounter a buffer underflow
     * while the reciever is muted.
     */
    void sleepOut() {
      debugMsg("Sleep Out");      
      int err; 
      {
	boost::mutex::scoped_lock mt_lock(alsa_lock);
	err = snd_pcm_drain(pcm_out);
      }
      if(err != 0) {
	std::cerr << boost::format("snd_pcm_drain returned %d\n") % err; 
      }
    }
    /**
     * start the output stream
     */
    void wakeOut() {
      debugMsg("Wake Out");
      boost::mutex::scoped_lock mt_lock(alsa_lock);      
      int err; 
      if((err = snd_pcm_prepare(pcm_out)) < 0) {
	throw
	  SoDaException((boost::format("AudioALSA::wakeOut() Failed to wake after sleepOut() pcm_prepare -- %s")
			 % snd_strerror(err)).str(), this);
      }
      if((err = snd_pcm_start(pcm_out)) < 0) {
	throw
	  SoDaException((boost::format("AudioALSA::wakeOut() Failed to wake after sleepOut() pcm_start -- %s")
			 % snd_strerror(err)).str(), this);
      }
    }
        
    /**
     * stop the input stream so that we don't encounter a buffer overflow
     * while the transmitter is inactive.
     */
    void sleepIn() {
      debugMsg("Sleep In");            
      boost::mutex::scoped_lock mt_lock(alsa_lock);
      snd_pcm_drop(pcm_in);

      // now read the input buffers until they're empty
      int buf[1000];      
      int len = 1000; 
      int stat = 1;
      while(stat > 0) {
	stat = snd_pcm_readi(pcm_in, buf, len);
	std::cerr << "-@-";
	if(stat == 0) break; 
	else if(stat == -EAGAIN) continue; 
	else break; 
      }
    }

    /**
     * start the input stream
     */
    void wakeIn() {
      debugMsg("Wake In");                  
      boost::mutex::scoped_lock mt_lock(alsa_lock);      
      int err; 
      if((err = snd_pcm_prepare(pcm_in)) < 0) {
	throw
	  SoDaException((boost::format("AudioALSA::wakeIn() Failed to wake after sleepIn() -- %s")
			 % snd_strerror(err)).str(), this);
      }
      if((err = snd_pcm_start(pcm_in)) < 0) {
	throw
	  SoDaException((boost::format("AudioALSA::wakeIn() Failed to wake after sleepIn() -- %s")
			 % snd_strerror(err)).str(), this);
      }
    }

  protected:
    snd_pcm_t * pcm_out; ///< The playback (output) handle. 
    snd_pcm_t * pcm_in;  ///< The capture (input) handle. 
    snd_pcm_hw_params_t * hw_in_params;  ///< the input parameter list
    snd_pcm_hw_params_t * hw_out_params; ///< the output parameter list

    /**
     * setup the playback handle and features. 
     */
    void setupPlayback(std::string audio_port_name);

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
	if(fatal) throw SoDaException((boost::format("%s %s") % exp % snd_strerror(err)).str(), this);
	else std::cerr << boost::format("%s %s %s\n") % getObjName() % exp % snd_strerror(err);
      }
    }

  private:
    snd_async_handler_t * pcm_send_callback; ///< parameter block for audio out callbacks

    /**
     * @brief callback from alsa output channel -- object method
     * @param pcm_callback all the dope on why we are here. 
     */
    void handleOutReady(snd_async_handler_t * pcm_callback);
    
    /** 
     * @brief callback from alsa output channel -- class method
     * @param pcm_callback all the dope on why we are here. 
     */
    static void audioOutCallback(snd_async_handler_t * pcm_callback) {
      AudioALSA * obj;
      obj = static_cast<AudioALSA *> snd_async_handler_get_callback_private(pcm_callback);
      obj->handleOutReady(pcm_callback); 
    } 

    /**
     * recvBufferReady_priv -- are there samples waiting in the audio device?
     *                    
     * actual implementation of ready check, but without protecting mutex. 
     *
     * @param len the number of samples that we wish to get
     * @return true if len samples are waiting in in the device buffer
     */
    bool recvBufferReady_priv(unsigned int len)  ;    

    /**
     * sendBufferReady_priv -- is there enough space in the audio device
     *                    send buffer for a call from send?
     * actual implementation, but without protecting mutex.
     *
     * @param len the number of samples that we wish to send
     * @return true if there is sufficient space. 
     */
    bool sendBufferReady_priv(unsigned int len)  ;

  private:
    boost::mutex alsa_lock;
    
  };

}


#endif
