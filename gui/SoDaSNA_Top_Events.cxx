/*
  Copyright (c) 2012, Matthew H. Reilly (kb1vc)
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

#include "SoDaSNA_Top.h"
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <wx/wx.h>
#include "../src/Command.hxx"
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace SoDaSNA_GUI {
  
  void SoDaSNA_Top::OnAbout( wxCommandEvent& event )
  {
    AboutDialog * ad = new AboutDialog(this, SDR_version_string);
    ad->ShowModal(); 
  }

  void AboutDialog::OnAboutOK( wxCommandEvent & event)
  {
    if(IsModal()) EndModal(wxID_OK);
    else {
      SetReturnCode(wxID_OK);
      this->Show(false); 
    }
  }

  void NewConfigDialog::OnCreateConfigDefault( wxCommandEvent & event)
  {
    // get our home directory
    std::string homedir = getenv("HOME");
    std::string configdir = homedir + "/.SoDaRadio";
    std::string configfile = configdir + "/SoDaSNA.cfg";

    // does ${HOME}/.SoDaSNA exist?
    struct stat sb; 
    if(stat(configdir.c_str(), &sb) == 0) {
      // The file exists. 
      if (!S_ISDIR(sb.st_mode)) {
	// but it is NOT a directory!
	// Set an error.
	std::string ermsg = configdir + " is NOT a directory.\nPlease fix this, and try again.";
	m_StatusInfo->SetLabel(wxString(ermsg.c_str(), wxConvUTF8));
	return; 
      }
    }
    else {
      // create the directory?
      int stat = mkdir(configdir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
      if(stat < 0) {
	std::string ermsg = "Could not create " + configdir + "\nPlease fix this, and try again.";
	m_StatusInfo->SetLabel(wxString(ermsg.c_str(), wxConvUTF8));
	return; 
      }
    }

    radio->SaveConfig(wxString(configfile.c_str(), wxConvUTF8));

    radio->SetConfigFileName(wxString(configfile.c_str(), wxConvUTF8));
    
    m_StatusInfo->SetLabel(wxString(wxT("                                  \n                                  ")));    
    if(IsModal()) EndModal(wxID_OK);
    else {
      SetReturnCode(wxID_OK);
      this->Show(false); 
    }
  }
  
  void NewConfigDialog::OnDismissCreateConfigDefault( wxCommandEvent & event)
  {
    if(IsModal()) EndModal(wxID_OK);
    else {
      SetReturnCode(wxID_OK);
      this->Show(false); 
    }
  }
  
  void SoDaSNA_Top::OnOpenConfig( wxCommandEvent& event )
  {
    wxString defaultDir = wxT("~/.SoDa");
    wxString defaultFilename = wxT("SoDa.soda_cfg");
    wxString wildcard = wxT("SoDa Config files (*.soda_cfg)|*.soda_cfg");
    wxFileDialog dialog(this, wxT("Load Configuration File"), defaultDir, defaultFilename, wildcard, wxFD_OPEN);
  

    if (dialog.ShowModal() == wxID_OK) {
      wxString fname = dialog.GetPath();
      LoadConfig(fname);
    }
  }

  void SoDaSNA_Top::OnSaveConfig( wxCommandEvent& event )
  {
    if(save_config_file_name.length() == 0) {
      OnSaveConfigAs(event);
    }
    else {
      SaveConfig(save_config_file_name); 
    }
  }

  void SoDaSNA_Top::OnSaveConfigAs( wxCommandEvent& event )
  {
    std::cerr << "Got to \"OnSaveConfigAs\"" << std::endl; 

    std::cerr << "About to show save_config_as dialog" << std::endl; 

    if (save_config_dialog->Show() == wxID_OK) {
      std::cerr << "About to get path." << std::endl; 
      wxString fname = save_config_dialog->GetPath();
      std::cerr << "got the path: " << fname << std::endl; 
      save_config_file_name = fname; 
      SaveConfig(fname);
    }

    std::cerr << "Show modal returned ." << std::endl;

  }

  void SoDaSNA_Top::OnCalibrate( wxCommandEvent& event )
  {
    CalibrateDialog * cd = new CalibrateDialog(this, this);
    if(cd->ShowModal() == wxID_OK) {
      std::cerr << "Updating calibration settings." << std::endl; 
    }
    else {
      std::cerr << "Discarding new calibration settings." << std::endl;
    }
  }

  void SoDaSNA_Top::OnSampleUpdate( wxCommandEvent & event ) {
    
  }
  
  void SoDaSNA_Top::OnClose( wxCommandEvent& event )
  {
    debugMsg("Sending Radio Server a STOP command.");
    // This will cause the radio server to abort as well. 
    SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::STOP, 0);
    sendMsg(&ncmd);

    debugMsg("Stopping RadioListener thread.");
    // stop the radio listener thread.
    listener->stop();

    debugMsg("Passing close event up the chain.");
    event.Skip();
  }
  void SoDaSNA_Top::OnQuit( wxCommandEvent& event )
  {
    Close(); 
  }

  void SoDaSNA_Top::OnUserGuide( wxCommandEvent& event )
  {
  }
  void SoDaSNA_Top::OnFreqEnter( wxMouseEvent& event )
  {
    UpdatePlotAxes();
  }
  void SoDaSNA_Top::OnFreqEnter( wxCommandEvent& event )
  {
    UpdatePlotAxes();    
  }
  void SoDaSNA_Top::OnFreqRangeSel( wxCommandEvent& event )
  {
    UpdatePlotAxes();    
  }
  void SoDaSNA_Top::OnSweepSpeed( wxCommandEvent& event )
  {
  }

  void SoDaSNA_Top::OnSweepControl( wxCommandEvent& event )
  {
    wxRadioBox * w = (wxRadioBox *) event.GetEventObject(); 
    wxString res = w->GetString(w->GetSelection()); 
    if(res == wxString(wxT("Single"))) {
      sweep_mode = SINGLE_SWEEP; 
      debugMsg("Starting Single Sweep");
    }
    else if(res == wxString(wxT("Continuous"))) {
      sweep_mode = CONTINUOUS_SWEEP;
      debugMsg("Starting Continuous Sweep");
    }
    else {
      sweep_mode = NO_SWEEP;
      debugMsg("Turning sweep off");
    }

    if(sweep_mode == NO_SWEEP) {
      SoDa::Command txoff(SoDa::Command::SET, SoDa::Command::TX_STATE, 0);      
      sendMsg(&txoff);
    }
    else {
      doSweep(true);
    }
  }

  void SoDaSNA_Top::OnOutputPowerSel( wxScrollEvent& event )
  {
    wxSlider * w = (wxSlider *) event.GetEventObject();     
    double val = (double) w->GetValue();
    SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::TX_RF_GAIN_DB, 
		       val);

    sendMsg(&ncmd);
  }
  
  void SoDaSNA_Top::OnUpdateSNAPlot(wxCommandEvent & event)
  {
  }

  void SoDaSNA_Top::GetFreqSettings()
  {
    // get the center freq
    std::string cfstr(m_CenterFreqBox->GetValue().mb_str()); 
    double cfreq = boost::lexical_cast<double>(cfstr);
    std::cerr << "cfreq from CFB = " << cfreq << std::endl; 
    // its units
    cfreq = cfreq * multFromUnitsString(m_CenterUnits);
    std::cerr << " after mult = " << cfreq << std::endl; 
    // its span
    double span = unitsFromSpan(m_SpanMag);
    std::cerr << "span base = " << span << std::endl; 
    // its span units
    span = span * multFromUnitsString(m_SpanUnits);
    std::cerr << " after mult = " << span << std::endl; 

    // make sure the span is in range
    bool ok = true; 
    if((span > max_span) || (span < min_span)) {
      postErrorText((boost::format("Span frequency must be in the range %g to %g\n")
		     % min_span % max_span).str());
      ok = false; 
    }

    if((cfreq > max_cfreq) || (cfreq < min_cfreq)) {
      postErrorText((boost::format("Center frequency must be in the range %g to %g\n")
		     % min_cfreq % max_cfreq).str());
      ok = false; 
    }
    
    if(ok) {
      clearErrorText(); 

      std::cerr << " cfreq = " << cfreq << std::endl; 
      // first round the cfreq so that it makes sense in the span. 
      double per_block = span / 10.0; // this is the increment per block. 
      std::cerr << " per_block = " << per_block << std::endl; 
      double round_cf = round(cfreq / per_block);
      std::cerr << " round_cf = " << round_cf << std::endl;       
      display_cfreq = round_cf * per_block; 
      std::cerr << " display_cfreq = " << display_cfreq << std::endl; 

      display_start_freq = display_cfreq - per_block * 5.0;
      display_end_freq = display_cfreq + per_block * 5.0; 
    }

  }

  void SoDaSNA_Top::UpdatePlotAxes() 
  {
    GetFreqSettings();
    updateXYPlot(); 
  }

  void SoDaSNA_Top::doSweep(bool initial_sweep)
  {
    debugMsg("Starting sweep");
    // send a message to start the sweep. 
    
    if(sweep_mode != NO_SWEEP) {
      debugMsg("really going to try a sweep...");
      if(!initial_sweep && (sweep_mode == SINGLE_SWEEP)) {
	debugMsg("This was a single sweep.");
	sweep_mode = NO_SWEEP;
	// set the selection to IDLE
	m_SweepControl->SetSelection(0);
	// turn off the sweeper
	SoDa::Command txoff(SoDa::Command::SET, SoDa::Command::TX_STATE, 0);
	sendMsg(&txoff);	
      }
      else {
	// now send a message to the server to start a scan
	SoDa::Command txon(SoDa::Command::SET, SoDa::Command::TX_STATE, 1);
	
	double step_freq = (display_end_freq - display_start_freq) / 100.0; 
	double time_per_step = 1.15; 
	SoDa::Command sweep(SoDa::Command::SET, SoDa::Command::SNA_SCAN_START, 
			    display_start_freq, display_end_freq, step_freq, time_per_step); 
	debugMsg("Created the sweep messages.");
	sendMsg(&txon);
	sendMsg(&sweep); 
      }
    }
  }
}
