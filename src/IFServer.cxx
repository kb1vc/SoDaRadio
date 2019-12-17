/*
Copyright (c) 2019 Matthew H. Reilly (kb1vc)
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
#include "IFServer.hxx"
#include <boost/format.hpp>
#include <iostream>
#include <string>

// namespace doesn't matter here... let's do without.

/**
 * To make an accessory thread that gets loaded and connected automagically, 
 * just create a normal thread object.  Like this: 
 */
IFServer if_server(std::string("IF_Network_Server"));


IFServer::IFServer(const std::string & name) : SoDa::Thread(name) {
  // create the socket
  server_socket = new SoDa::UD::ServerSocket("IFServer");

  current_rx_center_freq = 0.0; 
}


void IFServer::run() {
  bool exitflag = false; 

  SoDa::Buf * rxbuf; 

  int loop_count = 0; 
  int sent_count = 0; 

  while(!exitflag) {
    // look for a command
    auto cmd = cmd_stream->get(cmd_subs); 
    while (cmd != NULL) {
      exitflag |= (cmd->target == SoDa::Command::STOP);      
      execCommand(cmd);
      // is it a frequency report? 

      cmd_stream->free(cmd);
      cmd = cmd_stream->get(cmd_subs); 
    }


    bool did_work = false; 
    int len; 
    // do we have an IF buffer?
    if((rxbuf = rx_stream->get(rx_subs)) != NULL) {
      did_work = true; 
      // now send the rx buffer. 
      if(sendBuffer(rxbuf)) sent_count++; 
      len = rxbuf->getComplexLen();
      // free up the buffer. 
      rx_stream->free(rxbuf);
    }

    if(!did_work) {
      usleep(10000);
    }

    loop_count++; 
    if((loop_count % 0x3ff) == 0) {
      std::cerr << loop_count << "  " << sent_count << " len = " << len << std::endl;
      
    }
  }
}

bool IFServer::sendBuffer(SoDa::Buf * rxbuf) {
  if(server_socket->isReady()) {  
    // package up a message.
    unsigned int buffer_length = rxbuf->getComplexLen() * sizeof(std::complex<float>);

    // dangerous hack... can't figure out how else to do it. 
    int message_size = sizeof(unsigned long) + sizeof(double) + buffer_length;
    char message[message_size];
    auto blen_p = reinterpret_cast<unsigned long *>(message);
    auto freq_p = reinterpret_cast<double *>(message + sizeof(unsigned long));

    char * cbuf = message + sizeof(unsigned long) + sizeof(double);

    *blen_p = buffer_length;
    *freq_p = current_rx_center_freq;
    
    // copy the data
    memcpy(cbuf, rxbuf->getComplexBuf(), buffer_length);

    server_socket->put(message, message_size);

    return true;
  }
  return false; 
}

void IFServer::shutDown() {
  // close the socket
  delete server_socket; 
}

void IFServer::subscribeToMailBox(const std::string & mbox_name, SoDa::BaseMBox * mbox_p)
{
  if(SoDa::connectMailBox<SoDa::CmdMBox>(this, cmd_stream, "CMD", mbox_name, mbox_p)) {
    cmd_subs = cmd_stream->subscribe();
  }
  if(SoDa::connectMailBox<SoDa::DatMBox>(this, rx_stream, "RX", mbox_name, mbox_p)) {
    rx_subs = rx_stream->subscribe();
  }
}

void IFServer::execRepCommand(SoDa::Command * cmd)
{
  switch (cmd->target) {
  case SoDa::Command::RX_FE_FREQ:
    current_rx_center_freq = cmd->dparms[0];
    std::cerr << boost::format("Set new rx center freq = %g\n") % current_rx_center_freq;     
    break;
  }
}
