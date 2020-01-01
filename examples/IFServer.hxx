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

#include <SoDaRadio/SoDaBase.hxx>
#include <SoDaRadio/SoDaThread.hxx>
#include <SoDaRadio/UDSockets.hxx>
/**
 * @file IFServer.hxx
 *
 * IFServer is an example of a simple "plugin" that may be 
 * built as a shareable library and then loaded into SoDaRadio
 * via the loadable plugin interface. 
 * 
 * Plugins may be loaded by setting an environment variable
 * "SODA_LOAD_LIBS" or by providing the "--load" option to 
 * the SoDaServer.  
 * 
 * For instance, once this plugin has been built, it will be loaded 
 * by this command: 
 * 
~~~
$ SoDaRadio --serverargs "--load libIFServer.so"
~~~
 * 
 * or, using an environment variable instead: 
 * 
~~~
$ export SODA_LOAD_LIBS=libIFServer.so
$ SoDaRadio
~~~
 *
 * User developed plugins may find this a useful starting point. 
 * 
 * @author Matt Reilly (kb1vc)
 *
 */


/**
 * @class IFServer 
 * 
 * This is a thread that subscribes to the SoDaRadio command and 
 * IF streams. It listens for new IF buffers from the RX thread, 
 * and makes them available on a unix domain socket called "IFServer"
 * and located in the directory from which SoDaRadio was started. 
 *
 * Each block is of the form: 
 *
 * uint32 buffer_length (bytes)
 * double center
 * complex<float> [buffer_length]
 */
class IFServer : public SoDa::Thread {
public:

  /**
   * @brief create the plugin thread instance. 
   * 
   * This will register the plugin with the thread registrar and make the
   * server process aware of the plugin's existence. 
   * 
   * @param name This is a convenience name used in error reports and 
   * debugging aids. It need not be unique or even meaningful. 
   */
  IFServer(const std::string & name);

  /**
   * @brief connect to useful mailboxes. 
   * 
   * This is a virtual method that should be implemented by 
   * any SoDa::Thread object, as it is the mechanism through
   * which a thread may subscribe or connect to the data and
   * command streams.  
   * 
   * The SoDaRadio server will offer each mailbox/stream to 
   * every thread.  A thread may subscribe to or ignore the stream. 
   * 
   * @param mbox_name which mailbox are we being offered? 
   * @param mbox_p a pointer to the mailbox we are being offered. 
   */
  void subscribeToMailBox(const std::string & mbox_name, SoDa::BaseMBox * mbox_p);

  /**
   * @brief This is the method that does the actual work.  It is 
   * called by the server for each thread.  The thread should not
   * return from this function until a STOP message arrives on the command
   * stream. 
   */
  void run();

  /**
   * @brief Each thread must implement this method.  It provides for
   * an orderly shutdown of all open connections, or other resources
   * and processes in a thread. It is *not* necessarily the destructor
   * for this thread. 
   */
  void shutDown();

  /**
   * @brief A thread may implement any or all of the command 
   * execution methods (execRepCommand, execSetCommand, execGetCommand). 
   * 
   * execRepCommand interprets an incoming SoDa::Command where the
   * SoDa::Command::CmdType is REP (report).  Commands of this kind
   * carry information like "this is my status" or "the current 
   * RX front end tuning frequency is X MHz." 
   * 
   * This plugin listens on the command channel for "STOP" messages
   * and "RX_FE_FREQ" messages.  The former tells the plugin to exit
   * the run method.  The latter tells the plugin that the IF center
   * frequency has changed to the new RX_FE_FREQ. 
   * 
   * @param cmd a command of type SoDa::Command that includes the
   * type of command (report, setter, getter), the parameter being
   * referenced (RX_FE_FREQ, STOP...) and an optional data value. 
   */
  void execRepCommand(SoDa::Command * cmd);
  
protected:
  /**
   * @brief send a buffer of complex samples to clients connected
   * to the socket.  The buffer is preceded by a buffer length (UI32)
   * and the current center frequency (double).  The buffer itself
   * is a sequence of complex<float> values. 
   * 
   * @param rxbuf a complex<float> buffer wrapped in a SoDa::Buf object.
   * @return true if data was sent, false otherwise. 
   */
  bool sendBuffer(SoDa::Buf * rxbuf); 

  unsigned int cmd_subs; ///< mailbox subscription ID for command stream
  SoDa::CmdMBox * cmd_stream; ///< mailbox producing command stream from user
  unsigned int rx_subs; ///< mailbox subscription ID for command stream
  SoDa::DatMBox * rx_stream; ///< mailbox producing command stream from user

  /**
   * @brief unix domain server socket object.  SoDa::UD::ServerSocket
   * wraps the normal select/read/write/accept/connect interface in a 
   * simple socket object. 
   */
  SoDa::UD::ServerSocket * server_socket; 

  double current_rx_center_freq; 
};

#endif
