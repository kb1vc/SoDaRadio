#ifndef AUDIO_PCM_HDR
#define AUDIO_PCM_HDR


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
namespace SoDa {
  /**
   * @brief Generic Audio Interface Class
   *
   * This is a pure virtual class that is the interface spec
   * for whatever actual audio interface is provided.  The spec
   * is suitable for implementations using ALSA (@see AudioALSA),
   * Port Audio (@see AudioPA),
   * and an IP link.  The network interface has not yet been implemented.
   */
  class AudioIfc : public SoDaBase {
  public:
    enum DataFormat { FLOAT, DFLOAT, INT32, INT16, INT8 };

    /*
     * constructor
     * @param sample_rate in Hz -- 48000 is a good choice
     * @param fmt -- the format of the data (FLOAT, DFLOAT, INT32, INT16, INT8)
     * @param _sample_count_hint -- the size of the buffers passed to
     *                              and from the audio device (in samples)
     */
    AudioIfc(unsigned int _sample_rate,
	     DataFormat _fmt,
	     unsigned int _sample_count_hint,
	     const std::string & name = "AudioIfc") : SoDaBase(name) {
      sample_rate = _sample_rate;
      sample_count_hint = _sample_count_hint;
      format = _fmt;
      switch (format) {
      case FLOAT: datatype_size = sizeof(float);
	break; 
      case DFLOAT: datatype_size = sizeof(double);
	break; 
      case INT32: datatype_size = sizeof(int);
	break; 
      case INT16: datatype_size = sizeof(short);
	break; 
      case INT8: datatype_size = sizeof(char);
	break; 
      }
    }

    /**
     * send -- send a buffer to the audio output
     * @param buf buffer of type described by the DataFormat selected at init
     * @param len number of elements in the buffer to send
     * @return number of elements transferred to the audio output
     */
    virtual int send(void * buf, unsigned int len) = 0; 

    /**
     * sendBufferReady -- is there enough space in the audio device
     *                    send buffer for a call from send?
     * @param len the number of samples that we wish to send
     * @return true if there is sufficient space. 
     */
    virtual bool sendBufferReady(unsigned int len) = 0; 


    /**
     * recv -- get a buffer of data from the audio input
     * @param buf buffer of type described by the DataFormat selected at init
     * @param len number of elements in the buffer to send
     * @param block block on this call if no data is available (ignored)
     * @return number of elements transferred to the audio output
     */
    virtual int recv(void * buf, unsigned int len, bool block = true) = 0;

    /**
     * recvBufferReady -- is there enough space in the audio device
     *                    recv buffer for a call from recv?
     * @param len the number of samples that we wish to get
     * @return true if there is sufficient space. 
     */
    virtual bool recvBufferReady(unsigned int len) = 0; 


    /**
     * set the gain for the output device.
     * @param gain -- range from 0 to 1.0
     * @return true if gain was set, false otherwise.
     */
    virtual bool setOutGain(float gain) {
      out_gain = gain;
      return true; 
    }

    /**
     * set the gain for the input device.
     * @param gain -- range from 0 to 1.0
     * @return true if gain was set, false otherwise.
     */
    virtual bool setInGain(float gain) {
      in_gain = gain;
      return true; 
    }

    /**
     * get the gain for the output device.
     * @return the gain; 
     */
    virtual float getOutGain() { return out_gain; }

    /**
     * get the gain for the input device.
     * @return input gain
     */
    virtual float getInGain() { return in_gain; }

    /**
     * stop the output stream so that we don't encounter a buffer underflow
     * while the reciever is muted.
     */
    virtual void sleepOut() = 0;
    /**
     * start the output stream
     */
    virtual void wakeOut() = 0;
        
    /**
     * stop the input stream so that we don't encounter a buffer overflow
     * while the transmitter is inactive.  This should also flush all incoming
     * buffers. (Virtual audio devices can get ahead of the processing by a good bit.)
     */
    virtual void sleepIn() = 0;
    /**
     * start the input stream
     */
    virtual void wakeIn() = 0;
        

    virtual std::string currentPlaybackState() { return std::string("UNKNOWN"); }
    virtual std::string currentCaptureState() { return std::string("UNKNOWN"); }    

  protected:
    unsigned int sample_rate;
    DataFormat format;
    unsigned int sample_count_hint; 

    float in_gain;
    float out_gain; 

    int datatype_size; 
  };
}


#endif
