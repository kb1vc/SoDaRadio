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

#include "Debug.hxx"
#include "AudioQtRXTX.hxx"

#include <alsa/asoundlib.h>

#include <SoDa/Format.hxx>

#define _USE_MATH_DEFINES
#include <cmath>

// Use the simple setup, as recent (March 2015) changes to the 
// ALSA/pulseaudio interactions have made explicit selection
// of parameters somewhat harder to figure out.  In particular, 
// buffer management has become far more important.
#define ALSA_USE_SIMPLE_SETUP

namespace SoDa {
  AudioQtRXTX::AudioQtRXTX(unsigned int _sample_rate,
			   unsigned int _sample_count_hint, 
			   std::string audio_sock_basename, 
			   std::string audio_port_name) :
    AudioQtRX(_sample_rate, _sample_count_hint, "AudioQtRXTX ALSA Interface") {

    std::cerr << "Creating AudioQtRXTX\n";    
    // code is largely borrowed from equalarea.com/paul/alsa-audio.html
    setupCapture(audio_port_name);
    
    setupNetwork(audio_sock_basename); 

    ang = 0.0; 
    ang_incr = 2.0 * M_PI / 48.0; 
  }

  void AudioQtRXTX::setupCapture(std::string audio_port_name)
  {
    // char pcm_cap_name[] = "default"; // "hw:0,2";
    const char *pcm_cap_name = audio_port_name.c_str();    
    snd_pcm_stream_t instream  = SND_PCM_STREAM_CAPTURE; 
    if(snd_pcm_open(&pcm_in, pcm_cap_name, instream, 0) < 0) {
      std::cerr << SoDa::Format("can't open Alsa PCM device [%0] for  input... Crap.\n")
	.addS(pcm_cap_name);
      exit(-1); 
    }

#ifdef ALSA_USE_SIMPLE_SETUP
    checkStatus(snd_pcm_set_params(pcm_in,
				   SND_PCM_FORMAT_FLOAT, 
				   SND_PCM_ACCESS_RW_INTERLEAVED, 
				   1,
				   sample_rate,
				   1, 
				   500000), // 100000),
		"Failed to do simple set params for output.", true); 
#else    
    setupParams(pcm_in, hw_in_params);
#endif    
  }

  void AudioQtRXTX::setupParams(snd_pcm_t * dev, snd_pcm_hw_params_t *  & hw_params_ptr)
  {
    snd_pcm_hw_params_t * hw_paramsp;
    std::lock_guard<std::mutex> mt_lock(alsa_mutex);
    
    checkStatus(snd_pcm_hw_params_malloc(&hw_paramsp), 
		"ALSA failed to allocate hardware params block", 
		true); 


    hw_params_ptr = hw_paramsp;

    checkStatus(snd_pcm_hw_params_any (dev, hw_paramsp), "ALSA failed in setupParams init parm block", true);
    
    checkStatus(snd_pcm_hw_params_set_access (dev, hw_paramsp, SND_PCM_ACCESS_RW_INTERLEAVED),
		"ALSA failed in setupParams set access", true);

    checkStatus(snd_pcm_hw_params_set_format (dev, hw_paramsp, SND_PCM_FORMAT_FLOAT),
		"ALSA failed in setupParams set format", true);
	
    checkStatus(snd_pcm_hw_params_set_rate_near (dev, hw_paramsp, &sample_rate, 0), 
		"ALSA failed in setupParams set sample rate", true);
	
    checkStatus(snd_pcm_hw_params_set_channels (dev, hw_paramsp, 1), 
		"setupParams set number of channels", true);

    checkStatus(snd_pcm_hw_params_set_buffer_size (dev, hw_paramsp,
						   sample_count_hint * datatype_size), 
		"ALSA failed in setupParams set buffer size", true);

    checkStatus(snd_pcm_hw_params (dev, hw_paramsp), 
		"ALSA failed in setupParams set parameter block", true);
	
    checkStatus(snd_pcm_prepare (dev), 
		"ALSA failed in setupParams prepare audio interface", true);
  }

  bool AudioQtRXTX::recvBufferReady(unsigned int len) {
    std::lock_guard<std::mutex> lock(alsa_mutex);
    return recvBufferReady_priv(len);
  }

  bool AudioQtRXTX::recvBufferReady_priv(unsigned int len)  {
    snd_pcm_sframes_t sframes_ready = snd_pcm_avail(pcm_in);

    if(sframes_ready == -EBADFD) {
      debugMsg("recvBufferReady got EBADFD, tried recovery");
      return false; 
    }
    else if(sframes_ready == -EPIPE) {
      // we got an under-run... just ignore it.
      int err; 
      std::cerr << "A";  // but put a goat dropping on the console
      if((err = snd_pcm_recover(pcm_in, sframes_ready, 1)) < 0) {
	checkStatus(err, "recvBufferReady got EPIPE, tried recovery", false);
      }
      sframes_ready = snd_pcm_avail(pcm_in);
    }
    
    if(sframes_ready < 0) {
      checkStatus(sframes_ready, "recvBufferReady", false);
    }

    return sframes_ready >= len; 
  }



  int AudioQtRXTX::recv(void * buf, unsigned int len, bool when_ready) {
    int err;
    int olen = len;
    int loopcount = 0; 
    {
      std::lock_guard<std::mutex> mt_lock(alsa_mutex);      

      if(when_ready && !recvBufferReady_priv(len)) return 0;

      char * cbuf = (char *) buf; 
      while(1) {
	loopcount++; 
	err = snd_pcm_readi(pcm_in, cbuf, len);

	if(err == (int)len) {
	  break; 
	}
	else if(err == -EAGAIN) continue; 
	else if(err < 0) {
	  checkStatus(err, "recv", true);
	}
	else if(err != (int)len) {
	  len -= err;
	  cbuf += (err * datatype_size);
	}
      }
    }
    return olen; 
  }

  /**
   * stop the input stream so that we don't encounter a buffer overflow
   * while the transmitter is inactive.
   */
  void AudioQtRXTX::sleepIn() {
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
  void AudioQtRXTX::wakeIn() {
    debugMsg("Wake In");                  
    std::lock_guard<std::mutex> mt_lock(alsa_mutex);      
    int err; 
    if((err = snd_pcm_prepare(pcm_in)) < 0) {
      throw
	SoDa::Radio::Exception(SoDa::Format("AudioQtRXTX::wakeIn() Failed to wake after sleepIn() -- %0")
			       .addS(snd_strerror(err)).str(), this);
    }
    if((err = snd_pcm_start(pcm_in)) < 0) {
      throw
	SoDa::Radio::Exception(SoDa::Format("AudioQtRXTX::wakeIn() Failed to wake after sleepIn() -- %0")
			       .addS(snd_strerror(err)).str(), this);
    }
  }

  std::string AudioQtRXTX::currentCaptureState() {
    std::lock_guard<std::mutex> mt_lock(alsa_mutex);                  
    debugMsg("curCaptureState");                        
    return currentState(pcm_in);
  }

  std::string AudioQtRXTX::currentState(snd_pcm_t * dev) {
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
}
