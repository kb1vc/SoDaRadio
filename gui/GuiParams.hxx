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

#ifndef GUIPARAMS_HDR
#define GUIPARAMS_HDR

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <wx/app.h>

extern "C" {
#include <stdlib.h>
}

namespace SoDa {
  class BaseGuiParams {
  public:
    BaseGuiParams(int argc, wxChar ** argv, 
		  const std::string def_sock_basename, 
		  const std::string config_filename = std::string(""), 
		  const std::string log_filename = std::string(""));

    std::string getServerSocketBasename() { return server_sock_basename; }
    std::string getServerName() { return server_name; }
    std::string getLogFileName() { return log_filename; }
    std::string getConfigFileName() { return config_filename; }
    std::string getUHDArgs() { return uhd_args; }
    unsigned int getDebugLevel() { return debug_level; }
  private:
    // this is really quite gross -- wxApp is not very nice about this. 
    char ** convertWXargs2Cargs(int argc, wxChar ** argv);
    
    boost::program_options::variables_map pmap;

    std::string server_name;     ///< Where do we find the server?
    // message socket params
    std::string server_sock_basename; 
    std::string config_filename; 
    std::string log_filename;
    std::string uhd_args;
    unsigned int debug_level; ///< 0 => no debug messages .. more detail with higher values
  };

  class RadioGuiParams : public BaseGuiParams {
  public:
    RadioGuiParams(int argc, wxChar ** argv) : 
      BaseGuiParams(argc, argv, 
		    std::string("/tmp/SoDa_"),
		    getenv("HOME") + std::string("/.SoDaRadio/SoDa.soda_cfg"), 
		    std::string("SoDa.soda_log")) {
    }
  }; 

  class SNAGuiParams : public BaseGuiParams {
  public:
    SNAGuiParams(int argc, wxChar ** argv) : 
      BaseGuiParams(argc, argv,
		    std::string("/tmp/SoDaSNA_"),
		    getenv("HOME") + std::string("/.SoDaRadio/SoDaSNA.cfg")) {
    }
  }; 
}


#endif
