/*
  Copyright (c) 2013, Matthew H. Reilly (kb1vc)
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

#include "RigCtrl.hxx"
#include <strings.h>

unsigned int SoDa::RigCtrl::net_buffer_length = 1024;

SoDa::RigCtrl::RigCtrl(Params * params, CmdMBox * _cmd_stream, unsigned int portnum)
  : SoDa::SoDaThread("RigCtrl")
{
  // connect to our message streams.
  cmd_stream = _cmd_stream; 

  // subscribe to them.
  cmd_subs = cmd_stream->subscribe();

  // create the network ports
  // This RigCtrl object is a server.
  server_socket = new SoDa::IP::ServerSocket(portnum, true);

  // now setup the net buffer
  network_buffer = new char[net_buffer_length + 1];
  net_buf_ptr = network_buffer;
  net_buf_left = net_buffer_length;

  initCommandInterp();   
}

void SoDa::RigCtrl::initCommandInterp()
{
  ci.setHandlerObj(this);
  ci.makeCommand(std::string("F"), std::string("set_freq"), &SoDa::RigCtrl::setFreq);
  ci.makeCommand(std::string("f"), std::string("get_freq"), &SoDa::RigCtrl::getFreq); 
}

bool SoDa::RigCtrl::getNetCommands()
{
  // this is a little complicated -- The network protocol won't necessarily
  // ensure that every packet is a complete command, or that there is 
  // only one command per incoming packet.  Parsing is up to us.
  // So, we keep a buffer of incoming data.  As each buffer comes in,
  // we scan it for '\n' to mark the end of a command.  Then we
  // put a pointer to each command on the current command queue
  unsigned long nbp = (unsigned long) net_buf_ptr; 
  debugMsg(boost::format("In getNetCommands -- net_buf_ptr = %lx net_buf_left = %d\n")
	   % nbp % net_buf_left);
  int stat = server_socket->readBuf(net_buf_ptr, net_buf_left);
  debugMsg(boost::format("readBuf returned stat = %d\n") % stat);
  if(stat > 0) {
    net_buf_left -= stat;
    net_buf_ptr += stat;
    *net_buf_ptr = '\000';
    return true; 
  }
  else {
    return false; 
  }
}

bool SoDa::RigCtrl::processBusCommands()
{
  SoDa::Command * ring_cmd;
  bool didwork = false; 
  while((ring_cmd = cmd_stream->get(cmd_subs)) != NULL) {
    didwork = true;
    execCommand(ring_cmd);
    cmd_stream->free(ring_cmd);
  }
  return didwork; 
}

bool SoDa::RigCtrl::processNetCommands()
{
  char * cmd_buf = network_buffer;
  char * cur_cmd = cmd_buf; 
  char * end_of_buf = network_buffer + net_buffer_length; 
  // scan through the network buffer, and parse
  // each complete command.

  bool flag = false; 
  debugMsg(boost::format("got command buffer [%s]\n") % network_buffer);
  for(char * bp = cmd_buf;
      bp <= end_of_buf; 
      bp++) {
    if((*bp == '\000') || (*bp == '\r') || (*bp == '\n')) {
      // end of command -- parse it.
      *bp = '\000';
      if(strlen(cur_cmd) != 0) {
	debugMsg(boost::format("about to parse [%s]\n") % cur_cmd);
	ci.parse(std::string(cur_cmd));
      }
      flag = true; 
      // now point to the next command
      cur_cmd = bp+1; 
    }
  }

  debugMsg("About to reconcile leftovers\n");
  // cur_cmd now points to the rump end of the command buffer.
  // there may be incomplete commands here. 
  // now move all the leftovers down to the
  // start of the buffer.
  net_buf_left = net_buffer_length;
  char * bufp; 
  for(bufp = network_buffer;
      cur_cmd <= end_of_buf;
      bufp++, cur_cmd++) {
    *bufp = *cur_cmd;
    net_buf_left--;
  }

  // now the buffer has whatever rump command is left in it.
  net_buf_ptr = bufp + 1;

  return flag; 
}

void SoDa::RigCtrl::run()
{
  std::cerr << boost::format("RigCtrl has started server_socket = %p.\n") % server_socket; 
  // do the actual work.
  while(1) {
    // did we do any work this time around? 
    bool didwork = false;

    if(server_socket->isReady()) {

      // look at the network interface -- get and process commands. 
      if(getNetCommands()) {
	didwork = processNetCommands();
      }

      // now look at the command queue -- send out commands.
      didwork |= processBusCommands();
    }

    if(!didwork) usleep(100000);
  }
}


/// we got a set freq command from the hamlib interface
/// @param cmd_line vector of tokens from the command line. 
bool SoDa::RigCtrl::setFreq(std::vector<std::string> & cmd_line) {
  std::cerr << boost::format("Got setFreq [%s]\n") % cmd_line[0]; 
}

/// we got a get freq command from the hamlib interface
/// @param cmd_line vector of tokens from the command line. 
bool SoDa::RigCtrl::getFreq(std::vector<std::string> & cmd_line) {
  std::cerr << boost::format("Got getFreq [%s]\n") % cmd_line[0]; 
}
