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
#define ALSA_DEBUG_FLOW 
namespace SoDa {
#if HAVE_LIBASOUND
  boost::mutex AudioALSA::alsa_lib_mutex; 
  
  AudioALSA::AudioALSA(unsigned int _sample_rate,
		       unsigned int _sample_count_hint) :
    AudioIfc(_sample_rate, _sample_count_hint, "AudioALSA ALSA Interface") {

    // we translate all floats into shorts...
    recv_buflen = sample_count_hint * 2;
    send_buflen = sample_count_hint * 2;
    // and it is two channels wide.
    short_send_buffer = new short[send_buflen * 2];
    short_recv_buffer = new short[recv_buflen * 2];
    
    try {
      // code is largely borrowed from equalarea.com/paul/alsa-audio.html
      std::cerr << "ALSA About to setup playback.\n";
      setupPlayback();

      std::cerr << "ALSA About to setup capture.\n";
      setupCapture();
    }
    catch (SoDa::SoDaException * exc) {
      std::cerr << "AudioALSA caught fatal exception in setup. " << std::endl; 
      std::cerr << "\t" << exc->toString() << std::endl; 
      exit(-1);
    }
    underrun_count = 0;
    event_count = write_count = chunklet_count = whole_count = again_count = 0;
    std::cerr << "ALSA setup complete.\n";
  }

  void AudioALSA::setupPlayback()
  {
    boost::mutex::scoped_lock lock(alsa_lib_mutex);    
    // setup the playback (output) stream
    char pcm_name[] = "front"; // "default"; 
    if(snd_pcm_open(&pcm_out, pcm_name, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK) < 0) {
      std::cerr << boost::format("can't open Alsa PCM device for output ... Crap.\n");
      exit(-1); 
    }
    std::cerr << "About to call setupParams from playback." << std::endl;

    setupParams(pcm_out, hw_out_params);
  }

  void AudioALSA::setupCapture()
  {
    boost::mutex::scoped_lock lock(alsa_lib_mutex);    
    char pcm_cap_name[] = "hw:0,0"; // "default"; // "hw:0,2";
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

    std::cerr << "pre p_a" << std::endl;     
    checkStatus(snd_pcm_hw_params_any (dev, hw_paramsp), "setupParams init parm block", true);

    std::cerr << "pre s_fmt" << std::endl; 
    
    snd_pcm_format_t alsa_fmt = SND_PCM_FORMAT_S16_LE; // we use signed shorts...
    checkStatus(snd_pcm_hw_params_set_format (dev, hw_paramsp, alsa_fmt),
		"setupParams set format", true);


    std::cerr << "pre s_r_n" << std::endl; 

    checkStatus(snd_pcm_hw_params_set_rate_near (dev, hw_paramsp, &sample_rate, 0), 
		"setupParams set sample rate", true);

    std::cerr << "pre s_c" << std::endl; 
    checkStatus(snd_pcm_hw_params_set_channels (dev, hw_paramsp, 2), 
		"setupParams set number of channels", true);

    std::cerr << "pre s_b_s" << std::endl; 
    checkStatus(snd_pcm_hw_params_set_buffer_size (dev, hw_paramsp,
						   sample_count_hint * 2), // size is in frames... * datatype_size), 
		"setupParams set buffer size", true);

    std::cerr << "pre s_acc" << std::endl; 
    checkStatus(snd_pcm_hw_params_set_access (dev, hw_paramsp, SND_PCM_ACCESS_RW_INTERLEAVED),
		"setupParams set access", true);
    
    std::cerr << "pre s_pb" << std::endl; 
    checkStatus(snd_pcm_hw_params (dev, hw_paramsp), 
		"setupParams set parameter block", true);

    std::cerr << "pre prep" << std::endl;     
    checkStatus(snd_pcm_prepare (dev), 
		"setupParams prepare audio interface", true);
    std::cerr << "leaving setup." << std::endl; 
  }


