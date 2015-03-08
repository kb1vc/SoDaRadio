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
#include "SoDaSNA_Top.h"
#include <wx/string.h>
#include <wx/wx.h>
#include <wx/colour.h>
#include "../src/Command.hxx"
#include "../src/Debug.hxx"
#include <boost/format.hpp>
#include "SoDaLogo.xpm"
#include "FindHome.hxx"
/**
 * @file SoDaSNA_Top.cxx
 *
 * @brief The toplevel methods for the SoDaSNA GUI
 *
 * @author Matt Reilly (kb1vc)
 */

namespace SoDaSNA_GUI {
  /**
   * The SoDaSNA object
   *
   * The SoDaSNA program provides a wxWidgets based GUI to control
   * the SoDa radio SDR control program. SoDaSNA is partitioned into
   * three major components
   *
   * @li The Application object defined in SoDaSNA_App that launches the 
   * main GUI control thread defined by SoDaSNA_Top
   * @li the SNAListenener object that connects to the SoDa::UI thread.
   * @li the wxWidgets GUI event loop that dispatches user requests
   * through the SoDaSNA_Top thread.
   */
  SoDaSNA_Top::SoDaSNA_Top( SoDa::SNAGuiParams & params,  wxWindow* parent )
:
    SoDaSNAFrame( parent ), SoDa::Debug("SoDaSNA_Top")
  {
    // init all pointers to NULL;
    pgram_plot = NULL;
    pgram_trace = NULL;
    listener = NULL;
    config_tree_alloc = config_tree = NULL; 
    
    // revision string is initially empty
    SDR_version_string[0] = '\000';

    // make an icon.
    wxIcon logo((char **) &SoDaLogo);
    this->SetIcon(logo);

    SoDa::Debug::setDefaultLevel(params.getDebugLevel());
    setDebugLevel(params.getDebugLevel());

    debugMsg(boost::format("Debug level set to %d") % getDebugLevel());

    // setup the save config dialog -- we need to create one 
    // gtk file dialog before the fork, as there is a bug in the GTK 
    // implementations of gtk_file_chooser_dialog_new.. a mystery to me. 
    debugMsg((boost::format("creating save config dialog with this = %p.\n") % this).str());
    save_config_dialog = new wxFileDialog(this,
					  wxT("Save to Selected Configuration File"),
					  wxT("~/.SoDaRadio"),
					  wxT("SoDaSNA.cfg"),					     
					  wxT("SoDa Config files (*.cfg)|*.cfg"),
					  wxFD_SAVE | wxFD_CHANGE_DIR);
    debugMsg("created save config dialog.");    
    // Did we find a soda configuration file?  

    // Startup the server process -- it should be in the same directory as
    // this program, unless an alternate server image was specified. 
    std::string myhome = findHome();

    // Now start the server
    std::string server = params.getServerName();
    std::string server_commandline_string; 
    if(server == "SoDaSNAServer") {
      // find it in our home directory
      server_commandline_string = myhome + "/SoDaSNAServer";
    }
    else {
      server_commandline_string = server; 
    }

    // setup the comm channel.
    std::string sock_basename = params.getServerSocketBasename(); 

    // fix a problem with UBUNTU menu proxy... .
    char mproxyfix[] = "UBUNTU_MENUPROXY=";
    putenv(mproxyfix);

    // Note that earlier versions used fork/execv but this really screws up 
    // gtk for reasons that are not obvious.  (The file chooser gets very very 
    // sluggish.)
    if(server != "None") {
      // use wxExecute 
      wxString scmd = wxString(server_commandline_string.c_str(), wxConvUTF8) +  wxT(" --uds_name ") + wxString(sock_basename.c_str(), wxConvUTF8);

      std::string uhd_args = params.getUHDArgs();
      if(uhd_args != "") {
	scmd = scmd + wxT(" --uhdargs ") + wxString(uhd_args.c_str(), wxConvUTF8);
      }

      if(getDebugLevel()) {
	scmd = scmd + 
	  wxString((boost::format(" --debug %d") % getDebugLevel()).str().c_str(), wxConvUTF8);
      }

      debugMsg(boost::format("About to execute [%s]\n") % scmd.mb_str());

      // this appears to be "safe" wrt wxwidgets and the gtk stuff behind it.
      wxExecute(scmd);
    }

    // setup the client socket trying once every second for 60 seconds before we give up
    soda_radio = new SoDa::UD::ClientSocket(sock_basename + "_cmd", 60);

    // create the listener thread
    debugMsg("Creating listener thread.");
    listener = new SNAListenerThread(this);

    // create the periodogram plot
    // and init its controls
    int xs, ys;
    m_DisplayPanel->GetSize(&xs, &ys);
    debugMsg(boost::format("size of xy graph panel %d x %d ") % xs % ys);

    pgram_plot = new SoDaRadio_GUI::XYPlot(m_DisplayPanel, NULL, wxID_ANY, 
					   wxDefaultPosition,  wxSize(xs, ys), //wxDefaultSize, // 
					   SoDaRadio_GUI::XYPlot::DRAW_LABEL | SoDaRadio_GUI::XYPlot::DRAW_MARKERS |
					   SoDaRadio_GUI::XYPlot::DRAW_TITLE | SoDaRadio_GUI::XYPlot::DRAW_VERT_MARKER_BANDS 
					   );

    pgram_plot->SetXTicTemplate(wxT("%8.4f"), 1e-6);
    pgram_plot->SetXCenterTemplate(wxT("%12.4f"));
    pgram_plot->SetXLabelMode(SoDaRadio_GUI::XYPlot::ABSOLUTE);
    int t, b, l, r; 
    pgram_plot->GetGridOffsets(t, b, l, r); 
    t += 50;
    b += 40;
    l += 50;
    pgram_plot->SetGridOffsets(t, b, l, r); 
    pgram_plot->SetXLabel(wxT("Frequency (MHz)"));
    pgram_plot->SetYLabel(wxT("Gain (dB)"));
    
    // debugMsg("Running listener thread.");
    // listener->Run(); 

  
    // Now connect up a few events
    Connect(MSG_UPDATE_MODELNAME, wxEVT_COMMAND_MENU_SELECTED,
	    wxCommandEventHandler(SoDaSNA_Top::OnUpdateModelName));

    Connect(MSG_SNA_SCAN_REPORT, wxEVT_COMMAND_MENU_SELECTED,
	    wxCommandEventHandler(SoDaSNA_Top::OnScanReport));

    Connect(MSG_SNA_SCAN_END, wxEVT_COMMAND_MENU_SELECTED,
	    wxCommandEventHandler(SoDaSNA_Top::OnScanEnd));

    min_span = 100.0e3;
    max_span = 2.0e9;
    min_cfreq = 50.0e6;
    max_cfreq = 2.0e9;

    display_cfreq = 150.0e6;
    display_start_freq = 149.5e6;
    display_end_freq = 150.5e6;

    GetFreqSettings();
    
    display_min_gain = -70.0;
    display_max_gain = 30.0; 
    updateXYPlot();

    sweep_mode = NO_SWEEP; 
  }

