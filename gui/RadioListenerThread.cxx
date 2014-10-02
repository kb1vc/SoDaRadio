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

#include "RadioListenerThread.hxx"

#include "SoDaRadio_Top.h"
#include "SoDaRadio_App.hxx"
#include "../src/Debug.hxx"
#include <iostream>
#include <wx/wx.h>
#include <wx/app.h>
#include <sys/types.h>
#include <unistd.h>
#include <boost/format.hpp>

namespace SoDaRadio_GUI {

  RadioListenerThread::RadioListenerThread(SoDaRadio_Top * _radio_gui) : SoDa::Debug("RadioListenerThread")
  {
    radio_gui = _radio_gui;

    cmd_q = radio_gui->GetCmdQueue();
    fft_q = radio_gui->GetFFTQueue();

    spect_buflen = 4096;
    old_spect_buflen = spect_buflen;
    spectrum_step_freq = spectrum_low_freq = spectrum_hi_freq = 0.0; 
    freq_buffer = new double[spect_buflen];
    spect_buffer = new float[spect_buflen]; 

  }

  void RadioListenerThread::setupFreqBuffer()
  {
    int i;
    double curfreq = spectrum_low_freq; 
    for(i = 0; i < spect_buflen; i++) {
      curfreq = spectrum_low_freq + ((double) i) * spectrum_step_freq;
      freq_buffer[i] = curfreq;
      spect_buffer[i] = -200.0;
    }
  }

  void RadioListenerThread::setupSpectrumDisplay()
  {
    if((spect_buflen == 0) ||
       (spectrum_step_freq == 0.0) || 
       (spectrum_low_freq == 0.0) || 
       (spectrum_hi_freq == 0.0))
      return;

    if(old_spect_buflen != spect_buflen) {
      freq_buffer = new double[spect_buflen]; 
      spect_buffer = new float[spect_buflen];
    }


    setupFreqBuffer();
  
    // now setup the vectors for the spectrum plots.
    if(old_spect_buflen != spect_buflen) {
      while(!radio_gui->CreateSpectrumTrace(freq_buffer, spect_buffer, spect_buflen)) {
	// the window hasn't been created yet... chill.
	wxThread::Sleep(100); 
      }
      old_spect_buflen = spect_buflen; 
    }
  }

  void * RadioListenerThread::Entry()
  {
    // setup a trap handler for segsegv so we know when it happens.
    hookSigSeg();
    
    //listen on the gui's command queue and on the fft queue.
    bool exitflag = false;

    pid_t pid = getpid(); 


    SoDa::Command * ncmd;

    ncmd = new SoDa::Command();

    int dbgctr = 0; 
    while(!exitflag) {
      bool didwork = false; 
      int stat = cmd_q->get(ncmd, sizeof(SoDa::Command));
      if(stat > 0) {
	if(ncmd->cmd == SoDa::Command::REP) execRepCommand(ncmd);
	didwork = true;
      }

      if(spect_buffer != NULL) {
	stat = fft_q->get(spect_buffer, sizeof(float) * spect_buflen);
	if(stat > 0) {
	  // if((dbgctr & 0xff) == 0) std::cerr << "."; 
	  wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED,
			       SoDaRadio_Top::MSG_UPDATE_SPECTRUM);
	  radio_gui->GetEventHandler()->AddPendingEvent(event); 
	  didwork = true;
	}
	else if(stat < 0) {
	  std::cerr << "Error reading fft q." << std::endl; 
	}
      }
      if(!didwork) {
	wxThread::Sleep(100);
      }

    }

    delete ncmd;   
    std::cerr << "Radio listener thread has shut down." << std::endl; 
    return NULL; 
  }

  void RadioListenerThread::execRepCommand(SoDa::Command * cmd)
  {
    bool check_buf_setup = false; 
    switch(cmd->target) {
    case SoDa::Command::SPEC_RANGE_LOW:
      spectrum_low_freq = cmd->dparms[0] + radio_gui->getRXOffset();
      check_buf_setup = true; 
      break; 
    case SoDa::Command::SPEC_RANGE_HI:
      spectrum_hi_freq = cmd->dparms[0] + radio_gui->getRXOffset(); 
      check_buf_setup = true; 
      break; 
    case SoDa::Command::SPEC_STEP:
      spectrum_step_freq = cmd->dparms[0]; 
      check_buf_setup = true; 
      break; 
    case SoDa::Command::SPEC_BUF_LEN:
      spect_buflen = cmd->iparms[0]; 
      check_buf_setup = true;
      break;
    case SoDa::Command::LO_OFFSET:
      radio_gui->setLOOffset(cmd->dparms[0]);
      break;
    case SoDa::Command::GPS_LATLON:
      radio_gui->setGPSLoc(cmd->dparms[0], cmd->dparms[1]); 
      break; 
    case SoDa::Command::GPS_UTC:
      radio_gui->setGPSTime(cmd->iparms[0], cmd->iparms[1], cmd->iparms[2]); 
      break;
    case SoDa::Command::GPS_LOCK:
      if(cmd->iparms[0] != 0) {
	std::cerr << "GPS is LOCKED" << std::endl; 
      }
      else {
	std::cerr << "GPS is UNLOCKED" << std::endl; 
      }
      break;
    case SoDa::Command::SDR_VERSION:
      radio_gui->setSDRVersion(cmd->sparm); 
      break;
    case SoDa::Command::TX_CW_MARKER:
      {
	wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED,
			     SoDaRadio_Top::MSG_TERMINATE_TX);
	radio_gui->GetEventHandler()->AddPendingEvent(event); 
      }
      break;
    case SoDa::Command::HWMB_REP:
      debugMsg(boost::format("Got HWMB Report [%s]\n") % cmd->sparm); 
      radio_gui->setRadioName(wxString((char*) cmd->sparm, wxConvUTF8));
      debugMsg("Set the Radio Name field.\n");
      break;  
    default:
      break; 
    }

    if(check_buf_setup) {
      debugMsg("Calling setupSpectrumDisplay\n");
      setupSpectrumDisplay();
      debugMsg("Called setupSpectrumDisplay\n");
    }
  }
}
