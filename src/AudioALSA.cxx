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
#include "AudioALSA.hxx"
#if HAVE_ASOUNDLIB
#include <alsa/asoundlib.h>
#endif

#include <boost/format.hpp>

// Use the simple setup, as recent (March 2015) changes to the 
// ALSA/pulseaudio interactions have made explicit selection
// of parameters somewhat harder to figure out.  In particular, 
// buffer management has become far more important.
#define ALSA_USE_SIMPLE_SETUP

namespace SoDa {
#if HAVE_LIBASOUND
  AudioALSA::AudioALSA(unsigned int _sample_rate,
		       DataFormat _fmt,
		       unsigned int _sample_count_hint, 
		       std::string audio_port_name) :
    AudioIfc(_sample_rate, _fmt, _sample_count_hint, "AudioALSA ALSA Interface") {

    // code is largely borrowed from equalarea.com/paul/alsa-audio.html
    setupPlayback(audio_port_name);

    setupCapture(audio_port_name);
  }

  void AudioALSA::setupPlayback(std::string audio_port_name)
  {
    (void) audio_port_name; 
    // setup the playback (output) stream
    char pcm_name[] =  "default"; 
    
    // const char *pcm_name = audio_port_name.c_str();
    if(snd_pcm_open(&pcm_out, pcm_name, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK) < 0) {
      std::cerr << boost::format("can't open Alsa PCM device [%s] for output ... Crap.\n") % pcm_name;
      exit(-1); 
    }

#ifdef ALSA_USE_SIMPLE_SETUP    
    checkStatus(snd_pcm_set_params(pcm_out,
				   SND_PCM_FORMAT_FLOAT, 
				   SND_PCM_ACCESS_RW_INTERLEAVED, 
				   1,
				   sample_rate,
				   1, 
				   500000), // 100000),
		"Failed to do simple set params for output.", true); 
#else   
    setupParams(pcm_out, hw_out_params);
#endif
  }

  void AudioALSA::setupCapture(std::string audio_port_name)
  {
    // char pcm_cap_name[] = "default"; // "hw:0,2";
    const char *pcm_cap_name = audio_port_name.c_str();    
    snd_pcm_stream_t instream  = SND_PCM_STREAM_CAPTURE; 
    if(snd_pcm_open(&pcm_in, pcm_cap_name, instream, 0) < 0) {
      std::cerr << boost::format("can't open Alsa PCM device [%s] for  input... Crap.\n") % pcm_cap_name;
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

  void AudioALSA::setupParams(snd_pcm_t * dev, snd_pcm_hw_params_t *  & hw_params_ptr)
  {
    snd_pcm_hw_params_t * hw_paramsp;
    boost::mutex::scoped_lock mt_lock(alsa_lock);
    
    checkStatus(snd_pcm_hw_params_malloc(&hw_paramsp), 
		"ALSA failed to allocate hardware params block", 
		true); 


    hw_params_ptr = hw_paramsp;

    checkStatus(snd_pcm_hw_params_any (dev, hw_paramsp), "ALSA failed in setupParams init parm block", true);
    
    checkStatus(snd_pcm_hw_params_set_access (dev, hw_paramsp, SND_PCM_ACCESS_RW_INTERLEAVED),
		"ALSA failed in setupParams set access", true);

    checkStatus(snd_pcm_hw_params_set_format (dev, hw_paramsp, translateFormat(format)),
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

  bool AudioALSA::recvBufferReady(unsigned int len) {
      boost::mutex::scoped_lock lock(alsa_lock);
      return recvBufferReady_priv(len);
  }

  bool AudioALSA::recvBufferReady_priv(unsigned int len)  {
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


  bool AudioALSA::sendBufferReady(unsigned int len)  {
    boost::mutex::scoped_lock lock(alsa_lock);
    return sendBufferReady_priv(len);
  }


  bool AudioALSA::sendBufferReady_priv(unsigned int len)  {

    snd_pcm_sframes_t sframes_ready;
    while(1) {
      sframes_ready= snd_pcm_avail(pcm_out);
    
      if(sframes_ready == -EPIPE) {
	// we got an under-run... we can't just ignore it.
	// if pcm_avail returns -EPIPE we need to recover and restart the pipe... sigh.
	int err; 
	if((err = snd_pcm_recover(pcm_out, sframes_ready, 1)) < 0) {
	  checkStatus(err, "sendBufferReady got EPIPE, tried recovery", false);

	  if((err = snd_pcm_start(pcm_out)) < 0) {
	  throw
	    SoDaException((boost::format("AudioALSA::sendBufferReady() Failed to wake after sleepOut() -- %s")
			   % snd_strerror(err)).str(), this);
	  }
	}
      }
      else {
	checkStatus(sframes_ready, "sendBufferReady", false);
	break;
      }
    }
    return sframes_ready >= len; 
  }

  int AudioALSA::send(void * buf, unsigned int len, bool when_ready) {
    int err;
    int olen = len;
    boost::mutex::scoped_lock mt_lock(alsa_lock);      

    if(when_ready && !sendBufferReady_priv(len)) return 0; 

    char * cbuf = (char *) buf; 
    int cont_count = 0; 
    while(1) {
      err = snd_pcm_writei(pcm_out, cbuf, len);
      
      if(err == (int) len) return len; 
      else if((err == -EAGAIN) || (err == 0)) {
	cont_count++; 
	if(cont_count >= 10) {
	  cont_count = 0; 
	}
	continue;
      }
      else if(err == -EPIPE) {
	// we got an under-run... just ignore it.
	if((err = snd_pcm_recover(pcm_out, err, 1)) < 0) {
	  checkStatus(err, "send got EPIPE, tried recovery", false);
	}
      }
      else if(err < 0) {
	checkStatus(err, "send", true);
      }
      else if(err != (int)len) {
	len -= err;
	cbuf += (err * datatype_size);
      }
    }
   
    return olen; 
  }

  int AudioALSA::recv(void * buf, unsigned int len, bool when_ready) {
    int err;
    int olen = len;
    int loopcount = 0; 
    {
      boost::mutex::scoped_lock mt_lock(alsa_lock);      

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

  
  snd_pcm_format_t AudioALSA::translateFormat(AudioIfc::DataFormat fmt) {
    switch(fmt) {
    case FLOAT: return SND_PCM_FORMAT_FLOAT;
      break; 
    case DFLOAT: return SND_PCM_FORMAT_FLOAT64;
      break;
    case INT32: return SND_PCM_FORMAT_S32;
      break; 
    case INT16: return SND_PCM_FORMAT_S16;
      break; 
    case INT8: return SND_PCM_FORMAT_S8;
      break; 
    }
    return SND_PCM_FORMAT_S16_LE; 
  }
#else 
  AudioALSA::AudioALSA(unsigned int _sample_rate,
		       DataFormat _fmt,
		       unsigned int _sample_count_hint,
		       std::string audio_port_name) :
    AudioIfc(_sample_rate, _fmt, _sample_count_hint, "AudioALSA ALSA Interface")
  {
    std::cerr << "ALSA Sound Library is not enabled in this build version.";  
    throw SoDa::SoDaException("ALSA Sound Library is not enabled in this build version.");  
  }
#endif // HAVE_LIBASOUND
}
