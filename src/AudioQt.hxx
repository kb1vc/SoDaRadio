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
#include "UDSockets.hxx"
#include <string>
#include <mutex>
// Only works if we have ALSA
#include <alsa/asoundlib.h>
#include <boost/format.hpp>
#include <iostream>
#include <stdexcept>

namespace SoDa {
  /**
   * @class AudioQt
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
  class AudioQt : public AudioIfc, public Debug {
  public:
    /**
     * constructor
     * @param _sample_rate in Hz  48000 is a good choice
     * @param _sample_count_hint  the size of the buffers passed to and from
     *                              the audio device (in samples)
     * @param audio_sock_basename starting string for the unix-domain socket 
     *                            that carries the audio stream from the SoDaServer (radio) process
     * @param audio_port_name  which ALSA device are we connecting to?
     */
    AudioQt(unsigned int _sample_rate,
	    unsigned int _sample_count_hint = 1024,
	    std::string audio_sock_basename = std::string("soda_"),
	    std::string audio_port_name = std::string("default"));

    ~AudioQt() {
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
    void sleepIn() {
      debugMsg("Sleep In");            
      std::lock_guard<std::mutex> mt_lock(alsa_mutex);
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
      std::lock_guard<std::mutex> mt_lock(alsa_mutex);      
      int err; 
      if((err = snd_pcm_prepare(pcm_in)) < 0) {
	throw
	  SoDaException((boost::format("AudioQt::wakeIn() Failed to wake after sleepIn() -- %s")
			 % snd_strerror(err)).str(), this);
      }
      if((err = snd_pcm_start(pcm_in)) < 0) {
	throw
	  SoDaException((boost::format("AudioQt::wakeIn() Failed to wake after sleepIn() -- %s")
			 % snd_strerror(err)).str(), this);
      }
    }
    std::string currentPlaybackState() {
      return std::string("Fabulous");
    }

    std::string currentCaptureState() {
      std::lock_guard<std::mutex> mt_lock(alsa_mutex);                  
      debugMsg("curCaptureState");                        
      return currentState(pcm_in);
    }
  protected:
    snd_pcm_t * pcm_in;  ///< The capture (input) handle. 
    snd_pcm_hw_params_t * hw_in_params;  ///< the input parameter list

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
     * setup the capture handle and features.
     */
    void setupCapture(std::string audio_port_name); 

    /**
     * setup the network sockets for the audio link to the user interface.
     */
    void setupNetwork(std::string audio_sock_basename) ;
    
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

    SoDa::UD::ServerSocket * audio_rx_socket; 

    // debug assistance
    float ang; 
    float ang_incr; 
  };

}


#endif
