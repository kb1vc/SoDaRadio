/*
  Copyright (c) 2012,2022 Matthew H. Reilly (kb1vc)
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

#include "GPSmon.hxx"
#include <list>

namespace SoDa {
  
  GPSmon::GPSmon(Params * params) : Thread("GPSmon")
  {
    cmd_stream = NULL;

    gps_shim = new GPSDShim(params->getGPSHostName(), params->getGPSPortName());
  }

  void GPSmon::run()
  {
    bool exitflag = false;
    CmdMsg cmd;

    if(cmd_stream == NULL) {
      throw Radio::Exception(std::string("Missing a stream connection.\n"),
			     this);	
    }
  
    while(!exitflag) {
      while((cmd = cmd_stream->get(cmd_subs)) != NULL) {
	// process the command.
	execCommand(cmd);
	exitflag |= (cmd->target == Command::STOP);
	//      std::cerr << "GPSmon got a message. target = " << cmd->target << std::endl; 
      }

      double lat, lon;
      struct tm utc_time;
      // this could block for 0.1 seconds
      if(gps_shim->getFix(100000, utc_time, lat, lon)) {
	  
	cmd_stream->put(Command::make(Command::REP, Command::GPS_UTC, 
				      (int) utc_time.tm_hour,
				      (int) utc_time.tm_min, 
				      (int) utc_time.tm_sec));

	cmd_stream->put(Command::make(Command::REP, Command::GPS_LATLON, 
				      lat, lon)); 
      }	
      else if(!gps_shim->isEnabled()) {
	// If we have no gps widget, the getFix call returned
	// immediately.  we don't really have anything to do here, so go
	// to sleep for a little while.
	usleep(100000);
      }
    }
  }

  void GPSmon::execGetCommand(Command * cmd)
  {
    (void) cmd; 
    return; 
  }

  void GPSmon::execSetCommand(Command * cmd)
  {
    (void) cmd; 
    return; 
  }

  void GPSmon::execRepCommand(Command * cmd)
  {
    (void) cmd;
    return;
  }

  /// implement the subscription method
  void GPSmon::subscribe() {
    auto reg = MailBoxRegistry::getRegistrar();
  
    cmd_stream = MailBoxBase::convert<MsgMBox>(reg->get("CMD"));
    cmd_subs = cmd_stream->subscribe();
  }
}
