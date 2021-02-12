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

#include "Params.hxx"
#include <iostream>
#include <stdlib.h>
#include <SoDa/Options.hxx>
#include <SoDa/Utils.hxx>

SoDa::Params::Params(int argc, char * argv[])
{
  SoDa::Options cmd;
  cmd.addP(&help_please, "help", 'h', "Show this message")
    .addV<std::string>(&load_list, "load", 'l',
		       "one or more shared library objects, each containing a SoDa::Thread object")
    .add<std::string>(&radio_args, "uhdargs", 'u', "",  
		     "extra parameters to pass to device creator (e.g. device id, address, type)")
    .add<std::string>(&config_filename, "config",  'c',  "~/.soda/config.xml",
		      "configuration file for initial settings")
    .add<std::string>(&server_sock_basename, "uds_name", 's',  "/tmp/SoDa_",
		      "unix domain socket name for server to UI client message channels")
    .add<std::string>(&audio_portname, "audio", 'a',  "default", 
		      "Audio device name for ALSA audio.")
    .add<bool>(&force_integer_N_mode, "intN", 'N',  true, 
	       "Force Integer-N synthesis for front-end local oscillators")
    .add<unsigned int>(&debug_level, "debug", 'D',  0,
		       "Enable debug messages for value > 0.  Higher values may produce more detail.")
    .add<std::string>(&radio_type, "radio", 'r',  "USRP", 
     "the radio type (USRP is the only currently supported device)")
    .add<std::string>( &gps_hostname, "gps_host", 'G',  "localhost", 
		      "hostname for gpsd server")
    .add<std::string>(&gps_portname, "gps_port", 'p',  "2947",
		      "port number for gpsd server")
    .add<std::string>(&lock_file_name, "lockfile", 'k',  "SoDa.lock", 
		      "lock file to signal that a sodaradio server is active")
    .addInfo("SoDa -- The 'SoD' stands for Software Defined. The 'a' doesn't stand for anything.")
    ;

  cmd.parse(argc, argv); 

  // do we need a help message?
  if(cmd.isPresent("help")) {
    cmd.printHelp(std::cout);
    exit(-1); 
  }

  // now we fill in the parameters.
  readConfigFile(config_filename);

  // when we're asked for loadable modules, we also scan the SODA_LOAD_LIBS env variable. 
  load_list_env_appended = false;    
}

void SoDa::Params::readConfigFile(std::string & cf_name)
{
  (void)cf_name;
  // do nothing for now... later this will be the .xml file.
  clock_source_internal = true;
  rx_rate = 625000.0; // 100.0e6/256.;
  tx_rate = 625000.0 / 2.0 ; //100.0e6/2048.;
  rx_ant = "RX2";
  tx_ant = "TX/RX";
}

void SoDa::Params::saveConfigFile(std::string & cf_name)
{
  (void) cf_name; 
  // do nothing for now... later this will be the .xml file. 
}

const std::vector<std::string> & SoDa::Params::getLibs() {
  // note that this will append
  if(!load_list_env_appended) {
    // did we set an env variable
    char * envlibs_str = getenv("SODA_LOAD_LIBS");
    if(envlibs_str != NULL) {
      std::string envlibs(envlibs_str);
      std::list<std::string> spl = SoDa::split(envlibs_str, ", ", true);
      for(auto e : spl) {
	load_list.push_back(e);
      }
    }
  }
  load_list_env_appended = true;
  return load_list; 
}
