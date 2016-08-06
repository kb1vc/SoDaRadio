/*
Copyright (c) 2016, Matthew H. Reilly (kb1vc)
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

#ifndef HAMLIB_LISTENER_HDR
#define HAMLIB_LISTENER_HDR
#include "SoDaBase.hxx"
#include "MultiMBox.hxx"
#include "Command.hxx"
#include "Params.hxx"
#include "UI.hxx"
#include "LineSocket.hxx"

#include <boost/foreach.hpp>

#include <uhd/types/ranges.hpp>
#include <map>
#include <vector>

#include <hamlib/rig.h>

namespace SoDa {

  class HamlibListener; 


  class HamlibListener : public SoDaThread {
  public:
    HamlibListener(Params * params, 
		   uhd::freq_range_t & rx_range, 
		   uhd::freq_range_t & tx_range, 
		   CmdMBox * cmd_stream);
    ~HamlibListener();
    
    void run();

    void sendResponse(std::string resp) {
      server_socket->putRaw(resp.c_str(), resp.length()); 
    }
  protected:    

    bool cmdDumpState(const std::vector<std::string> cmdvec);

    bool cmdVFO(const std::vector<std::string> cmdvec) { 
      if((cmdvec[0] == std::string("v")) || (cmdvec[0] == std::string("get_vfo"))) {
	sendResponse(current_VFO + "\n");
      }
      else if((cmdvec[0] == std::string("V")) || (cmdvec[0] == std::string("set_vfo"))) {
	current_VFO = cmdvec[1];
	sendResponse("RPRT 0\n");
      }
    }

    bool cmdFreq(const std::vector<std::string> cmdvec) { 
      if((cmdvec[0] == std::string("f")) || (cmdvec[0] == std::string("get_freq"))) {
	std::string resp;
	if(current_VFO == std::string("TX")) {
	  resp = (boost::format("%12.9g\n") % tx_freq).str();
	}
	else {
	  resp = (boost::format("%12.9g\n") % rx_freq).str();	  
	}
	sendResponse(resp); 	
      }
      else if((cmdvec[0] == std::string("F")) || (cmdvec[0] == std::string("set_freq"))) {
	
	double setfreq = boost::lexical_cast<double>(cmdvec[1]);
	if((current_VFO == std::string("TX")) || (current_VFO == std::string("Main"))) {
	  if((setfreq < tx_freq_min) || (setfreq > tx_freq_max)) {
	    sendResponse((boost::format("RPRT %d\n") % RIG_EINVAL).str());
	  }
	  else {
	    SoDa::Command *ncmd = new SoDa::Command(Command::SET, Command::TX_FE_FREQ, setfreq);
	    std::cerr << boost::format("Hamlib sending tx freq cmd freq=%12g [%s]\n") % setfreq % ncmd->toString();
	    cmd_stream->put(ncmd);
	  }
	}
	if((current_VFO == std::string("RX")) || (current_VFO == std::string("Main"))) {
	  if((setfreq < rx_freq_min) || (setfreq > rx_freq_max)) {
	    sendResponse((boost::format("RPRT %d\n") % RIG_EINVAL).str());
	  }
	  else {
	    SoDa::Command *ncmd = new SoDa::Command(Command::SET, Command::RX_FE_FREQ, setfreq);	    
	    std::cerr << boost::format("Hamlib sending tx freq cmd freq=%12g [%s]\n") % setfreq % ncmd->toString();	    
	    cmd_stream->put(ncmd);	    
	  }
	}
	sendResponse("RPRT 0\n");
      }
      return true; 
    }

    bool cmdPTT(const std::vector<std::string> cmdvec) { 
      if((cmdvec[0] == std::string("t")) || (cmdvec[0] == std::string("get_ptt"))) {
	sendResponse(ptt_state ? "1\n" : "0\n");
      }
      else if((cmdvec[0] == std::string("T")) || (cmdvec[0] == std::string("set_ptt"))) {
	int ptt = (cmdvec[1] == std::string("0")) ? 0 : 2;
	cmd_stream->put(new SoDa::Command(Command::SET, Command::TX_STATE, ptt));
	sendResponse("RPRT 0\n");	
      }
      return true; 
    }

    bool cmdMode(const std::vector<std::string> cmdvec) {
      if((cmdvec[0] == std::string("m")) || (cmdvec[0] == std::string("get_mode"))) {
	std::string res = (boost::format("%s\n2000\n") % soda2hl_modmap[mod_type]).str();
	sendResponse(res);
      }
      else if((cmdvec[0] == std::string("M")) || (cmdvec[0] == std::string("set_mode"))) {
	std::cerr << "\n\n\n\n\n\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";       	
	std::cerr << boost::format("HamlibListener got setmode [%s]\n") % cmdvec[1]; 
	std::cerr << "\n\n\n\n\n\n\n"; 
	if(cmdvec[1] == std::string("?")) {
	  std::string res; 
	  std::string delim = ""; 
	  BOOST_FOREACH( const M2SMap::value_type & me, soda2hl_modmap) {
	    res += delim + me.second;
	    delim = " "; 
	  }
	  res += "\n";
	  sendResponse(res);
	  sendResponse("RPRT 0\n");		  
	}
	else if(hl2soda_modmap.find(cmdvec[1]) != hl2soda_modmap.end()) {
	  std::cerr << boost::format("\n\n\n\nHamlibListener: SET MODE to [%s]\n\n\n\n") % cmdvec[1]; 	
	  std::cerr << "\n\n\n\n\n\n\n"; 
	  SoDa::Command::ModulationType mod = hl2soda_modmap[cmdvec[1]];
	  cmd_stream->put(new SoDa::Command(Command::SET, Command::RX_MODE, mod));
	  cmd_stream->put(new SoDa::Command(Command::SET, Command::TX_MODE, mod));	  
	  sendResponse("RPRT 0\n");	
	}
	else {
	  sendResponse((boost::format("RPRT %d\n") % RIG_EINVAL).str());	  
	}
      }
      else {
	std::cerr << "#####################################################################\n";
	  sendResponse("RPRT 4\n");		
      }
      return true; 
    }

    void registerCommand(const std::string & short_cmd, 
			 const std::string & long_cmd, 
			 bool(HamlibListener::*fptr)(const std::vector<std::string>));
    void handleCommand(const std::string & cmd_buf);


    // command list. 
    std::map<std::string, bool(HamlibListener::*)(const std::vector<std::string>) > command_map; 

    // frequency ranges for tx and rx
    double rx_freq_min, rx_freq_max; 
    double tx_freq_min, tx_freq_max; 

    // the internal communications paths -- between the SoDa threads. 
    CmdMBox * cmd_stream;

    unsigned int cmd_subs;

    // current state of the radio:
    double rx_freq, tx_freq; 
    std::string current_VFO;
    bool ptt_state; 
    typedef std::map<SoDa::Command::ModulationType, std::string> M2SMap;
    M2SMap soda2hl_modmap;
    std::map<std::string, SoDa::Command::ModulationType> hl2soda_modmap;     
    SoDa::Command::ModulationType mod_type; 
    
    
    
    // these are the pieces of the posix message queue interface to the GUI or whatever.
    SoDa::IP::LineServerSocket * server_socket;

    void execSetCommand(Command * cmd);
    void execGetCommand(Command * cmd);
    void execRepCommand(Command * cmd);
  }; 
}


#endif
