#include "AudioPA.hxx"
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

#include <boost/format.hpp>

namespace SoDa {
  AudioPA::AudioPA(unsigned int _sample_rate, DataFormat _fmt, unsigned int _sample_count_hint) :
    AudioIfc(_sample_rate, _fmt, _sample_count_hint, "AudioPA Port Audio Interface") {

    checkStatus(pa_stat = Pa_Initialize(), "Init");

    checkStatus(Pa_OpenDefaultStream(&pa_instream, 1, 0, translateFormat(_fmt), sample_rate, sample_count_hint, NULL, NULL), "OpenIn");

    checkStatus(Pa_OpenDefaultStream(&pa_outstream, 0, 1, translateFormat(_fmt), sample_rate, sample_count_hint, NULL, NULL), "OpenOut");
    
    
  }

  bool AudioPA::sendBufferReady(unsigned int len) {
    int bavail = Pa_GetStreamWriteAvailable(pa_outstream);
    if(bavail < 0) {
      checkStatus(bavail, "sendBufferReady:");
    }
    return bavail >= len; 
  }

  bool AudioPA::recvBufferReady(unsigned int len) {
    int bavail = Pa_GetStreamReadAvailable(pa_instream);
    return bavail >= len; 
  }

  
  int AudioPA::send(void * buf, unsigned int len) {
    int err;
    int olen = len;

    if(!Pa_IsStreamActive(pa_outstream)) {
      checkStatus(Pa_StartStream(pa_outstream), "sendWake");
    }
    pa_stat = Pa_WriteStream(pa_outstream, buf, len);

    if(pa_stat != 0) {
      std::cerr << boost::format("write to audio interface failed (%s)\n") % Pa_GetErrorText(pa_stat);
    }
    return olen; 
  }
  
  int AudioPA::recv(void * buf, unsigned int len, bool block) {
    int err;
    int olen = len;

    if(!Pa_IsStreamActive(pa_instream)) {
      checkStatus(Pa_StartStream(pa_instream), "recvWake");
    }
    pa_stat = Pa_ReadStream(pa_instream, buf, len);
    if(pa_stat != paNoError) {
      std::cerr << boost::format("read from audio interface failed (%s)\n") % Pa_GetErrorText(pa_stat);
    }
    return olen; 
  }

  
  PaSampleFormat AudioPA::translateFormat(AudioIfc::DataFormat fmt) {
    switch(fmt) {
    case FLOAT: return paFloat32;
      break; 
    case DFLOAT:
      throw (new SoDaException("Unsupported data type DFLOAT for PortAudio driver.", this));
      break;
    case INT32: return paInt32;
      break; 
    case INT16: return paInt16;
      break; 
    case INT8: return paInt8;
      break; 
    }
    return paInt16;
  }

}
