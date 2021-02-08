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
#include <boost/algorithm/string.hpp>

SoDa::Params::Params(int argc, char * argv[])
{
  namespace po = boost::program_options;

  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "help message")
    ("load", po::value<std::vector<std::string> >(&load_list)->multitoken(), 
     "one or more shared library objects, each containing a SoDa::Thread object")
    ("uhdargs", po::value<std::string>(&radio_args)->default_value(""),
     "extra parameters to pass to device creator (e.g. device id, address, type)")
    ("config",  po::value<std::string>(&config_filename)->default_value("~/.soda/config.xml"),
     "configuration file for initial settings")
    ("uds_name", po::value<std::string>(&server_sock_basename)->default_value("/tmp/SoDa_"),
     "unix domain socket name for server to UI client message channels")
    ("audio", po::value<std::string>(&audio_portname)->default_value("default"), 
     "Audio device name for ALSA audio.")
    ("intN", po::value<bool>(&force_integer_N_mode)->default_value(false)->implicit_value(true),
     "Force Integer-N synthesis for front-end local oscillators")
    ("fracN", po::value<bool>(&force_frac_N_mode)->default_value(false)->implicit_value(true),
     "Force Fractional-N synthesis for front-end local oscillators")
    ("debug", po::value<unsigned int>(&debug_level)->default_value(0)->implicit_value(1),
     "Enable debug messages for value > 0.  Higher values may produce more detail.")
    ("radio", po::value<std::string>(&radio_type)->default_value("USRP"), 
     "the radio type (USRP, Lime)")
    ("gps_host", po::value<std::string>(&gps_hostname)->default_value("localhost"), 
     "hostname for gpsd server")
    ("gps_port", po::value<std::string>(&gps_portname)->default_value("2947"),
     "port number for gpsd server")
    ("lockfile", po::value<std::string>(&lock_file_name)->default_value("SoDa.lock"), 
     "lock file to signal that a sodaradio server is active")
    ;

  po::store(po::parse_command_line(argc, argv, desc), pmap);
  po::notify(pmap);

  // do we need a help message?
  if(pmap.count("help")) {
    std::cout << "SoDa -- The 'SoD' stands for Software Defined. The 'a' doesn't stand for anything.   " << desc << std::endl;
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
      std::vector<std::string> spl;
      boost::algorithm::split(spl, envlibs, boost::is_any_of(", "));
      for(auto e : spl) {
	load_list.push_back(e);
      }
    }
  }
  load_list_env_appended = true;
  return load_list; 
}
