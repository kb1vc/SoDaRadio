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
#include "IPSockets.hxx"

#include <uhd/types/ranges.hpp>

namespace SoDa {

  class HamlibListener; 
  class HLCmdFunc {
  public:
    HLCmdFunc(const std::string _short_cmd, const std::string _long_get_cmd, const std::string _long_set_cmd) {
      short_cmd = _short_cmd; 
      long_get_cmd = _long_get_cmd;
      long_set_cmd = _long_set_cmd; 
    }
 
    virtual bool operator()(HamlibListener * hll, const std::string & cmd); 

  protected:
    bool isMine(const std::string & cmd); 
    std::string getStrArg(int argnum);
    double getDblArg(int argnum); 
    int getIntArg(int argnum); 

    std::string short_cmd; 
    std::string long_get_cmd;
    std::string long_set_cmd; 
  }; 

  class HLCSetFreq : public HLCmdFunc {
  public:
    HLCSetFreq() : HLCmdFunc("F", "get_freq", "set_freq") {}
  };

  class HamlibListener : public SoDaThread {
  public:
    HamlibListener(Params * params, 
		   uhd::freq_range_t & rx_range, 
		   uhd::freq_range_t & tx_range, 
		   CmdMBox * cmd_stream);
    ~HamlibListener();
    
    void run();

  protected:

    void cmdDumpState();

    // frequency ranges for tx and rx
    double rx_freq_min, rx_freq_max; 
    double tx_freq_min, tx_freq_max; 

    // the internal communications paths -- between the SoDa threads. 
    CmdMBox * cmd_stream;

    unsigned int cmd_subs;

    // these are the pieces of the posix message queue interface to the GUI or whatever.
    SoDa::IP::ServerSocket * server_socket;

    void execSetCommand(Command * cmd);
    void execGetCommand(Command * cmd);
    void execRepCommand(Command * cmd);
  }; 
}


#endif
