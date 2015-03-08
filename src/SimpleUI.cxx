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

#include "SimpleUI.hxx"


SoDa::SimpleUI::SimpleUI(Params * params,
	     CmdMBox * _cmd_stream) : SoDa::SoDaThread("SimpleUI")
{
  // connect to our message streams.
  cmd_stream = _cmd_stream; 

  // subscribe to them.
  cmd_subs = cmd_stream->subscribe();

  // create the network ports
  // This SimpleUI object is a server.
  server_socket = new SoDa::UD::ServerSocket(params->getServerSocketBasename() + "_cmd");

}


SoDa::SimpleUI::~SimpleUI()
{
  delete server_socket;
}

void SoDa::SimpleUI::run()
{
  SoDa::Command * net_cmd, * ring_cmd;

  net_cmd = NULL;
  ring_cmd = NULL;
  
  char buf[1024];

  unsigned int socket_read_count = 0;
  unsigned int socket_empty_count = 0;
  unsigned int iter_count = 0;
  bool new_connection = true; ;
  while(1) {
    iter_count++;
    bool didwork = false;
    bool got_new_netmsg = false; 
    // listen on the socket.

    if(server_socket->isReady()) {
      if(new_connection) {
	std::string vers= (boost::format("%s SVN %s") % PACKAGE_VERSION % SVN_VERSION).str(); 
	SoDa::Command * vers_cmd = new SoDa::Command(Command::REP,
						     Command::SDR_VERSION,
						     vers.c_str());
	server_socket->put(vers_cmd, sizeof(SoDa::Command));

	new_connection = false; 
      }
      
      if(net_cmd == NULL) {
	net_cmd = new SoDa::Command();
      }
      int stat = server_socket->get(net_cmd, sizeof(SoDa::Command));
      if(stat <= 0) {
	socket_empty_count++; 
      }
      else {
	socket_read_count++;
	got_new_netmsg = true; 
      }
    }
    else {
      new_connection = true; 
    }

    // if there are commands arriving from the socket port, handle them.
    if(got_new_netmsg) {
      debugMsg(boost::format("Got command [%s]\n") % net_cmd->toString());
      cmd_stream->put(net_cmd);
      didwork = true; 
      net_cmd = NULL; 
    }

    while((ring_cmd = cmd_stream->get(cmd_subs)) != NULL) {
      if(ring_cmd->cmd == SoDa::Command::REP) {
	server_socket->put(ring_cmd, sizeof(SoDa::Command));
      }
      execCommand(ring_cmd); 
      cmd_stream->free(ring_cmd);
      didwork = true; 
    }

    // if there is nothing to do, sleep for a little while.
    if(!didwork) usleep(100);
  }


  return; 
}



void SoDa::SimpleUI::execSetCommand(Command * cmd)
{
  return; 
}

void SoDa::SimpleUI::execGetCommand(Command * cmd)
{
  return;
}

void SoDa::SimpleUI::execRepCommand(Command * cmd)
{
  return;
}


