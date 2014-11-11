/*
Copyright (c) 2014, Matthew H. Reilly (kb1vc)
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

 ///
 ///  @file RigCtrl.hxx
 ///  @brief Thread class that listens on the Rig Control interface
 ///  It interprets a command stream that follows the rigctld (hamlib)
 ///  command set. 
 ///
 ///
 ///  @author M. H. Reilly (kb1vc)
 ///  @date   November 2014
 ///

#ifndef RIGCTRL_HDR
#define RIGCTRL_HDR
#include "SoDaBase.hxx"
#include "MultiMBox.hxx"
#include "Command.hxx"
#include "Params.hxx"
#include "RangeMap.hxx"
#include "IPSockets.hxx"
#include "CommandInterpreter.hxx"

namespace SoDa {
  ///  @class RigCtrl
  /// 
  ///  RigCtrl listens on the Command Stream message channel for
  ///  requests from other components (including the SoDa::UI listener)
  ///  and on a localhost socket for requests from a client using
  ///  the hamlib/rigctl protocol
  ///  http://manpages.ubuntu.com/manpages/natty/man8/rigctld.8.html
  ///
  ///  RigCtrl passes messages between the RigCTL socket to the command stream
  ///
  class RigCtrl : public SoDaThread {
  public:
    /// Constructor
    /// Build a RigCtrl thread
    /// @param params Pointer to a parameter object with all the initial settings
    /// and identification for the attached RIG
    /// @param _cmd_stream Pointer to the command stream message channel
    /// @param _portnum Port number that we'll listen on for commands localhost:portnum
    RigCtrl(Params * params, CmdMBox * _cmd_stream, unsigned int portnum = 4532); 

    /// start the thread and do the work
    void run();

  private:

    // Setup the command interpreter
    CommandInterpreter<RigCtrl> ci;

    /// Initialize the command interpreter -- register
    /// each of the action routines. 
    void initCommandInterp();

    /// These are the commands that get handled from the hamlib interface.
    
    /// we got a set freq command from the hamlib interface
    /// @param cmd_line vector of tokens from the command line. 
    bool setFreq(std::vector<std::string> & cmd_line);

    /// we got a get freq command from the hamlib interface
    /// @param cmd_line vector of tokens from the command line. 
    bool getFreq(std::vector<std::string> & cmd_line);
    
    static unsigned int net_buffer_length;
    char * network_buffer;
    char * net_buf_ptr;
    unsigned int net_buf_left;
    /// @brief get input strings from the network socket.
    /// @return true if we saw any characters on the port at all. 
    bool getNetCommands(); 
    
    /// @brief process the commands in the network buffer
    /// @return true if at least one command was processed. 
    bool processNetCommands(); 

    /// @brief process the commands on the local cmd_stream.
    /// @return true if at least one command was processed. 
    bool processBusCommands(); 
    
    Params * params; ///< parameters for the parent program

    CmdMBox * cmd_stream; ///< the SoDa command stream that we listen on.
    unsigned int cmd_subs; ///< the subscription to our SoDa command stream.
    
    IP::ServerSocket * server_socket; ///< hamlib/rigctl server socket
  };
}


#endif
