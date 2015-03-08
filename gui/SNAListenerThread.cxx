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

#include "SNAListenerThread.hxx"
#include "SoDaSNA_Top.h"
#include "SoDaSNA_App.hxx"
#include "../src/Debug.hxx"
#include <iostream>
#include <wx/wx.h>
#include <wx/app.h>
#include <sys/types.h>
#include <unistd.h>
#include <boost/format.hpp>

namespace SoDaSNA_GUI {

  SNAListenerThread::SNAListenerThread(SoDaSNA_Top * _SNA_gui) : SoDa::Debug("SNAListenerThread")
  {
    SNA_gui = _SNA_gui;

    cmd_q = SNA_gui->GetCmdQueue();
  }


  void * SNAListenerThread::Entry()
  {
    // setup a trap handler for segsegv so we know when it happens.
    hookSigSeg();
    
    //listen on the gui's command queue and on the fft queue.
    exitflag = false;

    pid_t pid = getpid(); 

    SoDa::Command * ncmd;

    ncmd = new SoDa::Command();

    int dbgctr = 0;
    int repctr = 0;
    int specctr = 0;
    
    while(!exitflag) {
      bool didwork = false;
      dbgctr++; 
	  
      int stat = cmd_q->get(ncmd, sizeof(SoDa::Command));
      if(stat > 0) {
	if(ncmd->cmd == SoDa::Command::REP) execRepCommand(ncmd);
	else if(ncmd->cmd == SoDa::Command::SET) execSetCommand(ncmd);	
	didwork = true;
	repctr++; 
      }

      if(!didwork) {
	wxThread::Sleep(100);
      }
    }

    delete ncmd;   
    debugMsg("+++++++++++ Radio listener thread has shut down.\n");

    if(TestDestroy()) {
      debugMsg("TestDestroy returned true, calling Exit");
      Exit();
    }
    return NULL; 
  }

  void SNAListenerThread::execRepCommand(SoDa::Command * cmd)
  {
    bool check_buf_setup = false; 
    switch(cmd->target) {
    case SoDa::Command::HWMB_REP:
      debugMsg(boost::format("Got HWMB Report [%s]\n") % cmd->sparm); 
      SNA_gui->setRadioName(wxString((char*) cmd->sparm, wxConvUTF8));
      debugMsg("Set the Radio Name field.\n");
      break;
    case SoDa::Command::SNA_SCAN_REPORT:
      SNA_gui->createUpdateEvent(cmd->dparms[0], cmd->dparms[1]);
      debugMsg(boost::format("SCAN Report freq = %g  mag = %g\n") 
	       % cmd->dparms[0] % cmd->dparms[1]);
      break;
    case SoDa::Command::SNA_SCAN_END:
      SNA_gui->createScanEndEvent();
      debugMsg("SCAN End\n");
      break;
    default:
      break; 
    }
  }

  void SNAListenerThread::execSetCommand(SoDa::Command * cmd)
  {
    switch(cmd->target) {
    default:
      break; 
    }
  }

}
