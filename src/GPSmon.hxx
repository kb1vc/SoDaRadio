/*
Copyright (c) 2012,2017,2022 Matthew H. Reilly (kb1vc)
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

#ifndef GPS_MON_HDR
#define GPS_MON_HDR
#include "SoDaBase.hxx"
#include "Thread.hxx"
#include "Command.hxx"
#include "Params.hxx"
#include "UI.hxx"
#include "fix_gpsd_ugliness.hxx"

#include <time.h>
#include <sys/time.h>

namespace SoDa {
  class GPSmon : public SoDa::Thread {
  public:
    GPSmon(Params_p params);

    /// implement the subscription method
    void subscribe();
    
    void run();
  private:
    void execGetCommand(Command * cmd); 
    void execSetCommand(Command * cmd); 
    void execRepCommand(Command * cmd); 

    MsgMBoxPtr cmd_stream;
    MsgSubs    cmd_subs;

    GPSDShim * gps_shim; 
  }; 
}


#endif
