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

#include "GPS_TSIPmon.hxx"
#include "CWGenerator.hxx"
#include <list>

SoDa::GPS_TSIPmon::GPS_TSIPmon(Params * params, CmdMBox * _cmd_stream) : SoDa::SoDaThread("GPS_TSIPmon")
{

  cmd_stream = _cmd_stream;
  cmd_subs = cmd_stream->subscribe();

  std::list<TSIP::Report*> replist; 
  PTR_p = new SoDaPrimaryTimingReport(cmd_stream); 
  STR_p = new SoDaSuplementalTimingReport(cmd_stream); 
  replist.push_back(PTR_p);
  replist.push_back(STR_p); 
  
  TSIP_reader_p = new TSIP::Reader(params->getGPSDev(), replist); 
}

void SoDa::GPS_TSIPmon::run()
{
  bool exitflag = false;
  Command * cmd;
  
  while(!exitflag) {
    while((cmd = cmd_stream->get(cmd_subs)) != NULL) {
      // process the command.
      execCommand(cmd);
      exitflag |= (cmd->target == Command::STOP);
      //      std::cerr << "GPS_TSIPmon got a message. target = " << cmd->target << std::endl; 
      cmd_stream->free(cmd);
    }

    TSIP::Report * r = TSIP_reader_p->readStream();
    if(r != NULL) {
      r->updateStatus(); 
    }

    usleep(1000);
  }
}

void SoDa::GPS_TSIPmon::execGetCommand(Command * cmd)
{
  return; 
}

void SoDa::GPS_TSIPmon::execSetCommand(Command * cmd)
{
  return; 
}

void SoDa::GPS_TSIPmon::execRepCommand(Command * cmd)
{
  return;
}
