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

#ifndef SimpleUI_HDR
#define SimpleUI_HDR
#include "SoDaBase.hxx"
#include "MultiMBox.hxx"
#include "Command.hxx"
#include "Params.hxx"
#include "UDSockets.hxx"

namespace SoDa {
  /**
   * SimpleUI -- operates as a basic command relay from a UD socket to/from a command stream.
   */
  class SimpleUI : public SoDaThread {
  public:
    SimpleUI(Params * params, 
	  CmdMBox * cmd_stream);
    ~SimpleUI();
    
    void run();

  protected:
    // the internal communications paths -- between the SoDa threads. 
    CmdMBox * cmd_stream;
    unsigned int cmd_subs;

    // these are the pieces of the posix message queue interface to the GUI or whatever.
    SoDa::UD::ServerSocket * server_socket;

    void execSetCommand(Command * cmd);
    void execGetCommand(Command * cmd);
    void execRepCommand(Command * cmd);

  }; 
}


#endif
