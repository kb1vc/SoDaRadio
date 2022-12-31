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

#include "GuiParams.hxx"
#include <iostream>


SoDa::GuiParams::GuiParams(int argc, char ** argv)
{
  cmd
    .add<std::string>(&server_name, "server", 's', "", 
     "Name/path to SoDaServer program. Normally found in the directory containing SoDaRadio")
    .add<std::string>(&server_args, "serverargs", 'a', "", 
     "Argument string to be passed to SoDaServer program")
    .add<std::string>(&server_sock_basename, "uds_name", 'S', "",
     "unix domain socket name for server to UI client message channels")
    .add<std::string>(&config_filename, "config", 'c', "",
     "Configuration file with initial settings. Otherwise SoDaRadio will look in ${HOME}/.config/kb1vc.org/SoDaRadioQT.conf"
     )
    .add<std::string>(&log_filename, "log", 'l', "SoDa.soda_log",
     "Log filename")
    .add<unsigned int>(&hamlib_portnumber, "hamlib_port", 'h', 4575,
     "TCP port number for this hamlib server.")
    .add<std::string>(&audio_portname, "audio", 'A', "default",
     "Audio device name for ALSA audio.")
    .add<unsigned int>(&debug_level, "debug", 'D', 0,
     "Enable debug messages for value > 0.  Higher values may produce more detail.")
    ;

  std::cerr << "About to parse GUI commands\n";
  for(int i = 0; i < argc; i++) {
    std::cerr << i << "\t[" << argv[i] << "]\n";
  }
  
  if(!cmd.parse(argc, argv)) {
    no_command_only_help = true; 
    std::cerr << "Ooops.\n";
  }
  else {
    no_command_only_help = false;
    std::cerr << "OK-----------------.\n";    
  }
}


