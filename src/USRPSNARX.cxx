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

#include "USRPSNARX.hxx"

#include <uhd/utils/safe_main.hpp>
#include <uhd/utils/thread_priority.hpp>
#include <uhd/usrp/multi_usrp.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

SoDa::USRPSNARX::USRPSNARX(Params * params, uhd::usrp::multi_usrp::sptr _usrp,
		     CmdMBox * _cmd_stream) : SoDa::SoDaThread("USRPSNARX")
{
  cmd_stream = _cmd_stream;

  usrp = _usrp; 
  
  // subscribe to the command stream.
  cmd_subs = cmd_stream->subscribe();

  // create the rx buffer streamers.
  uhd::stream_args_t stream_args("fc32", "sc16");
  std::vector<size_t> channel_nums;
  channel_nums.push_back(0);
  stream_args.channels = channel_nums;
  rx_bits = usrp->get_rx_stream(stream_args);

  usrp->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS);
  
  rx_sample_rate = params->getRXRate();
  rx_buffer_size = params->getRFBufferSize(); 

  sample_buf = new std::complex<float>[rx_buffer_size]; 

  scount = 0;

}


void SoDa::USRPSNARX::run()
{
  uhd::set_thread_priority_safe(); 
  // now do the event loop.  we watch
  // for commands and responses on the command stream.
  // and we watch for data in the input buffer. 

  bool exitflag = false;

  while(!exitflag) {
    Command * cmd = cmd_stream->get(cmd_subs);
    if(cmd != NULL) {
      //      std::cerr << "\n\nIn RX loop got command\n\n" << std::endl; 
      // process the command.
      execCommand(cmd);
      exitflag |= (cmd->target == Command::STOP); 
      cmd_stream->free(cmd); 
    }
    else {
      if(do_collection) {
	// collect an RX buffer
	unsigned int left = rx_buffer_size; 
	unsigned int coll_so_far; 
	uhd::rx_metadata_t md; 
	md.reset(); 
	md.has_time_spec = true; 
	while(left != 0) {
	  unsigned int got = rx_bits->recv(&(sample_buf[coll_so_far]), left, md);
	  coll_so_far += got; 
	  left -= got;
	}
	// is the collect time > end_time? 
	if(!md.has_time_spec) {
	  std::cerr << "Got a bad md -- " << md.to_pp_string(false) << std::endl;
	}
	if(md.time_spec >= slot_end_time) {
	  // stop collecting
	  startStream();

	  //     send the summary	  
	  double magsq = 0.0; 
	  for(int i = 0; i < ring_elements; i++) {
	    magsq += sample_magsq[i]; 
	  }
	  cmd_stream->put(new Command(Command::REP, Command::SNA_SCAN_REPORT, 
				      test_freq, magsq));
	}
	else {
	  //     accumulate the statistics
	  double msq = 0.0; 
	  for(int i = 0; i < rx_buffer_size; i++) {
	    double vr, vi; 
	    vr = sample_buf[i].real();
	    vi = sample_buf[i].imag();
	    msq = vr * vr + vi * vi; 
	  }
	  sample_magsq[ring_count] = msq; 
	  ring_count++; 
	  if(ring_count == ring_elements) ring_count = 0; 
	}
      }
    }
  }

  stopStream(); 
}


void SoDa::USRPSNARX::execCommand(Command * cmd)
{
  //  std::cerr << "In USRPSNARX execCommand" << std::endl;
  switch (cmd->cmd) {
  case Command::GET:
    execGetCommand(cmd); 
    break;
  case Command::SET:
    execSetCommand(cmd); 
    break; 
  case Command::REP:
    execRepCommand(cmd); 
    break;
  default:
    break; 
  }
}

void SoDa::USRPSNARX::startStream()
{
  //  std::cerr << "Starting RX Stream from USRP" << std::endl;
  usrp->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS, 0);
  ring_count = 0; 
  do_collection = true;
}

void SoDa::USRPSNARX::stopStream()
{
  //  std::cerr << "Stoping RX Stream from USRP" << std::endl; 
  usrp->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS, 0);
  do_collection = false; 
}

void SoDa::USRPSNARX::execSetCommand(Command * cmd)
{
  //  std::cerr << "In USRPSNARX execSetCommand" << std::endl;
  int i; 
  switch(cmd->target) {
  case Command::SNAI_SCAN_READY:
    // collect
    // zero out the collection buffer
    for(i = 0; i < ring_elements; i++) { sample_magsq[i] = 0.0; }
    // save the start, end, and freq
    slot_start_time = cmd->dparms[0];
    slot_end_time = cmd->dparms[1];
    test_freq = cmd->dparms[2];
    startStream();
    break; 
  default:
    break; 
  }
}

void SoDa::USRPSNARX::execGetCommand(Command * cmd)
{
}

void SoDa::USRPSNARX::execRepCommand(Command * cmd)
{
}

