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

#include "USRPLO.hxx"
#include "Debug.hxx"
#include <uhd/utils/safe_main.hpp>
#include <uhd/utils/thread_priority.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

SoDa::USRPLO::USRPLO(Params * params, uhd::usrp::multi_usrp::sptr _usrp,
		     CmdMBox * _cmd_stream) : SoDa::SoDaThread("USRPLO")
{
  cmd_stream = _cmd_stream;
  usrp = _usrp; 

  // subscribe to the command stream.
  cmd_subs = cmd_stream->subscribe();

  // find out how to configure the transmitter
  LO_sample_rate = params->getTXRate();
  LO_buffer_size = params->getRFBufferSize();

  // we aren't waiting for anything. 
  waiting_to_run_dry = false; 

  // build the beacon buffer and the zero buffer.
  const_env = new std::complex<float>[LO_buffer_size];
  for(int i = 0; i < LO_buffer_size; i++) {
    const_env[i] = std::complex<float>(1.0, 0.0);
  }
  
  LO_enabled = false;
  LO_configured = false;
  LO_capable = false;
  
  // find out what kind of module we are. 
  cmd_stream->put(new Command(Command::GET, Command::HWMB_REP)); 

  debugMsg("Configuration complete.");
}

void SoDa::USRPLO::initLOStream()
{
  if(LO_capable) {
    // create the tx buffer streamers.
    debugMsg("About to create LO output stream.");
    uhd::stream_args_t stream_args("fc32", "sc16"); 
    stream_args.channels.push_back(1); 
    LO_bits = usrp->get_tx_stream(stream_args);
  }
}

void SoDa::USRPLO::run()
{
  uhd::set_thread_priority_safe(); 
  // now do the event loop.  we watch
  // for commands and responses on the command stream.
  // and we watch for data in the input buffer. 
  
  bool exitflag = false;
  SoDaBuf * txbuf, * cwenv;
  Command * cmd;
  bool didwork = false; 
  
  while(!exitflag) {
    bool didwork = false; 
    if((cmd = cmd_stream->get(cmd_subs)) != NULL) {
      // process the command.
      execCommand(cmd);
      didwork = true; 
      exitflag |= (cmd->target == Command::STOP); 
      cmd_stream->free(cmd); 
      didwork = true; 
    }
    else if(LO_enabled && LO_configured && LO_capable) {
      // now send it to the USRP
      LO_bits->send(const_env, LO_buffer_size, md);
      md.start_of_burst = false;
      didwork = true; 
    }

    if(!didwork) {
      usleep(100);
    }
  }
}


void SoDa::USRPLO::execSetCommand(Command * cmd)
{
  switch(cmd->target) {
  case SoDa::Command::TVRT_LO_ENABLE:
    debugMsg("Enable Transverter LO");
    LO_enabled = true; 
    md.start_of_burst = true;
    md.end_of_burst = false; 
    md.has_time_spec = false; 
    break; 
  case SoDa::Command::TVRT_LO_DISABLE:
    debugMsg("Disable Transverter LO");
    md.start_of_burst = false;
    md.end_of_burst = true;
    if(LO_capable) {
      LO_bits->send("", 0, md);
    }
    LO_enabled = false; 
    break;
  default:
    break; 
  }
}

void SoDa::USRPLO::execGetCommand(Command * cmd)
{
}

void SoDa::USRPLO::execRepCommand(Command * cmd)
{
  switch(cmd->target) {
  case SoDa::Command::TVRT_LO_CONFIG:
    debugMsg("LO configured");
    LO_configured = true; 
    break;
  case SoDa::Command::HWMB_REP:
    std::cerr << boost::format("USRPLO got model name of [%s]\n") % cmd->sparm;
    if(std::string(cmd->sparm) == "B210") {
      LO_capable = true;
      initLOStream();
    }
    break; 
  default:
    break;
  }
}

