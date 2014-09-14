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

#ifndef GPS_TSIPmon_HDR
#define GPS_TSIPmon_HDR
#include "SoDaBase.hxx"
#include "MultiMBox.hxx"
#include "Command.hxx"
#include "Params.hxx"
#include "UI.hxx"
#include "TSIP.hxx"


#include <time.h>
#include <sys/time.h>

namespace SoDa {
  class GPS_TSIPmon : public SoDaThread {
  public:
    GPS_TSIPmon(Params * params, CmdMBox * cmd_stream);
    void run();
  private:
    void execGetCommand(Command * cmd); 
    void execSetCommand(Command * cmd); 
    void execRepCommand(Command * cmd); 

    // this is the basetime
    struct timeval basetime; 
    double curtime() {
      struct timeval tp;
      gettimeofday(&tp, NULL);
      return ((double) (tp.tv_sec - basetime.tv_sec)) + 1e-6*((double)tp.tv_usec);
    }

    CmdMBox *cmd_stream;
    unsigned int cmd_subs;

    // Trimble TSIP reports
    class SoDaPrimaryTimingReport : public TSIP::PrimaryTimingReport {
    public:
      SoDaPrimaryTimingReport(CmdMBox * _mbox) : mbox(_mbox) {
      }
      bool updateStatus() {
	mbox->put(new SoDa::Command(Command::REP, Command::GPS_UTC,
				    (int) TimeHours, (int) TimeMinutes, (int) TimeSeconds));
	return true; 	
      }
    private:
      CmdMBox * mbox; 

    }; 

    class SoDaSuplementalTimingReport : public TSIP::SuplementalTimingReport {
    public:
      SoDaSuplementalTimingReport(CmdMBox * _mbox) : mbox(_mbox) {
      }
      bool updateStatus() {
	unsigned int p = ((unsigned int) SelfSurveyProgress); 
	mbox->put(new SoDa::Command(Command::REP, Command::GPS_LATLON, Latitude, Longitude));
	return true; 

	return true; 
      }
    private:
      CmdMBox * mbox; 

    }; 

    SoDaPrimaryTimingReport * PTR_p;
    SoDaSuplementalTimingReport * STR_p;

    TSIP::Reader * TSIP_reader_p;
  }; 
}


#endif
