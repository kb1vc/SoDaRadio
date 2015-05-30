/*
  Copyright (c) 2015, Matthew H. Reilly (kb1vc)
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

#include "TransLO.hxx"
#include <uhd/utils/safe_main.hpp>
#include <uhd/utils/thread_priority.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

SoDa::TransLO::TransLO(Params * params, uhd::usrp::multi_usrp::sptr _usrp,
		       CmdMBox * _cmd_stream) : SoDa::SoDaThread("TransLO")
{
  // It is important to NOT setup streamers here -- wait until the USRPCtl
  // unit is up and running and commands are circulating. 
  cmd_stream = _cmd_stream;
  usrp = _usrp; 

  // subscribe to the command stream.
  cmd_subs = cmd_stream->subscribe();

  LO_enabled = false;
  LO_configured = false;

  // create the tx buffer streamers.
  stream_args = new uhd::stream_args_t("fc32", "sc16");
  
  debugMsg("This radio is transverter LO capable");
  // use the second channel as a transverter LO
  stream_args->channels.push_back(1);

  // find out how to configure the transmitter
  tx_sample_rate = params->getTXRate();
  tx_buffer_size = params->getRFBufferSize();
  
  const_buf = new std::complex<float>[tx_buffer_size];
  zero_buf = new std::complex<float>[tx_buffer_size];      
  for(int i = 0; i < tx_buffer_size; i++) {
    zero_buf[i] = std::complex<float>(0.0, 0.0);
    const_buf[i] = std::complex<float>(0.3, 0.3);
  }
}

void SoDa::TransLO::run()
{
  uhd::set_thread_priority_safe(); 
  // now do the event loop.  we watch
  // for commands and responses on the command stream.
  // and we watch for data in the input buffer. 

  // get the tx streamer 
  debugMsg("Creating LO streamer.\n");
  getLOStreamer();  
  debugMsg("Created LO streamer.\n");

  bool exitflag = false;
  Command * cmd; 
  std::vector<std::complex<float> *> buffers(1);

  int debug_ctr = 0; 
  while(!exitflag) {
    bool didwork = false; 
    buffers[0] = const_buf;
    
    if((cmd = cmd_stream->get(cmd_subs)) != NULL) {
      // process the command.
      execCommand(cmd);
      didwork = true; 
      exitflag |= (cmd->target == Command::STOP); 
      cmd_stream->free(cmd); 
    }
    else if(LO_configured && LO_enabled) {
      // get a buffer and 
      tx_bits->send(buffers, tx_buffer_size, md);
      md.start_of_burst = false; 
      didwork = true; 

      if((debug_ctr & 1023) == 0) {
	debugMsg((boost::format("sent %d LO bursts\n") % debug_ctr).str());
      }
      debug_ctr++; 
    }

    if(!didwork) {
      usleep(1000);
    }
  }

  debugMsg("Leaving\n");
}



void SoDa::TransLO::getLOStreamer()
{
  try {
    tx_bits = usrp->get_tx_stream(*stream_args);
    //  rx_dummy_bits = usrp->get_rx_stream(*stream_args); 
  } 
  catch (const std::exception & e) {
    std::cerr << e.what() << boost::format(" fail in getLOStreamer\n");
  }
}


void SoDa::TransLO::execSetCommand(Command * cmd)
{
  switch(cmd->target) {
  case SoDa::Command::TVRT_LO_ENABLE:
    debugMsg("Enable Transverter LO");
    md.start_of_burst = true; 
    md.end_of_burst = false; 
    LO_enabled = true; 
    break; 
  case SoDa::Command::TVRT_LO_DISABLE:
    debugMsg("Disable Transverter LO");
    md.end_of_burst = true; 
    tx_bits->send(zero_buf, 10, md); 
    LO_enabled = false; 
    break;
  default:
    break; 
  }
}

void SoDa::TransLO::execRepCommand(Command * cmd)
{
  switch(cmd->target) {
  case SoDa::Command::TVRT_LO_CONFIG:
    debugMsg("LO configured");
    LO_configured = true; 
    break;
  default:
    break;
  }
}
