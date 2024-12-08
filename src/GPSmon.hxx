/*
Copyright (c) 2012,2017,2023,2024 Matthew H. Reilly (kb1vc)
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

#pragma once

#include "SoDaBase.hxx"
#include "SoDaThread.hxx"
#include "MultiMBox.hxx"
#include "Command.hxx"
#include "Params.hxx"
#include "UI.hxx"
#include "fix_gpsd_ugliness.hxx"

#include <time.h>
#include <sys/time.h>

namespace SoDa {
  class GPSmon;
  typedef std::shared_ptr<GPSmon> GPSmonPtr;
  
  class GPSmon : public SoDa::Thread {
  public:
    GPSmon(ParamsPtr params);

    static GPSmonPtr make(ParamsPtr params) {
      return std::make_shared<GPSmon>(params); 
    }

    /// implement the subscription method
    void subscribeToMailBoxList(CmdMailBoxMap & cmd_boxes,
				DatMailBoxMap & dat_boxes);
    
    void run();
  private:
    void execGetCommand(CommandPtr  cmd); 
    void execSetCommand(CommandPtr  cmd); 
    void execRepCommand(CommandPtr  cmd); 

    CmdMBoxPtr cmd_stream;

    GPSDShim * gps_shim; 
  }; 
}

