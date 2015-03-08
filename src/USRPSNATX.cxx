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

#include "USRPSNATX.hxx"
#include <uhd/utils/safe_main.hpp>
#include <uhd/utils/thread_priority.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

SoDa::USRPSNATX::USRPSNATX(Params * params, uhd::usrp::multi_usrp::sptr _usrp,
		     CmdMBox * _cmd_stream) : SoDa::SoDaThread("USRPSNATX")
{
  cmd_stream = _cmd_stream;
  usrp = _usrp; 

  // subscribe to the command stream.
  cmd_subs = cmd_stream->subscribe();

  // create the tx buffer streamers.
  stream_args = new uhd::stream_args_t("fc32", "sc16");
  stream_args->channels.push_back(0);

  // find out how to configure the transmitter
  tx_sample_rate = params->getTXRate();
  tx_buffer_size = params->getRFBufferSize();

  debugMsg(boost::format("Sample Rate %g Buffer Size %d\n") 
	   % tx_sample_rate % tx_buffer_size);

  // build the zero buffer and the transverter lo buffer
  const_buf = new std::complex<float>[tx_buffer_size];
  zero_buf = new std::complex<float>[tx_buffer_size];  
  for(int i = 0; i < tx_buffer_size; i++) {
    const_buf[i] = std::complex<float>(0.5, 0.5);
    zero_buf[i] = std::complex<float>(0.0, 0.0);    
  }
  
  tx_enabled = false;
}


void SoDa::USRPSNATX::run()
{
  uhd::set_thread_priority_safe(); 
  // now do the event loop.  we watch
  // for commands and responses on the command stream.
  // and we watch for data in the input buffer. 
  
  bool exitflag = false;
  Command * cmd; 
  int dbg_count = 0; 

  while(!exitflag) {
    bool didwork = false; 
    
    while((cmd = cmd_stream->get(cmd_subs)) != NULL) {
      // process the command.
      debugMsg(boost::format("Got command [%s]\n") % cmd->toString());
      execCommand(cmd);
      didwork = true; 
      exitflag |= (cmd->target == Command::STOP); 
      cmd_stream->free(cmd); 
      if(tx_enabled) debugMsg("TX enabled.");
      else debugMsg("TX disabled.");      
    }
    
    if(tx_enabled) {
      int sentcount; 
      sentcount = tx_bits->send(const_buf, tx_buffer_size, tx_md);
      if((dbg_count & 0xff) == 0) {
	debugMsg(boost::format("Sent %d bytes dbg_count %d\n") % sentcount % dbg_count); 
      }
      didwork = true;

    }

    if(!didwork) {
      usleep(100);
    } 
    else {
      dbg_count++;                 
    }
  }
}


void SoDa::USRPSNATX::transmitSwitch(bool tx_on)
{
  debugMsg("In transmitSwitch");
  if(tx_on) {
    debugMsg("in tx_on");
    if(tx_enabled) return;
    debugMsg("Starting burst"); 
    tx_md.start_of_burst = true;
    tx_md.end_of_burst = false;
    tx_md.has_time_spec = false; 
    tx_enabled = true; 
    getTXStreamer();
  }
  else {
    if(!tx_enabled) return;
    else {
      // close out the tx streamer
      debugMsg("Closing streamer");
      tx_md.end_of_burst = true;
      tx_bits->send(zero_buf, 10, tx_md);
    }
    tx_enabled = false;
    if(tx_bits) {
      debugMsg("Deleting streamer");
      tx_bits->~tx_streamer();
    }
  }
}

void SoDa::USRPSNATX::getTXStreamer()
{
  if(tx_bits) {
    tx_bits->~tx_streamer(); 
  }
  tx_bits = usrp->get_tx_stream(*stream_args); 
}



void SoDa::USRPSNATX::execSetCommand(Command * cmd)
{
  switch(cmd->target) {
  case Command::TX_STATE:
    transmitSwitch(cmd->iparms[0] == 3);
    break; 
  default:
    break; 
  }
}

void SoDa::USRPSNATX::execGetCommand(Command * cmd)
{
  switch(cmd->target) {
  default:
    break; 
  }
}

void SoDa::USRPSNATX::execRepCommand(Command * cmd)
{
  switch(cmd->target) {
  default:
    break;
  }
}

