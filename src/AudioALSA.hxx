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
#if HAVE_LIBASOUND
#  include <alsa/asoundlib.h>
#  define ALSA_DEF
#else
#  define ALSA_DEF { throw SoDa::SoDaException("ALSA Sound Library is not enabled in this build version."); } 
#endif
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
     * @param _fmt  the format of the data (FLOAT, DFLOAT, INT32, INT16, INT8)
     * @param _sample_count_hint  the size of the buffers passed to and from
     *                              the audio device (in samples)
     * @param audio_port_name  which ALSA device are we connecting to?
     */
    AudioALSA(unsigned int _sample_rate,
	      AudioIfc::DataFormat _fmt,
	      unsigned int _sample_count_hint = 1024,
	      std::string audio_port_name = std::string("default"));

    ~AudioALSA() {
      boost::mutex::scoped_lock lock(alsa_lock);          
#if HAVE_LIBASOUND
      snd_pcm_close(pcm_out);
#endif      
    }
    
    /**
     * send -- send a buffer to the audio output
     * @param buf buffer of type described by the DataFormat selected at init
     * @param len number of elements in the buffer to send
     * @param when_ready if true, test with sendBufferReady and return 0 if not ready
     * otherwise perform the send regardless.
     * @return number of elements transferred to the audio output
     */
    int send(void * buf, unsigned int len, bool when_ready = false) ALSA_DEF ;

    /**
     * sendBufferReady -- is there enough space in the audio device
     *                    send buffer for a call from send?
     * @param len the number of samples that we wish to send
     * @return true if there is sufficient space. 
     */
    bool sendBufferReady(unsigned int len) ALSA_DEF ;

    /**
     * recv -- get a buffer of data from the audio input
     * @param buf buffer of type described by the DataFormat selected at init
     * @param len number of elements in the buffer to get
     * @param when_ready if true, test with sendBufferReady and return 0 if not ready
     * otherwise perform the recv regardless.
     * @return number of elements transferred from the audio input
     */
    int recv(void * buf, unsigned int len, bool when_ready = false) ALSA_DEF ; 

    /**
     * recvBufferReady -- are there samples waiting in the audio device?
     *                    
     * @param len the number of samples that we wish to get
     * @return true if len samples are waiting in in the device buffer
     */
    bool recvBufferReady(unsigned int len) ALSA_DEF ;

    /**
     * stop the output stream so that we don't encounter a buffer underflow
     * while the reciever is muted.
     */
    void sleepOut() {
      debugMsg("Sleep Out");      
#if HAVE_LIBASOUND
      int err; 
      {
	boost::mutex::scoped_lock mt_lock(alsa_lock);
	err = snd_pcm_drain(pcm_out);
      }
      if(err != 0) {
	std::cerr << boost::format("snd_pcm_drain returned %d\n") % err; 
      }
#endif
    }
    /**
     * start the output stream
     */
    void wakeOut() {
      debugMsg("Wake Out");
#if HAVE_LIBASOUND
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
#endif
    }
        
    /**
     * stop the input stream so that we don't encounter a buffer overflow
     * while the transmitter is inactive.
     */
    void sleepIn() {
      debugMsg("Sleep In");            
#if HAVE_LIBASOUND
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
#endif
    }

    /**
     * start the input stream
     */
    void wakeIn() {
      debugMsg("Wake In");                  
#if HAVE_LIBASOUND
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
#endif
    }
#if HAVE_LIBASOUND
    std::string currentPlaybackState() {
      boost::mutex::scoped_lock mt_lock(alsa_lock);            
      debugMsg("curPlaybackState");                  
      std::string cs; 
      cs = currentState(pcm_out);      
      return (boost::format("%s  ready_frames = %d") % cs % snd_pcm_avail(pcm_out)).str();
    }

    std::string currentCaptureState() {
      boost::mutex::scoped_lock mt_lock(alsa_lock);                  
      debugMsg("curCaptureState");                        
      return currentState(pcm_in);
    }
#endif
  protected:
#if HAVE_LIBASOUND    
    snd_pcm_t * pcm_out; ///< The playback (output) handle. 
    snd_pcm_t * pcm_in;  ///< The capture (input) handle. 
    snd_pcm_hw_params_t * hw_in_params;  ///< the input parameter list
    snd_pcm_hw_params_t * hw_out_params; ///< the output parameter list

    /**
     *
     */
    std::string currentState(snd_pcm_t * dev) {
      snd_pcm_state_t st;
      st = snd_pcm_state(dev);

      switch (st) {
      case SND_PCM_STATE_OPEN:
	return std::string("SND_PCM_STATE_OPEN");
	break;
      case SND_PCM_STATE_SETUP:
	return std::string("SND_PCM_STATE_SETUP");
	break;
      case SND_PCM_STATE_PREPARED:
	return std::string("SND_PCM_STATE_PREPARED");
	break;
      case SND_PCM_STATE_RUNNING:
	return std::string("SND_PCM_STATE_RUNNING");
	break;
      case SND_PCM_STATE_XRUN:
	return std::string("SND_PCM_STATE_XRUN");
	break;
      case SND_PCM_STATE_DRAINING:
	return std::string("SND_PCM_STATE_DRAINING");
	break;
      case SND_PCM_STATE_PAUSED:
	return std::string("SND_PCM_STATE_PAUSED");
	break;
      case SND_PCM_STATE_SUSPENDED:
	return std::string("SND_PCM_STATE_SUSPENDED");
	break;
      case SND_PCM_STATE_DISCONNECTED:
	return std::string("SND_PCM_STATE_DISCONNECTED");
	break;
      default:
	return std::string("BADSTATE-UNKNOWN");
      }
    }

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
     * ALSA has predefined data type codes corresponding to float/ints of various sizes.
     * @param fmt the AudioIfc::DataFormat spec (FLOAT, DFLOAT, INT32, INT16, INT8)
     * @return a format specifier from the ALSA PCM format list.
     */
    snd_pcm_format_t translateFormat(AudioIfc::DataFormat fmt);

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
    /**
     * recvBufferReady_priv -- are there samples waiting in the audio device?
     *                    
     * actual implementation of ready check, but without protecting mutex. 
     *
     * @param len the number of samples that we wish to get
     * @return true if len samples are waiting in in the device buffer
     */
    bool recvBufferReady_priv(unsigned int len) ALSA_DEF ;    

    /**
     * sendBufferReady_priv -- is there enough space in the audio device
     *                    send buffer for a call from send?
     * actual implementation, but without protecting mutex.
     *
     * @param len the number of samples that we wish to send
     * @return true if there is sufficient space. 
     */
    bool sendBufferReady_priv(unsigned int len) ALSA_DEF ;

#endif // HAVE_LIBASOUND

  private:
    boost::mutex alsa_lock;
    
  };

}


#endif
