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
#if HAVE_LIBASOUND
#  include <alsa/asoundlib.h>
#  define ALSA_DEF
#else
#  define ALSA_DEF { throw SoDa::SoDaException("ALSA Sound Library is not enabled in this build version."); } 
#endif
#include <boost/format.hpp>

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

#include <iostream>
#include <stdexcept>

namespace SoDa {
  /**
   * @class AudioALSA
   *
   * @brief the ALSA audio interface class
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
  class AudioALSA : public AudioIfc {
  public:
    /**
     * constructor
     * @param _sample_rate in Hz -- 48000 is a good choice
     * @param _fmt -- the format of the data (FLOAT, DFLOAT, INT32, INT16, INT8)
     * @param _sample_count_hint -- the size of the buffers passed to and from
     *                              the audio device (in samples)
     */
    AudioALSA(unsigned int _sample_rate,
	      unsigned int _sample_count_hint = 1024);

    ~AudioALSA() {
#if HAVE_LIBASOUND
      snd_pcm_close(pcm_out);
#endif      
    }
    
    /**
     * send -- send a buffer to the audio output
     * @param buf buffer of type described by the DataFormat selected at init
     * @param len number of elements in the buffer to send
     * @return number of elements transferred to the audio output
     */
    int send(float * buf, unsigned int len) ALSA_DEF ;

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
     * @param block make this a blocking call --- ignored. 
     * @return number of elements transferred from the audio input
     */
    int recv(float * buf, unsigned int len, bool block = true) ALSA_DEF ; 

    /**
     * recvBufferReady -- is there enough space in the audio device
     *                    recv buffer for a call from recv?
     * @param len the number of samples that we wish to get
     * @return true if there is sufficient space. 
     */
    bool recvBufferReady(unsigned int len) ALSA_DEF ;

    /**
     * stop the output stream so that we don't encounter a buffer underflow
     * while the reciever is muted.
     */
    void sleepOut() {
#if HAVE_LIBASOUND
      snd_pcm_drain(pcm_out);
#endif
    }
    /**
     * start the output stream
     */
    void wakeOut() {
#if HAVE_LIBASOUND
      int err; 
      if((err = snd_pcm_prepare(pcm_out)) < 0) {
	throw
	  SoDaException((boost::format("AudioALSA::wakeOut() Failed to wake after sleepOut() -- %s")
			 % snd_strerror(err)).str(), this);
      }
      if((err = snd_pcm_start(pcm_out)) < 0) {
	throw
	  SoDaException((boost::format("AudioALSA::wakeOut() Failed to wake after sleepOut() -- %s")
			 % snd_strerror(err)).str(), this);
      }
#endif
    }
        
    /**
     * stop the input stream so that we don't encounter a buffer overflow
     * while the transmitter is inactive.
     */
    void sleepIn() {
#if HAVE_LIBASOUND
      snd_pcm_drop(pcm_in); 
#endif
    }
    /**
     * start the input stream
     */
    void wakeIn() {
#if HAVE_LIBASOUND
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

  protected:
#if HAVE_LIBASOUND    
    snd_pcm_t * pcm_out; ///< The playback (output) handle. 
    snd_pcm_t * pcm_in;  ///< The capture (input) handle. 
    snd_pcm_hw_params_t * hw_in_params;  ///< the input parameter list
    snd_pcm_hw_params_t * hw_out_params; ///< the output parameter list

    int underrun_count; ///< count of number of times we posted to the audio out and it had run dry. 
    int event_count; ///< how many anomalous and good send events did we have
    int write_count; ///< how many writes
    int chunklet_count; ///< how many short buffers
    int whole_count; ///< how many full buffers    
    int again_count; ///< how many eagain events
    

    /**
     * setup the playback handle and features. 
     */
    void setupPlayback();

    /**
     * setup the capture handle and features.
     */
    void setupCapture(); 

    /**
     * checkStatus check to see if the return status from an alsa call was OK
     * @param err -- the error number
     * @param exp -- why are we here
     * @param fatal -- if true, throw an exception, otherwise print an error to std::cerr
     */
    void checkStatus(int err, const std::string & exp, bool fatal = false) {
      
      if (err < 0) {
	if(fatal) {
	  std::cerr << "AudioALSA!!!: " << exp << snd_strerror(err) << std::endl;
	  throw SoDaException((boost::format("%s %s") % exp % snd_strerror(err)).str(), this);
	}
	else std::cerr << boost::format("%s %s %s\n") % getObjName() % exp % snd_strerror(err);
      }
    }

    /**
     * setup the parameters for a PCM device
     * @param dev the device handle
     * @param hw_params (out parameter) a pointer to a device parameter block
     */
    void setupParams(snd_pcm_t * dev, snd_pcm_hw_params_t * & hw_params);
    

    // we keep paired short buffers!!
        short float2Short(float v) 
    {
      if(v > 1.0) return 32767; 
      if(v < -1.0) return -32767; 
      else return static_cast<short>(v * 32767.0);
    }

    float short2Float(short v) 
    {
      float fv = static_cast<float>(v); 
      fv = fv / 32768.0; 
      if(fv > 1.0) return 1.0;
      if(fv < -1.0) return -1.0;
      else return fv;
    }

    bool translateFloat2Short(float * fb, unsigned int len);
    
    bool translateShort2Float(float * fb, unsigned int len);

    short * short_send_buffer;
    unsigned int send_buflen; 
    short * short_recv_buffer;
    unsigned int recv_buflen;     

    
    // mutual exclusion -- prevent us from "crossing the streams"
    // between send and recv clients. 
    static boost::mutex alsa_lib_mutex; 
    
#endif // HAVE_LIBASOUND
  };
}


#endif
