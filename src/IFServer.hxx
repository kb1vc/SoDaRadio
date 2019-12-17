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

#ifndef SODA_IF_SERVER_HDR
#define SODA_IF_SERVER_HDR

#include "SoDaBase.hxx"
#include "SoDaThread.hxx"
#include "UDSockets.hxx"
/**
 * @file IFServer.hxx
 *
 * This unit listens on the IF channel and exports a 
 * socket interface that sends out blocks of data. 
 * Each block is of the form: 
 * uint32 buffer_length (bytes)
 * double center
 * complex<float> [buffer_length]
 * 
 * The socket is a unix domain socket, so records are
 * transferred whole. 
 *
 * @author Matt Reilly (kb1vc)
 *
 */

// namespace doesn't matter here... let's do without. 
class IFServer : public SoDa::Thread {
public:
  IFServer(const std::string & name);

  /**
   * @brief connect to useful mailboxes. 
   * 
   * @param mbox_name which mailbox are we being offered? 
   * @param mbox_p a pointer to the mailbox we are being offered. 
   */
  void subscribeToMailBox(const std::string & mbox_name, SoDa::BaseMBox * mbox_p);

  void run();

  void shutDown();

  void execRepCommand(SoDa::Command * cmd);
  
protected:
  bool sendBuffer(SoDa::Buf * rxbuf); 

  unsigned int cmd_subs; ///< mailbox subscription ID for command stream
  SoDa::CmdMBox * cmd_stream; ///< mailbox producing command stream from user
  unsigned int rx_subs; ///< mailbox subscription ID for command stream
  SoDa::DatMBox * rx_stream; ///< mailbox producing command stream from user

  SoDa::UD::ServerSocket * server_socket;

  double current_rx_center_freq; 
};

#endif