  bool AudioALSA::recvBufferReady(unsigned int len)  {
    boost::mutex::scoped_lock lock(alsa_lib_mutex);
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
      std::cerr << "Got rcv buf ready return of " << sframes_ready << std::endl;
      checkStatus(sframes_ready, "recvBufferReady", false);
    }
    std::cerr << boost::format("AudioALSA recvBufferReady Got %d frames\n") % sframes_ready; 
    return sframes_ready >= len;
  }

  bool AudioALSA::sendBufferReady(unsigned int len)  {
    boost::mutex::scoped_lock lock(alsa_lib_mutex);    
    snd_pcm_sframes_t sframes_ready;
    while(1) {
      sframes_ready = snd_pcm_avail(pcm_out);
      if(sframes_ready == -EPIPE) {
	// we got an under-run... just ignore it.
	int err; 
	if(err = snd_pcm_recover(pcm_out, sframes_ready, 1) < 0) {
	  checkStatus(err, "sendBufferReady got EPIPE, tried recovery", false);
	}
      }
      else {
	break;
      }
    }
    checkStatus(sframes_ready, "sendBufferReady", false);

    // std::cerr << boost::format("AudioALSA sendBufferReady Got %d frames\n") % sframes_ready; 
    
    return sframes_ready >= len;
  }

  int AudioALSA::send(float * buf, unsigned int len) {
    boost::mutex::scoped_lock lock(alsa_lib_mutex);
    int err;
    int olen = len;

    translateFloat2Short(buf, len); 

    len = len * 2; // we're sending pairs... 
    char * cbuf = (char *) short_send_buffer; 
    while(1) {
      err = snd_pcm_writei(pcm_out, cbuf, len);
      write_count++; 
      event_count++;
      if(err == len) {
	whole_count++; 
	break;
      }
      else if(err == -EAGAIN){
	again_count++; 
	event_count++;
	continue;
      }
      else if(err == -EPIPE) {
	// we got an under-run... just ignore it.
	underrun_count++;
	event_count++;
	snd_pcm_prepare(pcm_out);	
	// if((err = snd_pcm_recover(pcm_out, err, 1)) < 0) {
	// 	  checkStatus(err, "send got EPIPE, tried recovery", false);
	// }
      }
      else if(err < 0) {
	checkStatus(err, "send", true);
      }
      else if(err != len) {
	chunklet_count++;
	event_count++;	
	len -= err;
	cbuf += (err * sizeof(short) * 2);
      }
    }

#ifdef ALSA_DEBUG_FLOW   
    if((event_count & 0xff) == 0) {
      event_count++; 
      std::cerr << boost::format("AAAAAAA: again = %d underrun = %d chunklet = %d whole = %d write = %d event = %d\n")
	% again_count % underrun_count % chunklet_count % whole_count % write_count % event_count;
    }
#endif    
    return olen; 
  }

  int AudioALSA::recv(float * fbuf, unsigned int len, bool block) {
    boost::mutex::scoped_lock lock(alsa_lib_mutex);
    int err;
    unsigned int olen = len; 
    
    if(len > recv_buflen) {
      delete[] short_recv_buffer; 
      recv_buflen = len; 
      short_recv_buffer = new short[recv_buflen * 2]; 
    }

    char * cbuf = (char *) short_recv_buffer;
    while(1) {
      err = snd_pcm_readi(pcm_in, cbuf, len);

      if(err == len) return len; 
      else if(err == -EAGAIN) continue; 
      else if(err < 0) {
	checkStatus(err, "recv", true);
      }
      else if(err != len) {
	len -= err;
	cbuf += (err * sizeof(short) * 2);
      }
    }

    translateShort2Float(fbuf, olen);
    return olen;
  }


  bool AudioALSA::translateFloat2Short(float * fb, unsigned int len) {
    short * obuf; 
    unsigned int oblen; 

    if(send_buflen < len) {
      delete[] short_send_buffer;
      send_buflen = len; 
      short_send_buffer = new short[send_buflen * 2]; 
    }

    int i;
    for(i = 0; i < len; i++) {
      short sv = float2Short(fb[i]); 
      short_send_buffer[i * 2] = sv;
      short_send_buffer[i * 2 + 1] = sv; 	
    }

    return true; 
  }

  bool AudioALSA::translateShort2Float(float * fb, unsigned int len) 
  {
    int i;
    for(i = 0; i < len; i++) {
      fb[i] = short2Float((short_recv_buffer[i * 2] + short_recv_buffer[i * 2 + 1]) / 2);
    }
    return true; 
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
