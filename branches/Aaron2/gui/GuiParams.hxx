/*
Copyright (c) 2013, Matthew H. Reilly (kb1vc)

Tracker changes (interface to hamlib and gpredict)
Copyright (c) 2014, Aaron Yankey Antwi (aaronyan2001@gmail.com)

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

namespace SoDa {
  class GuiParams {
  public:
    GuiParams(int argc, wxChar ** argv);

    std::string getServerSocketBasename() { return server_sock_basename; }
    std::string getServerName() { return server_name; }
    std::string getLogFileName() { return log_filename; }
    std::string getConfigFileName() { return config_filename; }
    std::string getUHDArgs() { return uhd_args; }
    bool withTracking() { return with_tracking; }
  private:
    // this is really quite gross -- wxApp is not very nice about this. 
    char ** convertWXargs2Cargs(int argc, wxChar ** argv);
    
    boost::program_options::variables_map pmap;

    bool with_tracking; ///< if true, the tracking listener is active.
    
    std::string server_name;     ///< Where do we find the server?
    // message socket params
    std::string server_sock_basename; 
    std::string config_filename; 
    std::string log_filename;
    std::string uhd_args; 
  };

}


#endif
