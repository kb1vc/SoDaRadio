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
#include "SimpleAccessory.hxx"
#include <SoDa/Format.hxx>
#include <iostream>
#include <string>

// namespace doesn't matter here... let's do without.

/**
 * To make an accessory thread that gets loaded and connected automagically, 
 * just create a normal thread object.  Like this: 
 */
SimpleAccessory my_accessory(std::string("Accessory1"));


SimpleAccessory::SimpleAccessory(const std::string & name) : SoDa::Thread(name) {
  get_count = set_count = rep_count = 0;
}

void SimpleAccessory::printReport() {
  std::cerr << SoDa::Format("Accessory named %s reports: Get: %8d  Set: %8d  Rep: %8d\n")
    .addS(getObjName())
    .addI(get_count, 8)
    .addI(set_count, 8)
    .addI(rep_count, 8); 
}

void SimpleAccessory::run() {
  bool exitflag = false; 

  while(!exitflag) {
    auto cmd = cmd_stream->get(cmd_subs); 
    while (cmd != NULL) {
      execCommand(cmd);
      exitflag |= (cmd->target == SoDa::Command::STOP);      
      cmd_stream->free(cmd);
      cmd = cmd_stream->get(cmd_subs); 
    }

    usleep(10000);
  }
}

void SimpleAccessory::subscribeToMailBox(const std::string & mbox_name, SoDa::BaseMBox * mbox_p)
{
  if(SoDa::connectMailBox<SoDa::CmdMBox>(this, cmd_stream, "CMD", mbox_name, mbox_p)) {
    cmd_subs = cmd_stream->subscribe();
  }
}


