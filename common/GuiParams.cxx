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
  namespace po = boost::program_options;
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "help message")
    ("server", po::value<std::string>(&server_name)->default_value(""),
     "Name/path to SoDaServer program. Normally found in the directory containing SoDaRadio")
    ("serverargs", po::value<std::string>(&server_args)->default_value(""), 
     "Argument string to be passed to SoDaServer program")
    ("uhdargs", po::value<std::string>(&uhd_args)->default_value(""),
     "multi uhd device address arguments -- 'type=b200' selects a B2xx unit in preference over an N2xx device")
    ("uds_name", po::value<std::string>(&server_sock_basename)->default_value(""),
     "unix domain socket name for server to UI client message channels")
    ("config", po::value<std::string>(&config_filename)->default_value(""),
     "Configuration file with initial settings. Otherwise SoDaRadio will look in ${HOME}/.SoDaRadio/SoDa.soda_cfg"
     )
    ("log", po::value<std::string>(&log_filename)->default_value("SoDa.soda_log"),
     "Log filename")
    ("hamlib_port", po::value<unsigned int>(&hamlib_portnumber)->default_value(4575),
     "TCP port number for this hamlib server.")
    ("audio", po::value<std::string>(&audio_portname)->default_value("default"), 
     "Audio device name for ALSA audio.")
    ("debug", po::value<unsigned int>(&debug_level)->default_value(0)->implicit_value(1),
     "Enable debug messages for value > 0.  Higher values may produce more detail.")
    ;

  po::store(po::parse_command_line(argc, argv, desc), pmap);
  po::notify(pmap);

  // do we need a help message?
  if(pmap.count("help")) {
    std::cout << "SoDa -- The 'SoD' stands for Software Defined. The 'a' doesn't stand for anything.   " << desc << std::endl;
    no_command_only_help = true; 
  }
  else {
    no_command_only_help = false; 
  }
}


