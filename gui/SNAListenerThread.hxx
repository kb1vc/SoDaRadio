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

#ifndef SNALISTENER_THREAD_HDR
#define SNALISTENER_THREAD_HDR
#include "../src/UDSockets.hxx"
#include "../src/Command.hxx"
#include "../src/Debug.hxx"
extern "C" {
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
}

#include <wx/wx.h>
#include <wx/thread.h>

namespace SoDaSNA_GUI {
  class SoDaSNA_Top;

  class SNAListenerThread : public wxThread, public SoDa::Debug {
  public:
    SNAListenerThread(SoDaSNA_Top * _radio);

    void * Entry();

    void stop() {
      if(IsPaused()) {
	debugMsg("Waking thread to send it a STOP.");
	// wake the thread up so we can kill it....
	Resume(); 
      }
      debugMsg("++++++++++  Got stop command -- exiting ++++++++");
      exitflag = true;
    }

  private:
    void execRepCommand(SoDa::Command * cmd);
    void execSetCommand(SoDa::Command * cmd);    

    bool exitflag; // Set to true when we want to terminate the application
    
    SoDaSNA_Top * SNA_gui;
    SoDa::UD::ClientSocket * cmd_q;
    float * spect_buffer;
    double * freq_buffer;
    int spect_buflen, old_spect_buflen;
    float spectrum_low_freq, spectrum_hi_freq, spectrum_step_freq;

    static void segfault_handler(int sig) {
      std::cerr << "Radio Listener Thread got a segfault." << std::endl;
      exit(-1);
    }

    void hookSigSeg() {
      struct sigaction act;
      act.sa_handler = SNAListenerThread::segfault_handler;
      sigemptyset(&act.sa_mask);
      act.sa_flags = 0;
      sigaction(SIGSEGV, &act, 0);
    }
  };

}

#endif
