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

namespace SoDa {
  class GuiParams {
  public:
    GuiParams(int argc, char ** argv);

    std::string getServerSocketBasename() const { return server_sock_basename; }
    std::string getServerName() const { return server_name; }
    std::string getServerArgs() const { return server_args; }
    std::string getLogFileName() const { return log_filename; }
    std::string getConfigFileName() const { return config_filename; }
    std::string getUHDArgs() const { return uhd_args; }
    unsigned int getDebugLevel() const { return debug_level; }
    std::string getAudioPortName() const { return audio_portname; }

    unsigned int getHamlibPortNumber() const { return hamlib_portnumber; }
    
    void setServerSocketBasename(const std::string s) { server_sock_basename = s; }

    bool hadNoCommand() { return no_command_only_help; }


  private:
    boost::program_options::variables_map pmap;

    bool no_command_only_help;

    std::string server_name;     ///< Where do we find the server?
    std::string server_args; ///< server control arguments.
    // message socket params
    std::string server_sock_basename; 
    std::string config_filename; 
    std::string log_filename;
    std::string uhd_args;
    unsigned int debug_level; ///< 0 => no debug messages .. more detail with higher values
    std::string audio_portname; 

    unsigned int hamlib_portnumber;
  };

}


#endif
