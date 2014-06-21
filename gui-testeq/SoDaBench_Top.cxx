/*
  Copyright (c) 2012,2013,2014 Matthew H. Reilly (kb1vc)
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
extern "C" {
#include "dem-gridlib.h"
#include <string.h>
}
#include "SoDaBench_Top.h"
#include <wx/string.h>
#include <wx/wx.h>
#include <wx/colour.h>
#include "../src/Command.hxx"
#include <boost/format.hpp>
#include "SoDaLogo.xpm"
#include "FindHome.hxx"
/**
 * @file SoDaBench_Top.cxx
 *
 * @brief The toplevel methods for the SoDaBench GUI
 *
 * @author Matt Reilly (kb1vc)
 */

namespace SoDaBench_GUI {
  /**
   * The SoDaBench object
   *
   * The SoDaBench program provides a wxWidgets based GUI to control
   * the SoDa bench SDR control program. SoDaBench is partitioned into
   * three major components
   *
   * @li The Application object defined in SoDaBench_App that launches the 
   * main GUI control thread defined by SoDaBench_Top
   * @li the BenchListenener object that connects to the SoDa::UI thread.
   * @li the wxWidgets GUI event loop that dispatches user requests
   * through the SoDaBench_Top thread.
   */
  SoDaBench_Top::SoDaBench_Top( SoDa::GuiParams & params,  wxWindow* parent )
:
    SoDaBenchFrame( parent )
  {
    // revision string is initially empty
    SDR_version_string[0] = '\000';

    // make an icon.
    wxIcon logo((char **) &SoDaLogo);
    this->SetIcon(logo);

    debug_mode = false;

    // Did we find a soda configuration file?  

    // Startup the server process -- it should be in the same directory as
    // this program, unless an alternate server image was specified. 
    std::string myhome = findHome();

    // Now start the server
    std::string server = params.getServerName();
    std::string server_commandline_string; 
    if(server == "SoDaServer") {
      // find it in our home directory
      server_commandline_string = myhome + "/SoDaServer";
    }
    else {
      server_commandline_string = server; 
    }

    // setup the comm channel.
    std::string sock_basename = params.getServerSocketBasename(); 

    // fix a problem with UBUNTU menu proxy... .
    char mproxyfix[] = "UBUNTU_MENUPROXY=";
    putenv(mproxyfix);
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
	std::string uhd_args = params.getUHDArgs();
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

    // setup the client socket trying once every second for 60 seconds before we give up
    soda_bench = new SoDa::UD::ClientSocket(sock_basename + "_cmd", 60);
    soda_fft = new SoDa::UD::ClientSocket(sock_basename + "_wfall", 60);

    // create the listener thread
    if(debug_mode || 1) {
      std::cerr << "Creating listener thread." << std::endl;
    }
    listener = new BenchListenerThread(this);
    // now launch it.
    if(debug_mode) {
      std::cerr << "Launching listener thread." << std::endl;
    }
    if(listener->Create() != wxTHREAD_NO_ERROR) {
      wxLogError(wxT("Couldn't create bench listener thread...")); 
    }

  
    if(debug_mode) {
      std::cerr << "Running listener thread." << std::endl;
    }
    listener->Run(); 

  }

    
}
