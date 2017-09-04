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

#include "GPSmon.hxx"
#include <list>
#include <errno.h>

SoDa::GPSmon::GPSmon(Params * params, CmdMBox * _cmd_stream) : SoDa::SoDaThread("GPSmon")
{
  cmd_stream = _cmd_stream;
  cmd_subs = cmd_stream->subscribe();

#if HAVE_GPSLIB  
  // now open the server connection
  errno = 0; 
  int stat = gps_open(params->getGPSHostName().c_str(), 
		      params->getGPSPortName().c_str(), 
		      &gps_data); 
  if(stat != 0) {
    std::cerr << boost::format("Could not open gpsd server connection. Error : %s\n") % gps_errstr(errno); 
    gps_server_ready = false; 
  }
  else gps_server_ready = true; 
  
  (void) gps_stream(&gps_data, WATCH_ENABLE, NULL);
#else
  gps_server_ready = false; 
#endif
  
}

void SoDa::GPSmon::run()
{
  bool exitflag = false;
  Command * cmd;
  
  int stat; 

  while(!exitflag) {
    while((cmd = cmd_stream->get(cmd_subs)) != NULL) {
      // process the command.
      execCommand(cmd);
      exitflag |= (cmd->target == Command::STOP);
      //      std::cerr << "GPSmon got a message. target = " << cmd->target << std::endl; 
      cmd_stream->free(cmd);
    }
    if(gps_server_ready) {
#if HAVE_GPSLIB          
      if(gps_waiting(&gps_data, 100000)) {
	errno = 0; 
	stat = gps_read(&gps_data); 
	if(stat == -1) gps_server_ready = false; 
	else {
	  time_t utc_time = (time_t) gps_data.fix.time; 
	  struct tm btime; 

	  gmtime_r(&utc_time, &btime); 
	  
	  cmd_stream->put(new SoDa::Command(Command::REP, Command::GPS_UTC, 
					    (int) btime.tm_hour,
					    (int) btime.tm_min, 
					    (int) btime.tm_sec));

	  cmd_stream->put(new SoDa::Command(Command::REP, Command::GPS_LATLON, 
					    gps_data.fix.latitude, 
					    gps_data.fix.longitude)); 
	}
      }
#endif    
    }
    else {
      usleep(100000);
    }
  }
}

void SoDa::GPSmon::execGetCommand(Command * cmd)
{
  (void) cmd; 
  return; 
}

void SoDa::GPSmon::execSetCommand(Command * cmd)
{
  (void) cmd; 
  return; 
}

void SoDa::GPSmon::execRepCommand(Command * cmd)
{
  (void) cmd;
  return;
}
