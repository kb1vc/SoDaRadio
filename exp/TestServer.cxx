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
#include <uhd/usrp/multi_usrp.hpp>

#include "SoDaBase.hxx"
#include "MultiMBox.hxx"

// the radio parts. 
#include "Params.hxx"
#include "UI.hxx"

namespace SoDa {
  class TestServer : public SoDaThread {
  public:
    TestServer(CmdMBox * _cwtxt_stream, CmdMBox * _cmd_stream) : SoDa::SoDaThread("TestServer"){
      cwtxt_stream = _cwtxt_stream;
      cmd_stream = _cmd_stream;

      cwtxt_subs = cwtxt_stream->subscribe();
      cmd_subs = cmd_stream->subscribe();
      
    }

    void run() {
      bool exitflag = false;
      Command * cmd; 
      while(!exitflag) {
	if((cmd = cmd_stream->get(cmd_subs)) != NULL) {
	  std::cerr << "CMD STREAM: [" << cmd->toString() << "]" << std::endl;
	  exitflag |= (cmd->target == Command::STOP);
	  cmd_stream->free(cmd); 
	}
	if((cmd = cwtxt_stream->get(cwtxt_subs)) != NULL) {
	  std::cerr << "CW TXT STREAM: [" << cmd->toString() << "]" << std::endl;
	  exitflag |= (cmd->target == Command::STOP);
	  cmd_stream->free(cmd); 
  	}
	usleep(10000); 
      }
    }
    
  private:
    CmdMBox * cwtxt_stream, * cmd_stream;
    int cwtxt_subs, cmd_subs; 
  };
}


#include "Command.hxx"

int doWork(int argc, char * argv[])
{
  // create the components of a fake radio
  // that can sink commands from the IP socket
  // interface. 

  SoDa::Params params(argc, argv);

  // These are the mailboxes that connect
  // the various widgets
  // the rx and tx streams are vectors of complex floats.
  // we don't declare the extent here, as it will be set
  // by a negotiation.  
  SoDa::DatMBox rx_stream, tx_stream, cw_env_stream, if_stream;
  SoDa::CmdMBox cmd_stream(false);
  SoDa::CmdMBox cwtxt_stream(false);
  SoDa::CmdMBox gps_stream(false);

  // The UI
  SoDa::UI ui(&params, &cwtxt_stream, &rx_stream, &if_stream,
	      &cmd_stream, &gps_stream);

  // The test unit
  SoDa::TestServer test_server(&cmd_stream, &cwtxt_stream);
  
  // start the UI -- configure stuff and all that. 
  ui.start();

  // start command consumers first.
  test_server.start();

  ui.join();
  test_server.join(); 
  // while(1) {
  //   usleep(1000000); 
  // }
  
  // when we get here, we are done... (UI should not return until it gets an "exit/quit" command.)
  return 0; 
}

int main(int argc, char * argv[])
{
  try {
    return doWork(argc, argv); 
  }
  catch (SoDa::SoDaException * exc) {
    std::cerr << "Exception caught at SoDa main: " << std::endl;
    std::cerr << "\t" << exc->toString() << std::endl; 
  }
}
