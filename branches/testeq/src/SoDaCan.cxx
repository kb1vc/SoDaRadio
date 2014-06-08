/*
Copyright (c) 2014, Matthew H. Reilly (kb1vc)
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

#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include "../gui/FindHome.hxx"
#include "UDSockets.hxx"
#include <map>

typedef void(CmdFn)(int, char **);
typedef std::map<std::string, CmdFn *> FuncMap;

void startServer(int argc, char * argv[])
{
  std::cerr << "In startServer" << std::endl; 
  namespace po = boost::program_options;
  po::positional_options_description m_pos;
  po::options_description m_cl("Command syntax");
  po::variables_map m_vars;

  std::string server;
  std::string uhd_args;
  std::string sock_basename; 
  m_cl.add_options()
    ("help", "help message")
    ("command", po::value< std::string >(), "")
    ("server", po::value<std::string>(&server)->default_value("SoDaServer"),
     "Name/path to SoDaServer program. Normally found in the directory containing SoDaCommand")
    ("uhdargs", po::value<std::string>(&uhd_args)->default_value(""),
     "multi uhd device address arguments -- 'type=b200' selects a B2xx unit in preference over an N2xx device")
    ("uds_name", po::value<std::string>(&sock_basename)->default_value("/tmp/SoDa_"),
     "unix domain socket name for server to UI client message channels")
    ("otherargs", po::value< std::vector< std::string > >(), "")
    ;

  m_pos.add("command", -1);


  po::variables_map vm; 
  po::store(po::command_line_parser(argc, argv).
	    options(m_cl).positional(m_pos).run(), vm);
  po::notify(vm);
  // Startup the server process -- it should be in the same directory as
  // this program, unless an alternate server image was specified. 
  std::string myhome = findHome();

  int stat; 
  // Now start the server

  std::string server_commandline_string; 
  if(server == "SoDaServer") {
    // find it in our home directory
    server_commandline_string = myhome + "/SoDaServer";
  }
  else {
    server_commandline_string = server; 
  }

  if(server != "None") {
    if(fork()) {
      int stat;
      char * argv[5];
      // coercion to char* (as opposed to const char*) is to get around
      // a compiler finickyness around conversions from const char** to const* char* or whatever.
      // sigh.
      argv[0] = (char*) server_commandline_string.c_str();
      argv[1] = (char*) "--uds_name";
      argv[2] = (char*) sock_basename.c_str();
      if(uhd_args != "") {
	argv[3] = (char*) "--uhdargs";
	argv[4] = (char*) uhd_args.c_str();
	argv[5] = NULL; 
      }
      else {
	argv[3] = NULL;
      }

      stat = execv(argv[0], argv); 
      if(stat < 0) {
	std::cerr << boost::format("Couldn't start SoDaServer. Got error [%s]. Is \"%s\" missing?\n")
	  % strerror(errno)
	  % server_commandline_string;
	exit(stat);
      }
    }
  }
}

void stopServer(int argc, char * argv[])
{
  std::cerr << "In startServer" << std::endl; 
}

void getRadioName(int argc, char * argv[])
{
  std::cerr << "In getRadioName" << std::endl; 
  namespace po = boost::program_options;
  po::positional_options_description m_pos;
  po::options_description m_cl("Command syntax");
  po::variables_map m_vars;

  std::string server;
  std::string uhd_args;
  std::string sock_basename; 
  m_cl.add_options()
    ("help", "help message")
    ("command", po::value< std::string >(), "")
    ("uds_name", po::value<std::string>(&sock_basename)->default_value("/tmp/SoDa_"),
     "unix domain socket name for server to UI client message channels")
    ;

  m_pos.add("command", -1);

  po::variables_map vm; 
  po::store(po::command_line_parser(argc, argv).
	    options(m_cl).positional(m_pos).run(), vm);
  po::notify(vm);

  SoDa::UD::ClientSocket * soda_radio =  new SoDa::UD::ClientSocket(sock_basename + "_cmd", 60);

  soda_radio->put(new SoDa::Command(SoDa::Command::GET, SoDa::Command::HWMB_REP), sizeof(SoDa::Command));
  SoDa::Command * ncmd = new SoDa::Command();
  int stat; 
  while(1) {
    stat = soda_radio->get(ncmd, sizeof(SoDa::Command));
    if(stat < 0) {
	std::cerr << boost::format("Bad message stat = %d. Got error [%s]\n") % stat
	  % strerror(errno);
    }
    else if((stat > 0) &&
	    (ncmd->cmd == SoDa::Command::REP) &&
	    (ncmd->target == SoDa::Command::HWMB_REP)) {
      std::cout << boost::format("USRP model is %s\n") % ncmd->sparm;
      exit(0);
    }
    else if(stat > 0) {
      std::cerr << "Got a message: " << ncmd->toString() << std::endl; 
    }
  }
}

int main(int argc, char * argv[])
{
  FuncMap command_func_map;
  command_func_map["start"] = startServer;
  command_func_map["stop"] = stopServer;
  command_func_map["name"] = getRadioName;
  
  namespace po = boost::program_options;
  po::positional_options_description m_pos;
  po::options_description m_cl("Command syntax");
  po::variables_map m_vars;

  m_cl.add_options()
    ("help", "help message")
    ("command", po::value< std::string >(), "")
    ("server", po::value<std::string>()->default_value("SoDaServer"),
     "Name/path to SoDaServer program. Normally found in the directory containing SoDaCommand")
    ("uhdargs", po::value<std::string>()->default_value(""),
     "multi uhd device address arguments -- 'type=b200' selects a B2xx unit in preference over an N2xx device")
    ("uds_name", po::value<std::string>()->default_value("/tmp/SoDa_"),
     "unix domain socket name for server to UI client message channels")
    ("otherargs", po::value< std::vector< std::string > >(), "")
    ;

  m_pos.add("command", -1);


  po::variables_map vm; 
  po::store(po::command_line_parser(argc, argv).
	    options(m_cl).positional(m_pos).run(), vm);
  po::notify(vm);


  if(vm.count("command")) {
    std::cout << "Command was [" << vm["command"].as<std::string>() << "]" << std::endl;
    std::string cmd = vm["command"].as<std::string>();
    BOOST_FOREACH(FuncMap::value_type &c, command_func_map) {
      if(c.first == cmd) c.second(argc, argv);
    }
  }
  else {
    std::cout << boost::format("Command syntax: %s <cmd> [--help]\n\twhere <cmd> is one of\n") % argv[0];
    BOOST_FOREACH(FuncMap::value_type &c, command_func_map) {
      std::cout << c.first << std::endl; 
    }
  }
    
}
