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
 * just create a normal thread object as shown below. 
 * 
 * The code below creates an instance of the IFServer class when the shareable
 * library (plugin) is loaded by the SoDaRadio server process. IFServer is a subclass of
 * SoDa::Thread.  All instances of SoDa::Thread will register with a singleton
 * object called the ThreadRegistry.  
 * 
 * The server interacts with the plugin via the registry. 
 */
IFServer if_server(std::string("IF_Network_Server"));


IFServer::IFServer(const std::string & name) : SoDa::Thread(name) {
  // create the socket
  server_socket = new SoDa::UD::ServerSocket("IFServer");

  // we'll start with a center frequency of 0.  
  current_rx_center_freq = 0.0; 
}

/**
 * This is the "do work" function.  It does not return until a 
 * STOP command is received in the command mailbox. 
 */
void IFServer::run() {
  bool exitflag = false; 

  // This will point to an incoming buffer of samples from the IF mailbox. 
  SoDa::Buf * rxbuf; 

  int sent_count = 0; 

  // run until we get a STOP command
  while(!exitflag) {
    // look for a command
    auto cmd = cmd_stream->get(cmd_subs); 
    while (cmd != NULL) {
      exitflag |= (cmd->target == SoDa::Command::STOP);

      // This function is provided by SoDa::Thread.  It will call
      // our execRepCommand function if the incoming message is a REPORT. 
      execCommand(cmd);

      cmd_stream->free(cmd);
      cmd = cmd_stream->get(cmd_subs); 
    }

    // it is a good idea to note whether we did any work.
    // If not, perhaps this thread should sleep for a little while.
    // This sounds counter-intuitive, but if we went to sleep when
    // there was work to do, it could take a long time to work through
    // a backlog. 
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

    // sleep intervals should be short WRT the interval between
    // buffers.  An IF buffer is 30K samples long at a rate of
    // 625K samples/sec. That suggests a buffer rate of about
    // 20K buffers/sec, or a new buffer every 50 microseconds.
    // Let's sleep for about 10 microseconds at a time. 
    if(!did_work) {
      usleep(10000);
    }
  }
}

/**
 * Each time we get a new IF buffer, we will attempt to send it
 * to the process (if any) listening to the server-socket. 
 */
bool IFServer::sendBuffer(SoDa::Buf * rxbuf) {
  // if there is someone listening, and the socket can take input...
  if(server_socket->isReady()) {  
    // package up a message.
    // We're going to need to build an outbound packet from an
    // unsigned long, a double, and a 1D array of complex<float>

    // First, how many bytes will the complex values require?
    unsigned int buffer_length = rxbuf->getComplexLen() * sizeof(std::complex<float>);

    // dangerous hack... can't figure out how else to do it.
    // The message size is the header + the buffer length
    int message_size = sizeof(unsigned long) + sizeof(double) + buffer_length;
    char message[message_size];

    // now build pointers to the length, frequency, and buffer start
    auto blen_p = reinterpret_cast<unsigned long *>(message);
    auto freq_p = reinterpret_cast<double *>(message + sizeof(unsigned long));
    char * cbuf = message + sizeof(unsigned long) + sizeof(double);

    // fill in the length and frequency
    *blen_p = buffer_length;
    *freq_p = current_rx_center_freq;
    
    // copy the data
    memcpy(cbuf, rxbuf->getComplexBuf(), buffer_length);

    // send the message
    server_socket->put(message, message_size);

    // we sent something -- inform the caller.
    return true;
  }

  // the socket wasn't ready, or nobody is listening.
  return false; 
}

void IFServer::shutDown() {
  // close the socket
  delete server_socket; 
}

void IFServer::subscribeToMailBox(const std::string & mbox_name, SoDa::BaseMBox * mbox_p)
{
  // This plugin thread needs to listen to the command stream.
  // The command stream supplies the current setting of the front-end oscillator,
  // which is also the center frequency for the incoming IF buffer.
  // The command stream indicates when the thread should stop. 
  if(SoDa::connectMailBox<SoDa::CmdMBox>(this, cmd_stream, "CMD", mbox_name, mbox_p)) {
    cmd_subs = cmd_stream->subscribe();
  }

  // This plugin thread needs to listen to the RX input data stream.
  // This stream carries the digitzed/downsampled data stream more-or-less
  // raw from the USRP. 
  if(SoDa::connectMailBox<SoDa::DatMBox>(this, rx_stream, "RX", mbox_name, mbox_p)) {
    rx_subs = rx_stream->subscribe();
  }
}

void IFServer::execRepCommand(SoDa::Command * cmd)
{
  // The thread got a command that is a REPORT.  Is it
  // reporting the RX front-end frequency? 
  switch (cmd->target) {
  case SoDa::Command::RX_FE_FREQ:
    // For a description of the format of this report message,
    // see the definition in SoDa::Command (described in the
    // doxygen documentation. 
    current_rx_center_freq = cmd->dparms[0];
    // We'll use the current rx center freq setting in outbound messages
    break;
  }
}
