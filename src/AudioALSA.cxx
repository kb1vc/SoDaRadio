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

#include "AudioALSA.hxx"
#if HAVE_ASOUNDLIB
#include <alsa/asoundlib.h>
#endif

#include <boost/format.hpp>

namespace SoDa {
#if HAVE_LIBASOUND
  AudioALSA::AudioALSA(unsigned int _sample_rate,
		       DataFormat _fmt,
		       unsigned int _sample_count_hint) :
    AudioIfc(_sample_rate, _fmt, _sample_count_hint, "AudioALSA ALSA Interface") {

    // code is largely borrowed from equalarea.com/paul/alsa-audio.html
    setupPlayback();

    setupCapture();
  }

  void AudioALSA::setupPlayback()
  {
    // setup the playback (output) stream
    char pcm_name[] = "default"; 
    if(snd_pcm_open(&pcm_out, pcm_name, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK) < 0) {
      std::cerr << boost::format("can't open Alsa PCM device for output ... Crap.\n");
      exit(-1); 
    }

    checkStatus(snd_pcm_set_params(pcm_out,
				   SND_PCM_FORMAT_FLOAT, 
				   SND_PCM_ACCESS_RW_INTERLEAVED, 
				   1,
				   sample_rate,
				   1, 
				   100000),
		"Failed to do simple set params for output.", true); 
#if 0    
    setupParams(pcm_out, hw_out_params);
#endif
  }

  void AudioALSA::setupCapture()
  {
    char pcm_cap_name[] = "default"; // "hw:0,2";
    snd_pcm_stream_t instream  = SND_PCM_STREAM_CAPTURE; 
    if(snd_pcm_open(&pcm_in, pcm_cap_name, instream, 0) < 0) {
      std::cerr << boost::format("can't open Alsa PCM device for  input... Crap.\n");
      exit(-1); 
    }

    setupParams(pcm_in, hw_in_params);
  }

  void AudioALSA::setupParams(snd_pcm_t * dev, snd_pcm_hw_params_t *  & hw_params_ptr)
  {
    int err;
    snd_pcm_hw_params_t * hw_paramsp;
    
    snd_pcm_hw_params_alloca(&hw_paramsp);

    hw_params_ptr = hw_paramsp;

    checkStatus(snd_pcm_hw_params_any (dev, hw_paramsp), "setupParams init parm block", true);
    
    checkStatus(snd_pcm_hw_params_set_access (dev, hw_paramsp, SND_PCM_ACCESS_RW_INTERLEAVED),
		"setupParams set access", true);

    checkStatus(snd_pcm_hw_params_set_format (dev, hw_paramsp, translateFormat(format)),
		"setupParams set format", true);
	
    checkStatus(snd_pcm_hw_params_set_rate_near (dev, hw_paramsp, &sample_rate, 0), 
		"setupParams set sample rate", true);
	
    checkStatus(snd_pcm_hw_params_set_channels (dev, hw_paramsp, 1), 
		"setupParams set number of channels", true);

    checkStatus(snd_pcm_hw_params_set_buffer_size (dev, hw_paramsp,
						   sample_count_hint * datatype_size), 
		"setupParams set buffer size", true);

    checkStatus(snd_pcm_hw_params (dev, hw_paramsp), 
		"setupParams set parameter block", true);
	
    checkStatus(snd_pcm_prepare (dev), 
		"setupParams prepare audio interface", true);
  }


  bool AudioALSA::recvBufferReady(unsigned int len)  {
    snd_pcm_sframes_t sframes_ready = snd_pcm_avail(pcm_in);
    if(sframes_ready == -EPIPE) {
      // we got an under-run... just ignore it.
      int err; 
      if(err = snd_pcm_recover(pcm_in, sframes_ready, 1) < 0) {
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
    snd_pcm_sframes_t sframes_ready = snd_pcm_avail(pcm_out);
    if(sframes_ready == -EPIPE) {
      // we got an under-run... just ignore it.
      int err; 
      if(err = snd_pcm_recover(pcm_out, sframes_ready, 1) < 0) {
	checkStatus(err, "sendBufferReady got EPIPE, tried recovery", false);
      }
      sframes_ready = snd_pcm_avail(pcm_out);
    }

    checkStatus(sframes_ready, "sendBufferReady", false);

    return sframes_ready >= len; 
  }

  int AudioALSA::send(void * buf, unsigned int len) {
    int err;
    int olen = len;

    char * cbuf = (char *) buf; 
    while(1) {
      err = snd_pcm_writei(pcm_out, cbuf, len);
      
      if(err == len) return len; 
      else if(err == -EAGAIN) continue;
      else if(err == -EPIPE) {
	// we got an under-run... just ignore it.
	if(err = snd_pcm_recover(pcm_out, err, 1) < 0) {
	checkStatus(err, "send got EPIPE, tried recovery", false);
	}
      }
      else if(err < 0) {
	checkStatus(err, "send", true);
      }
      else if(err != len) {
	len -= err;
	cbuf += (err * datatype_size);
      }
    }
   
    return olen; 
  }

  int AudioALSA::recv(void * buf, unsigned int len, bool block) {
    int err;
    int olen = len;

    int v1, v2;

    char * cbuf = (char *) buf; 
    while(1) {
      err = snd_pcm_readi(pcm_in, cbuf, len);

      if(err == len) return len; 
      else if(err == -EAGAIN) continue; 
      else if(err < 0) {
	checkStatus(err, "recv", true);
      }
      else if(err != len) {
	len -= err;
	cbuf += (err * datatype_size);
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
		       unsigned int _sample_count_hint) :
    AudioIfc(_sample_rate, _fmt, _sample_count_hint, "AudioALSA ALSA Interface")
  {
    std::cerr << "ALSA Sound Library is not enabled in this build version.";  
    throw SoDa::SoDaException("ALSA Sound Library is not enabled in this build version.");  
  }
#endif // HAVE_LIBASOUND
}
