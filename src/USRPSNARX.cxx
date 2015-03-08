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
			   CmdMBox * _cmd_stream, double rx_offset) : SoDa::SoDaThread("USRPSNARX")
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

  do_collection = false;

  // DC blocking filter -- removes dc offset
  // 0.99 has a 1kHz + bw
  // 0.9 has a 10kHz bw?
  dc_block = new SoDa::DCBlock<std::complex<float>, float>(0.9);

  // filter edges
  float low_cut, low_edge, hi_edge, hi_cut; 
  low_cut = rx_offset * 0.85;
  low_edge = rx_offset * 0.95;
  hi_edge = rx_offset * 1.05;    
  hi_cut = rx_offset * 1.15;
  debugMsg(boost::format("Creating offset filter at %g %g %g %g\n")
	   % low_cut % low_edge % hi_edge % hi_cut);
  offset_filter = new SoDa::OSFilter(low_cut, low_edge, hi_edge, hi_cut, 512, 1.0, rx_sample_rate, rx_buffer_size);

}


void SoDa::USRPSNARX::run()
{
  uhd::set_thread_priority_safe(); 
  // now do the event loop.  we watch
  // for commands and responses on the command stream.
  // and we watch for data in the input buffer. 

  debugMsg("in Run method.");

  bool exitflag = false;

  while(!exitflag) {
    Command * cmd = cmd_stream->get(cmd_subs);
    if(cmd != NULL) {
      debugMsg(boost::format("Got command [%s]\n") % cmd->toString());
      execCommand(cmd);
      exitflag |= (cmd->target == Command::STOP); 
      cmd_stream->free(cmd); 
    }
    else {
      if(do_collection) {
	// collect an RX buffer
	unsigned int left = rx_buffer_size; 
	unsigned int coll_so_far = 0; 
	uhd::rx_metadata_t md; 
	md.reset(); 
	md.has_time_spec = true; 
	while(left != 0) {
	  int got = rx_bits->recv(&(sample_buf[coll_so_far]), left, md);
	  if(got <= 0) {
	    std::cerr << boost::format("USRPSNARX got recv return of %d error code %s\n") % got % md.to_pp_string();
	  }
	  coll_so_far += got; 
	  left -= got;
	}

	bool accumulate = false; 

	// is the collect time > end_time? 
	if(!md.has_time_spec) {
	  std::cerr << "Got a bad md -- " << md.to_pp_string(false) << std::endl;
	}
	if(md.time_spec >= slot_end_time) {
	  //     send the summary	  
	  double magsq = 0.0; 
	  double min_magsq = 1e20;
	  double max_magsq = 0.0;
	  int lim = (ring_count > SNARX_MAX_RING_SAMPS) ? SNARX_MAX_RING_SAMPS : ring_count;
	  for(int i = 0; i < lim; i++) {
	    if(sample_magsq[i] > max_magsq) max_magsq = sample_magsq[i];
	    if(sample_magsq[i] < min_magsq) min_magsq = sample_magsq[i];	    
	    magsq += sample_magsq[i]; 
	  }
	  if(0 && ((max_magsq / min_magsq) > 91.2)) {
	    int jj; 
	    for(jj = 0; jj < rx_buffer_size; jj++) {
	      float r, im, mag; 
	      r = sample_buf[jj].real();
	      im = sample_buf[jj].imag();
	      mag = sqrt(r*r + im*im);
	      debugMsg(boost::format("RXB %g %g %g\n")
		       % sample_buf[jj].real() % sample_buf[jj].imag()
		       % mag); 
	    }
	    exit(-1);
	  }
	  if((max_magsq / min_magsq) < 1.2) {
	    // stop collecting
	    stopStream();
	    
	    magsq = magsq / ((double) lim);
	    cmd_stream->put(new Command(Command::REP, Command::SNA_SCAN_REPORT, 
				      test_freq, magsq));
	  }
	  else {
	    // we haven't settled yet -- keep going. 
	    debugMsg(boost::format("nonconverged freq %g  min,max [%g,%g] ratio %g\n") 
		     % test_freq % min_magsq % max_magsq % (max_magsq / min_magsq)); 
	    accumulate = true; 
	  }
	}
	else {
	  accumulate = true; 
	}

	if(accumulate) {
	  //     accumulate the statistics
	  double msq = 0.0; 
	  // first remove DC component
	  dc_block->apply(sample_buf, rx_buffer_size);
	  // filter around offset freq
	  // offset_filter->apply(sample_buf, sample_buf, 1.0);
	  for(int i = 0; i < rx_buffer_size; i++) {
	    double vr, vi; 
	    vr = sample_buf[i].real();
	    vi = sample_buf[i].imag();
	    msq = vr * vr + vi * vi; 
	  }

	  debugMsg(boost::format("Got %d samples ring_count = %d magsq = %g\n")
		   % rx_buffer_size % ring_count % msq);
	  
	  sample_magsq[ring_count % SNARX_MAX_RING_SAMPS] = msq; 
	  ring_count++;
	}
      }
    }
  }

  debugMsg("Exit run method");
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
  debugMsg("Starting RX Stream from USRP");
  usrp->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS, 0);
  debugMsg("Started RX Stream from USRP");  
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
  debugMsg(boost::format("got command [%s]\n") % cmd->toString());
  int i; 
  switch(cmd->target) {
  case Command::SNAI_SCAN_READY:
    // collect
    // zero out the collection buffer
    for(i = 0; i < SNARX_MAX_RING_SAMPS; i++) { sample_magsq[i] = 0.0; }
    // save the start, end, and freq
    slot_start_time = cmd->dparms[0];
    slot_end_time = cmd->dparms[1];
    test_freq = cmd->dparms[2];
    debugMsg(boost::format("SNAI_SCAN_READY start = %g  end = %g  freq = %g\n") % slot_start_time % slot_end_time % test_freq);
    startStream();
    debugMsg("Started Stream");
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

