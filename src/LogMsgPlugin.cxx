/*
Copyright (c) 2019,2022 Matthew H. Reilly (kb1vc)
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
#include "LogMsgPlugin.hxx"
#include <SoDa/Format.hxx>
#include <iostream>
#include <string>
#include "MailBoxRegistry.hxx"

/**
 * Logs all command SET, REP, GET messages on the cmd message stream. 
 * 
 */
extern "C" {
  LogMsgPlugin * initLib(const std::string & v) {
    return new LogMsgPlugin("LogMessages");
  }
}

LogMsgPlugin::LogMsgPlugin(const std::string & name) : SoDa::Thread(name) {
  std::cerr << "LogMsgPlugin Opening log file.\n";
  log.open("messages.log");
}

void LogMsgPlugin::logCommand(SoDa::CmdMsg cmd) {
  log << cmd->toString() << "\n";
}

void LogMsgPlugin::run() {
  bool exitflag = false; 

  while(!exitflag) {
    auto cmd = cmd_stream->get(cmd_subs); 
    while (cmd != NULL) {
      logCommand(cmd); 
      exitflag |= (cmd->target == SoDa::Command::STOP);      
      cmd = cmd_stream->get(cmd_subs); 
    }

    sleep_us(1000);
  }
}

void LogMsgPlugin::subscribe() {
  std::cerr << "LogMsgPlugin subscribing to streams\n";
  auto reg = SoDa::MailBoxRegistry::getRegistrar();
  cmd_stream = SoDa::MailBoxBase::convert<SoDa::MsgMBox>(reg->get("CMD"));
  cmd_subs = cmd_stream->subscribe();
}