  void SoDaSNA_Top::configureRadio(SoDa::SNAGuiParams & params) {
    debugMsg("about to load configuration.");
    // load the configuration from a default file,
    // if available.
    std::string cfn = params.getConfigFileName();

    if(cfn == "") {
      std::string home_dir(getenv("HOME"));
      cfn = home_dir + "/.SoDaRadio/SoDaSNA.cfg";
    }

    wxString config_filename(cfn.c_str(), wxConvUTF8);
    LoadConfig(config_filename);
    debugMsg("loaded configuration.");
  }
  
  bool SoDaSNA_Top::CreateSpectrumTrace(double * freqs, float * powers, unsigned int len)
  {
    // This method does NO wx GUI manipulations -- it is safe
    // to call from a separate thread. 
    wxMutexLocker lock(ctrl_mutex);
    if(pgram_plot == NULL) return false; 
  
    pgram_trace = new SoDaRadio_GUI::XYPlot::Trace(freqs, powers, len, 1, wxT(""));
    wxColor t1(0xff, 0x00, 0x00, 0x80); 
    pgram_plot->AddTrace(0, t1, pgram_trace);
    return true; 
  }


  void SoDaSNA_Top::updateXYPlot() {
    pgram_plot->SetCenterFreq(display_cfreq); 
    pgram_plot->SetScale(display_start_freq, display_end_freq, display_min_gain, display_max_gain);
  }

  void SoDaSNA_Top::postErrorText(const std::string & errmsg) {
    std::cerr << errmsg << std::endl; 
  }

  void SoDaSNA_Top::clearErrorText() {
  }


}
