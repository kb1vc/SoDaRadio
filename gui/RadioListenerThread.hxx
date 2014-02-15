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

#ifndef RadioLISTENER_THREAD_HDR
#define RadioLISTENER_THREAD_HDR
#include "../src/UDSockets.hxx"
#include "../src/Command.hxx"

#include <wx/wx.h>
#include <wx/thread.h>

namespace SoDaRadio_GUI {
  class SoDaRadio_Top;

  class RadioListenerThread : public wxThread {
  public:
    RadioListenerThread(SoDaRadio_Top * _radio);

    void * Entry();

  private:
    void setupSpectrumDisplay();
    void setupFreqBuffer();
    void execRepCommand(SoDa::Command * cmd); 
    SoDaRadio_Top * radio_gui;
    SoDa::UD::ClientSocket * cmd_q, * fft_q; 
    float * spect_buffer;
    double * freq_buffer;
    int spect_buflen, old_spect_buflen;
    float spectrum_low_freq, spectrum_hi_freq, spectrum_step_freq; 
  }; 
}

#endif
