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
#include <string.h>
}
#include "SoDaBench_Top.h"
#include "BenchListenerThread.hxx"
#include <wx/string.h>
#include <wx/wx.h>
#include <wx/colour.h>
#include "../src/Command.hxx"
#include <boost/format.hpp>
#include "SoDaLogo.xpm"
#include "../gui/FindHome.hxx"
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
  SoDaBench_Top::SoDaBench_Top( SoDa::GuiParams & _params,  wxWindow* parent )
:
    SoDaBenchFrame( parent ), SoDaRadio_GUI::GraphClient()
  {
    params = _params;
    // revision string is initially empty
    SDR_version_string = std::string("");

    // make an icon.
    wxIcon logo((char **) &SoDaLogo);
    this->SetIcon(logo);

    debug_mode = false;

    // startup the server
    setupServer(); 

    // now link to the server; 
    initListener(); 

  }

  void SoDaBench_Top::initListener()
  {
    // setup the comm channel.
    std::string sock_basename = params.getServerSocketBasename(); 

    std::cerr << "SoDaBench_Top about to create client socket." << std::endl;
  
    // setup the client socket trying once every second for 60 seconds before we give up
    soda_bench = new SoDa::UD::ClientSocket(sock_basename + "_bcmd", 60);
    std::cerr << boost::format("SoDaBench_Top created client socket [%s_bcmd]\n") % sock_basename; 

    soda_fft = new SoDa::UD::ClientSocket(sock_basename + "_bwfall", 60);
    std::cerr << boost::format("SoDaBench_Top created client socket [%s_bwfall]\n") % sock_basename; 

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

  void SoDaBench_Top::setupServer()
  {
    // Startup the server process -- it should be in the same directory as
    // this program, unless an alternate server image was specified. 
    std::string myhome = findHome();

    // Now start the server
    std::string server = params.getServerName();
    std::string server_commandline_string; 
    if(server == "SoDaBenchServer") {
      // find it in our home directory
      server_commandline_string = myhome + "/SoDaBenchServer";
    }
    else {
      server_commandline_string = server; 
    }

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

  }

  void SoDaBench_Top::unsupportedEvent(const std::string & str) {
    std::cerr << "Unsupported event: " << str << std::endl; 
  }
  
  void SoDaBench_Top::OnOpenConfig( wxCommandEvent & event)
  {
    unsupportedEvent(std::string("OpenConfig"));
  }

  void SoDaBench_Top::OnSaveConfig( wxCommandEvent & event)
  {
    unsupportedEvent(std::string("SaveConfig"));
  }

  void SoDaBench_Top::OnSaveConfigAs( wxCommandEvent & event)
  {
    unsupportedEvent(std::string("SaveConfigAs"));
  }

  void SoDaBench_Top::OnQuit( wxCommandEvent & event)
  {
    Close();
  }

  void SoDaBench_Top::OnAbout( wxCommandEvent & event)
  {
    char * version_string = (char *) SDR_version_string.c_str(); 
    AboutDialog * ad = new AboutDialog(this, version_string);
    ad->ShowModal();
  }


  void SoDaBench_Top::OnUserGuide( wxCommandEvent & event)
  {
    unsupportedEvent(std::string("UserGuide"));
  }


  bool SoDaBench_Top::CreateSpectrumTrace(double * freqs, float * powers, unsigned int len)
  {
    if(pgram_plot == NULL) return false; 
    if(wfall_plot == NULL) return false;
  
    pgram_trace = new SoDaRadio_GUI::XYPlot::Trace(freqs, powers, len, 1, wxT(""));
    wfall_plot->RegisterBuffers(freqs, powers, len); 
    wxColor t1(0xff, 0x00, 0x00, 0x80); 
    pgram_plot->AddTrace(0, t1, pgram_trace);
    return true; 
  }


  void SoDaBench_Top::configureHardware(int rx_chan, int tx_chan,
			   double rx_min_freq, double rx_max_freq,
			   double tx_min_freq, double tx_max_freq)
  {
    // what kind of widget are we?
    num_tx_channels = tx_chan;
    num_rx_channels = rx_chan;

    if(num_tx_channels < 2) {
      // disable the sweeper output B.
      m_RFOutEna_B->Enable(false);
      m_OutPowerSliderB->Enable(false);
    }
  }

  void SoDaBench_Top::OnOutputPowerSel(wxScrollEvent & event) {
    // which channel?
    wxSlider * sl = (wxSlider *) event.GetEventObject();
    double val = sl->GetValue();
    if(sl == m_OutPowerSliderA) {
      SoDa::Command opowA(SoDa::Command::SET, SoDa::Command::TX_RF_GAIN, val); 
      sendMsg(&opowA); 
    }
    else {
      SoDa::Command opowB(SoDa::Command::SET, SoDa::Command::TX_RF_CHAN_GAIN, val, 2.0); 
      sendMsg(&opowB); 
    }
  }

  double SoDaBench_Top::getFreqSetting(wxTextCtrl * fv, wxChoice * unit) {
    double mul = 1.0; 
    wxString ustr = unit->GetStringSelection();
    if(ustr == wxT("kHz")) mul = 1.0e3;
    else if(ustr == wxT("MHz")) mul = 1.0e6;
    else if(ustr == wxT("GHz")) mul = 1.0e9;

    wxString sfreq = fv->GetValue();
    double freq;
    if(!sfreq.ToDouble(&freq)) {
      freq = 1.0;
      setFreqSetting(fv, unit, freq * mul);
    }

    freq = freq * mul;
    
    if(freq < 0.0) {
      freq = -1.0 * freq; 
      setFreqSetting(fv, unit, freq); 
    }
    
    return freq; 
  }

  void SoDaBench_Top::setFreqSetting(wxTextCtrl * fv, wxChoice * unit, double freq) {
    double mul = 1.0; 
    wxString ustr = unit->GetStringSelection();
    if(ustr == wxT("kHz")) mul = 1.0e3;
    else if(ustr == wxT("MHz")) mul = 1.0e6;
    else if(ustr == wxT("GHz")) mul = 1.0e9;

    wxString sfreq = wxString::Format(wxT("%f"), freq / mul);

    fv->SetValue(sfreq);
  }
  
  void SoDaBench_Top::OnFreqEnter(wxCommandEvent & event) {
    // which widget was it?
    wxObject * w = event.GetEventObject();

    double center_freq, span_freq; 
    
    
    if ((w == m_SpanFreqBox) || (w == m_CenterFreqBox) ||
	(w == m_SpanUnits) || (w == m_CenterUnits)) {
      center_freq = getFreqSetting(m_CenterFreqBox, m_CenterUnits);
      span_freq = getFreqSetting(m_SpanFreqBox, m_SpanUnits);
      
      start_freq = center_freq - (span_freq * 0.5);
      stop_freq = center_freq + (span_freq * 0.5);
      if(start_freq < 0.0) start_freq = 0.0; 
      setFreqSetting(m_StartFreqBox, m_StartUnits, start_freq);
      setFreqSetting(m_StopFreqBox, m_StopUnits, stop_freq);

    }
    else if ((w == m_StartFreqBox) || (w == m_StopFreqBox) ||
	     (w == m_StartUnits) || (w == m_StopUnits)) {
      start_freq = getFreqSetting(m_StartFreqBox, m_StartUnits);
      stop_freq = getFreqSetting(m_StopFreqBox, m_StopUnits);
      // set the center and span
      center_freq = (stop_freq + start_freq) * 0.5;
      span_freq = (stop_freq - start_freq);
      setFreqSetting(m_CenterFreqBox, m_CenterUnits, center_freq); 
      setFreqSetting(m_SpanFreqBox, m_SpanUnits, span_freq); 
    }

    if(start_freq < stop_freq) {
      m_StartFreqBox->SetBackgroundColour(*wxWHITE);
      m_StopFreqBox->SetBackgroundColour(*wxWHITE);
      sendSweepCommand();       
    }
    else {
      m_StartFreqBox->SetBackgroundColour(*wxRED);
      m_StopFreqBox->SetBackgroundColour(*wxRED);
    }

  }

  void SoDaBench_Top::OnFreqRangeSel(wxCommandEvent & event) {
    OnFreqEnter(event); 
  }

  void SoDaBench_Top::sendSweepCommand()
  {
    std::cerr << boost::format("Send Sweep command from %g to %g with step %g and dwell time %g\n")
	     % start_freq % stop_freq % step_freq % step_time; 
  }
}
