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

#include "HamlibListener.hxx"
#include "version.h"
#include <hamlib/rig.h>
#include <boost/format.hpp>
SoDa::HamlibListener::HamlibListener(Params * params, 
				     uhd::freq_range_t & rx_range, 
				     uhd::freq_range_t & tx_range, 
				     CmdMBox * _cmd_stream) : SoDa::SoDaThread("HamlibListener")
{
  // connect to our message streams.
  cmd_stream = _cmd_stream; 

  // subscribe to them.
  cmd_subs = cmd_stream->subscribe();

  rx_freq_min = rx_range.start();
  if(rx_freq_min < 0.0) rx_freq_min = 1.0; 
  rx_freq_max = rx_range.stop();
  tx_freq_min = tx_range.start();
  if(tx_freq_min < 0.0) tx_freq_min = 1.0;   
  tx_freq_max = tx_range.stop();
  // create the network ports
  // This UI object is a server.
  server_socket = new SoDa::IP::ServerSocket(5901);
}


SoDa::HamlibListener::~HamlibListener()
{
  delete server_socket;
}

void SoDa::HamlibListener::run()
{
  SoDa::Command * net_cmd, * ring_cmd;

  net_cmd = NULL;
  ring_cmd = NULL;
  
  char buf[1024];


  unsigned int socket_read_count = 0;
  unsigned int socket_empty_count = 0;
  unsigned int iter_count = 0;
  bool new_connection = true; ;

  int max_buf_size = 1024;
  char * net_msg_buf = new char[max_buf_size];
  int cur_msg_buf_idx = 0; 
  while(1) {
    iter_count++;
    bool didwork = false;
    bool got_new_netmsg = false; 
    // listen on the socket.

    if(server_socket->isReady()) {
      if(new_connection) {
	new_connection = false; 
      }
      
      if(net_cmd == NULL) {
	net_cmd = new SoDa::Command();
      }
      int bufsize = max_buf_size - cur_msg_buf_idx; 
      int stat = server_socket->getRaw(&(net_msg_buf[cur_msg_buf_idx], bufsize, 100); // 100 uSec timeout
      if(stat <= 0) {
	socket_empty_count++; 
      }
      else {
	cur_msg_buf_idx += stat;
	// scan the buffer until we find a \n
	// then process the command
	// and shift the remaining bytes to the start of the buffer.
	asdf
	if(net_msg_buf[cur_msg_buf_idx - 1] == "\n") {
	  socket_read_count++;
	  std::cerr << boost::format("Hamlib: [%s]\n") % net_msg_buf; 
	  if (std::string(net_msg_buf) == std::string("\\dump_state")) {
	    cmdDumpState();
	  }
	  got_new_netmsg = true; 
	  cur_msg_buf_idx = 0; 
	}
      }
    }
    else {
      new_connection = true; 
    }

    SoDa::Command * ring_cmd; 
    while((ring_cmd = cmd_stream->get(cmd_subs)) != NULL) {
      if(ring_cmd->target == SoDa::Command::STOP) {
	return; 
      }
    }
    // if there are commands arriving from the socket port, handle them.
    // if there is nothing to do, sleep for a little while.
    if(!didwork) usleep(10000);
  }


  return; 
}

void SoDa::HamlibListener::cmdDumpState()
{
  std::string resp; 

  resp = "0\n"; // protocol version
  resp += "1 \n"; // seems to be ignored...
  resp += "2 \n"; // ITU region



  // rmode_t vfo_t ant_t

  // now the frequency ranges
  // RX
  resp += (boost::format("%15f %15f 0x%x %d %d 0x%x 0x%x\n")
	   % rx_freq_min
	   % rx_freq_max
	   % (RIG_MODE_AM | RIG_MODE_CW | RIG_MODE_USB | RIG_MODE_LSB | RIG_MODE_FM | RIG_MODE_WFM | RIG_MODE_CWR )
	   % -1  // this is rx  low_power
	   % -1  // this is rx high_power
	   % (RIG_VFO_A | RIG_VFO_B) // vfo_mask
	   % (RIG_ANT_1 | RIG_ANT_2)
	   ).str();
  resp += "0 0 0 0 0 0 0\n";

  // TX
  resp += (boost::format("%15f %15f 0x%x %d %d 0x%x 0x%x\n")
	   % tx_freq_min
	   % tx_freq_max
	   % (RIG_MODE_AM | RIG_MODE_CW | RIG_MODE_USB | RIG_MODE_LSB | RIG_MODE_FM | RIG_MODE_WFM | RIG_MODE_CWR )
	   % 1  // this is TX 1 mW
	   % 200  // this is TX 200 mW
	   % (RIG_VFO_A | RIG_VFO_B) // vfo_mask
	   % RIG_ANT_1
	   ).str();

  resp += "0 0 0 0 0 0 0\n";

  // now tuning steps
  resp += (boost::format("0x%x %d\n")
	   % (RIG_MODE_CW | RIG_MODE_USB | RIG_MODE_LSB | RIG_MODE_CWR )
	   % 1
	   ).str();
  resp += (boost::format("0x%x %d\n")
	   % (RIG_MODE_AM | RIG_MODE_FM | RIG_MODE_WFM)
	   % 100
	   ).str();
  resp += "0 0\n";

  // now filters
  resp += (boost::format("0x%x %d\n") 
	   % (RIG_MODE_CW | RIG_MODE_USB | RIG_MODE_LSB | RIG_MODE_CWR )  
	   % 100).str(); 
  resp += (boost::format("0x%x %d\n") 
	   % (RIG_MODE_CW | RIG_MODE_USB | RIG_MODE_LSB | RIG_MODE_CWR )  
	   % 500).str(); 
  resp += (boost::format("0x%x %d\n") 
	   % (RIG_MODE_AM | RIG_MODE_CW | RIG_MODE_USB | RIG_MODE_LSB | RIG_MODE_FM | RIG_MODE_CWR )  
	   % 2000).str(); 
  resp += (boost::format("0x%x %d\n") 
	   % (RIG_MODE_AM | RIG_MODE_CW | RIG_MODE_USB | RIG_MODE_LSB | RIG_MODE_FM | RIG_MODE_WFM | RIG_MODE_CWR )  
	   % 6000).str(); 

  resp += "0 0\n";

  // max RIT
  // mast XIT
  // max IF shift
  resp += "1000\n1000\n1000\n";

  // we don't have a speech synthesizer to announce frequencies
  resp += "0\n";

  // preamp list
  resp += "0\n";

  // attenuator list
  resp += "5 10 15 20 25 30 35\n";

  // has_get_func
  resp += (boost::format("0x%x\n") 
	   % (RIG_FUNC_NONE)).str();
  // has_set_func
  resp += (boost::format("0x%x\n") 
	   % (RIG_FUNC_NONE)).str();
  // has get level
  resp += (boost::format("0x%x\n") 
	   % (RIG_LEVEL_NONE)).str();
  // has set level
  resp += (boost::format("0x%x\n") 
	   % (RIG_LEVEL_NONE)).str();

  // has get_param
  resp += (boost::format("0x%x\n") 
	   % (RIG_PARM_NONE)).str();

  // has set param
  resp += (boost::format("0x%x\n") 
	   % (RIG_PARM_NONE)).str();


  std::cerr << boost::format("hamlib dumping [%s]\n") % resp; 
  server_socket->put(resp.c_str(), resp.length());

}


void SoDa::HamlibListener::execSetCommand(Command * cmd)
{
}

void SoDa::HamlibListener::execGetCommand(Command * cmd)
{
}

void SoDa::HamlibListener::execRepCommand(Command * cmd)
{
}

