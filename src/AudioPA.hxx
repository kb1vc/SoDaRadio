#ifndef AUDIO_PA_HDR
#define AUDIO_PA_HDR

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

#include <boost/format.hpp>
#include <portaudio.h>
#include <iostream>
#include <stdexcept>

namespace SoDa {
  /**
   * @class AudioPA
   *
   * @brief the PortAudio interface class
   *
   * AudioPA implements the interface specified by AudioIfc.  It should
   * be interchangable with other audio interface handlers.
   *
   * The PortAudio stuff is nice, but the library is very noisy -- it spews out
   * lots of "informational" messages on the console.  While the PA implementation
   * of the Audio class might look simpler, it is not clear that it performs as
   * well or is as clean as the ALSA PCM implementation, despite the fact that the
   * latter is quite likely implemented on top of the former... sigh. 
   */
  class AudioPA : public AudioIfc {
  public:
    /**
     * constructor
     * @param _sample_rate in Hz -- 48000 is a good choice
     * @param _sample_count_hint -- the size of the buffers passed to and from the audio device (in samples)
     * @param _port_name
     */
    AudioPA(unsigned int _sample_rate, 
	    unsigned int _sample_count_hint = 1024,
	    const std::string  & _port_name = std::string("default"));

    ~AudioPA() { }
    /**
     * sendBuf -- send a buffer to the audio output
     * @param buf buffer of type described by the DataFormat selected at init
     * @param len number of elements in the buffer to send
     * @param when_ready if true, test with sendBufferReady and return 0 if not ready
     * otherwise perform the send regardless.
     * @return number of elements transferred to the audio output
     */
    int send(void * buf, unsigned int len, bool when_ready = false);

    /**
     * sendBufferReady -- is there enough space in the audio device send buffer for a call from send?
     * @param len the number of samples that we wish to send
     * @return true if there is sufficient space. 
     */
    bool sendBufferReady(unsigned int len);

    /**
     * recvBuf -- get a buffer of data from the audio input
     * @param buf buffer of type described by the DataFormat selected at init
     * @param len number of elements in the buffer to get
     * @param block block on this call if data is not ready (assumed true).
     * @return number of elements transferred from the audio input
     */
    int recv(void * buf, unsigned int len, bool block = true);

    /**
     * recvBufferReady -- is there enough space in the audio device recv buffer for a call from recv?
     * @param len the number of samples that we wish to get
     * @return true if there is sufficient space. 
     */
    bool recvBufferReady(unsigned int len);

    /**
     * stop the output stream so that we don't encounter a buffer underflow
     * while the reciever is muted.
     */
    void sleepOut() {
      pa_stat = Pa_StopStream(pa_outstream);
      if(pa_stat != paStreamIsStopped) checkStatus(pa_stat, "sleepOut");      
    }
    /**
     * start the output stream
     */
    void wakeOut() {
      pa_stat = Pa_StartStream(pa_outstream);
      if(pa_stat != paStreamIsNotStopped) checkStatus(pa_stat, "wakeOut");
    }
        
    /**
     * stop the input stream so that we don't encounter a buffer overflow
     * while the transmitter is inactive.
     */
    void sleepIn() {
      pa_stat = Pa_StopStream(pa_instream);
      if(pa_stat != paStreamIsStopped) checkStatus(pa_stat, "sleepIn");      
    }
    /**
     * start the input stream
     */
    void wakeIn() {
      pa_stat = Pa_StartStream(pa_instream);
      if(pa_stat != paStreamIsNotStopped) checkStatus(pa_stat, "wakeIn");
    }

  protected:
    PaStream * pa_instream; ///< The capture (input) handle. 
    PaStream * pa_outstream; ///< The playback (output) handle. 
    PaError pa_stat; 

    /**
     * checkStatus -- test a return value and do the right thing
     * @param v the value returned from the call to the PortAudio widget
     * @param exp a string explaining why we were calling the routine in the first place
     * @param fatal if true, we signal an exception, otherwise, just print to the console.
     */
    void checkStatus(PaError v, const std::string & exp, bool fatal = false) {

      if (v != paNoError) {
	if(fatal) throw SoDa::Exception((boost::format("%s %s") % exp % Pa_GetErrorText(v)).str(), this);
	else std::cerr << boost::format("%s: %s %s\n") % getObjName() % exp % Pa_GetErrorText(v);
      }
    }
  };
}


#endif
