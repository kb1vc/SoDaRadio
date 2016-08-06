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
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

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


  // setup all the commands
  registerCommand("", "\\dump_state", &HamlibListener::cmdDumpState);
  registerCommand("v", "get_vfo", &HamlibListener::cmdVFO);
  registerCommand("V", "set_vfo", &HamlibListener::cmdVFO);  
  current_VFO = std::string("Main");
  registerCommand("f", "get_freq", &HamlibListener::cmdFreq);
  registerCommand("F", "set_freq", &HamlibListener::cmdFreq);  
  registerCommand("m", "get_mode", &HamlibListener::cmdMode);
  registerCommand("M", "set_mode", &HamlibListener::cmdMode);  
  registerCommand("t", "get_ptt", &HamlibListener::cmdPTT);
  registerCommand("T", "set_ptt", &HamlibListener::cmdPTT);  

  // setup the mode map
  soda2hl_modmap[SoDa::Command::LSB] = std::string("LSB");
  soda2hl_modmap[SoDa::Command::USB] = std::string("USB");
  soda2hl_modmap[SoDa::Command::AM] = std::string("AMS");
  soda2hl_modmap[SoDa::Command::NBFM] = std::string("FM");
  soda2hl_modmap[SoDa::Command::WBFM] = std::string("FMS");
  soda2hl_modmap[SoDa::Command::CW_U] = std::string("CW");
  soda2hl_modmap[SoDa::Command::CW_L] = std::string("CWR");  
  
  hl2soda_modmap[std::string("LSB")] = SoDa::Command::LSB;
  hl2soda_modmap[std::string("USB")] = SoDa::Command::USB;
  hl2soda_modmap[std::string("AMS")] = SoDa::Command::AM;
  hl2soda_modmap[std::string("FM")] = SoDa::Command::NBFM;
  hl2soda_modmap[std::string("CW")] = SoDa::Command::CW_U;
  hl2soda_modmap[std::string("CWR")]  = SoDa::Command::CW_L;

  // create the network ports
  // This UI object is a server.
  server_socket = new SoDa::IP::LineServerSocket(5900);
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
  while(1) {
    iter_count++;
    bool did_work = false;

    // listen on the socket.
    if(server_socket->isReady()) {
      if(new_connection) {
	new_connection = false;
	std::cerr << "HamlibListener got connection.\n"; 
      }
      
      int stat = server_socket->getLine(net_msg_buf, max_buf_size);
      if(stat < 0) {
	new_connection = true;
	std::cerr << "HamlibListener broke connection.\n"; 	
      }
      else if(stat > 0) {
	handleCommand(net_msg_buf); 
	did_work = true; 
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
      std::cerr << boost::format("HamlibListener got ring command : [%s]\n") % ring_cmd->toString();
      execCommand(ring_cmd);
    }
    // if there are commands arriving from the socket port, handle them.
    // if there is nothing to do, sleep for a little while.
    if(!did_work) usleep(10000);
  }


  return; 
}

bool SoDa::HamlibListener::cmdDumpState(const std::vector<std::string> cmdvec)
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
  sendResponse(resp); 

  return true;
}


void SoDa::HamlibListener::execSetCommand(Command * cmd)
{
  switch (cmd->target) {
  case SoDa::Command::RX_MODE:
  case SoDa::Command::TX_MODE:    
    mod_type = SoDa::Command::ModulationType(cmd->iparms[0]);
    std::cerr << boost::format("HamlibListener: got new mode %d type = [%s]\n") 
      % cmd->iparms[0] % soda2hl_modmap[mod_type]; 
    break; 
  }
}

void SoDa::HamlibListener::execGetCommand(Command * cmd)
{
}

void SoDa::HamlibListener::execRepCommand(Command * cmd)
{
  switch (cmd->target) {
  case SoDa::Command::RX_TUNE_FREQ:
    rx_freq = cmd->dparms[0]; 
    break; 
  case SoDa::Command::TX_TUNE_FREQ:
    tx_freq = cmd->dparms[0]; 
    break; 
  case SoDa::Command::TX_STATE:
    ptt_state = (cmd->iparms[0] & 2) != 0; 
    break; 
  }
}


void SoDa::HamlibListener::registerCommand(const std::string & short_cmd, 
					   const std::string & long_cmd, 
					   bool(HamlibListener::*fptr)(const std::vector<std::string>))
{
  // store a pointer to the command handler for each type. 
  if(short_cmd.length() != 0) {
    command_map[short_cmd] = fptr; 
  }
  if(long_cmd.length() != 0) {
    command_map[long_cmd] = fptr; 
  }
}

void SoDa::HamlibListener::handleCommand(const std::string & cmd_buf)
{
  // find first token -- use it as the key
  std::vector<std::string> cmdvec; 
  cmdvec = boost::split(cmdvec, cmd_buf, boost::is_any_of(" \t"), boost::token_compress_on);
  if(command_map.find(cmdvec[0]) != command_map.end()) {
    (this->*command_map[cmdvec[0]])(cmdvec);
  }
  else {
    std::cerr << boost::format("Could not handle command buffer [%s] (cmd = [%s])\n") % cmd_buf % cmdvec[0];
    std::string erresp = (boost::format("%s %d\n") % NETRIGCTL_RET % RIG_ENAVAIL).str();
    sendResponse(erresp);
  }
}
