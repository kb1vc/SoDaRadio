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

#include "SoDaRadio_Top.h"
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <wx/wx.h>
#include "../src/Command.hxx"
#include "Navigation.hxx"
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace SoDaRadio_GUI {
  
  void SoDaRadio_Top::OnAbout( wxCommandEvent& event )
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
    std::string configfile = configdir + "/SoDa.soda_cfg";

    // does ${HOME}/.SoDaRadio exist?
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

    radio->SaveSoDaConfig(wxString(configfile.c_str(), wxConvUTF8));

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
  
  void SoDaRadio_Top::OnOpenConfig( wxCommandEvent& event )
  {
    wxString defaultDir = wxT("~/.SoDa");
    wxString defaultFilename = wxT("SoDa.soda_cfg");
    wxString wildcard = wxT("SoDa Config files (*.soda_cfg)|*.soda_cfg");
    wxFileDialog dialog(this, wxT("Load Configuration File"), defaultDir, defaultFilename, wildcard, wxFD_OPEN);
  

    if (dialog.ShowModal() == wxID_OK) {
      wxString fname = dialog.GetPath();
      LoadSoDaConfig(fname);
    }
  }

  void SoDaRadio_Top::OnSaveConfig( wxCommandEvent& event )
  {
    if(save_config_file_name.length() == 0) {
      OnSaveConfigAs(event);
    }
    else {
      SaveSoDaConfig(save_config_file_name); 
    }
  }

  void SoDaRadio_Top::OnSaveConfigAs( wxCommandEvent& event )
  {
    std::cerr << "Got to \"OnSaveConfigAs\"" << std::endl; 

    std::cerr << "About to show save_config_as dialog" << std::endl; 

    if (save_config_dialog->ShowModal() == wxID_OK) {
      std::cerr << "About to get path." << std::endl; 
      wxString fname = save_config_dialog->GetPath();
      std::cerr << "got the path: " << fname << std::endl; 
      save_config_file_name = fname; 
      SaveSoDaConfig(fname);
    }

    std::cerr << "Show modal returned ." << std::endl;

  }

  void SoDaRadio_Top::OnOpenLogfile( wxCommandEvent& event )
  {
    wxString defaultDir = wxT("~/.SoDa");
    wxString defaultFilename = wxT("SoDa.soda_log");
    wxString wildcard = wxT("SoDa Log files (*.soda_log)|*.soda_log");
    wxFileDialog dialog(this, wxT("Open/Create Log File"), defaultDir, defaultFilename, wildcard, wxFD_SAVE);

    if (dialog.ShowModal() == wxID_OK) {
      log_file_name = dialog.GetPath();
      OpenSoDaLog(log_file_name);
    }
  }

  bool SoDaRadio_Top::OpenSoDaLog(const wxString & logfilename)
  {
    wxString tmp = logfilename; 
    std::string lfn("asdf");
    lfn = tmp.mb_str();
    logdialog->openLog(lfn);
    m_ClueBar->SetStatusText(logfilename, 1);
    return true; 
  }

  void SoDaRadio_Top::OnClose( wxCloseEvent& event )
  {
    debugMsg("Closing logfile.");
    // ask the logger to write the final log
    logdialog->closeLog();

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

  void SoDaRadio_Top::OnQuit( wxCommandEvent& event )
  {
    Close(); 
  }

  void SoDaRadio_Top::OnSelectPage( wxNotebookEvent& event )
  {
    if(SpectrumDisplay->GetSelection() == 0) {
      // this is the waterfall display
      SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::SPEC_AVG_WINDOW,
			 spect_config->getWfallWindowLen());
      sendMsg(&ncmd);
    }
    else {
      // this is the periodogram display
      SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::SPEC_AVG_WINDOW,
			 spect_config->getPerWindowLen()); 
      sendMsg(&ncmd);
    }
  }

  void SoDaRadio_Top::OnWFallFreqSel( wxMouseEvent& event )
  {
  }

  void SoDaRadio_Top::OnPeriodogramFreqSel( wxMouseEvent& event )
  {
  }

  void SoDaRadio_Top::OnPerWindowLenUpdate( wxScrollEvent & event )
  {
    wxSlider * w = (wxSlider *) event.GetEventObject();

    if(SpectrumDisplay->GetSelection() == 1)  {
      SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::SPEC_AVG_WINDOW,
		       w->GetValue()); 
      sendMsg(&ncmd);
    }
  }

  void SoDaRadio_Top::OnWfallWindowLenUpdate( wxScrollEvent & event )
  {
    wxSlider * w = (wxSlider *) event.GetEventObject();

    if(SpectrumDisplay->GetSelection() == 0)  {
      SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::SPEC_AVG_WINDOW,
		       w->GetValue()); 
      sendMsg(&ncmd);
    }
  }

  void SoDaRadio_Top::OnMenuConfigTXAudio(wxCommandEvent & event) {
    if(tx_audio_config != NULL) tx_audio_config->Show();
  }

  void SoDaRadio_Top::OnMenuConfigSpect(wxCommandEvent & event) {
    spect_config->Show();
  }
  void SoDaRadio_Top::OnOpenSpectConfig(wxMouseEvent & event) {
    spect_config->Show();
  }
  

  void SoDaRadio_Top::OnScrollSpeedUpdate( wxScrollEvent & event) {
    wxSlider * w = (wxSlider *) event.GetEventObject();
    int val = (int) w->GetValue();
    debugMsg(boost::format("About to send a scroll speed message, param = %d.")
	     % val);
    SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::SPEC_UPDATE_RATE,
		       w->GetValue()); 
    sendMsg(&ncmd);
    debugMsg("sent a scroll speed message.");
  }
  
  void SoDaRadio_Top::OnTerminateTX( wxCommandEvent& event )
  {
    // we got to the end of a CW string --
    // if we're transmitting, terminate TX mode... 
    if(tx_on) OnTXOnOff(event); 
  }

  void SoDaRadio_Top::OnTXOnOff( wxCommandEvent& event )
  {
    if(tx_on) {
      // set the label background color to GREY
      m_PTT->SetBackgroundColour(default_button_bg_color);
      m_PTT->SetLabel(wxString(wxT(" TX\nOFF")));
      // send the command string to turn off the TX
      SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::TX_STATE, 0);
      sendMsg(&ncmd);
      tx_on = false; 
    }
    else {
      // set the label background color to RED
      m_PTT->SetBackgroundColour(*wxRED);
      m_PTT->SetLabel(wxString(wxT(" TX\n ON")));
      // send the command string to turn on the TX
      SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::TX_STATE, 1);
      sendMsg(&ncmd);
      tx_on = true;
    }
  }

  void SoDaRadio_Top::OnSendText( wxCommandEvent & event)
  {
    wxTextCtrl * w = (wxTextCtrl *) event.GetEventObject();

    wxString cwstr = w->GetValue(); 
    w->SetValue(wxT(""));
    // now push the new text on the outbound text list.

    cwstr.append(wxT(" "));

    sendCWText(cwstr, 1, true);
  }

  void SoDaRadio_Top::sendCWMarker( int marker_id )
  {
    SoDa::Command cmdptr(SoDa::Command::SET, SoDa::Command::TX_CW_MARKER, marker_id);
    sendMsg(&cmdptr); 
  }

  void SoDaRadio_Top::sendCWText( const wxString & cwstr, int repeat_count, bool append_cr)
  {
    int rc;

    wxString mycwstr = cwstr; 
    if(repeat_count > 1) mycwstr = mycwstr + wxT(" "); 
    std::string cwststr(mycwstr.mb_str());
  
    for(rc = 0; rc < repeat_count; rc++) {
      // write the text to the CW transmit text block.
      m_CWTextOutbound->SetInsertionPointEnd();
      m_CWTextOutbound->AppendText(mycwstr);
      if(append_cr && (repeat_count == 1)) m_CWTextOutbound->AppendText(wxT("\n"));

      // and send it in chunks.  
      const char * cwbuf = cwststr.c_str();
      // scan the cwbuf to make sure that there are no control characters in it...
      // we use <ETX> (^C) to signal the end of a buffer. 
      int i;
      for(i = 0; i < mycwstr.Len(); i++) {
	if(!isprint(mycwstr[i])) mycwstr[i] = ' '; 
      }
    
      for(i = 0; i < mycwstr.Len(); i += SoDa::Command::getMaxStringLen()) {
	SoDa::Command cmdptr(SoDa::Command::SET, SoDa::Command::TX_CW_TEXT, &(cwbuf[i]));
	sendMsg(&cmdptr); 
      }
    }
    if(append_cr && (repeat_count != 1)) m_CWTextOutbound->AppendText(wxT("\n"));
  }

  void SoDaRadio_Top::OnCWControl( wxCommandEvent & event)
  {
    wxButton * button = (wxButton *) event.GetEventObject();
    wxCommandEvent nullCE; 
    if(!dead_carrier) {  
      if(button == m_CWsendCarrier) {
	if(!tx_on) OnTXOnOff(nullCE); 
	SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::TX_BEACON, 1);
	sendMsg(&ncmd);
	dead_carrier = true;
      }
    }
    else { // dead carrier is on -- turn it off... 
      SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::TX_BEACON, 0);
      sendMsg(&ncmd);
      dead_carrier = false;
      if(tx_on) OnTXOnOff(nullCE); 
    }

    if(button == m_CWsendEx) {
      if(!tx_on) OnTXOnOff(nullCE); 
      // send full exchange
      sendCWText(to_callsign, m_RepeatCount->GetValue(), false);
      sendCWText(wxT(" de "), 1, false);
      // now send our info... 
      button = m_CWsendInfo; 
    }

    if(button == m_CWsendInfo) {
      if(!tx_on) OnTXOnOff(nullCE); 
      sendCWText(from_callsign, m_RepeatCount->GetValue(), false);
      sendCWText(wxT(" "), 1, false); 
      sendCWText(from_grid, m_RepeatCount->GetValue(), false);
      sendCWText(wxT(" "), 1, false); 
    }
  
    if(button == m_CWsendCall) {
      if(!tx_on) OnTXOnOff(nullCE); 
      // repeat my call
      sendCWText(from_callsign, m_RepeatCount->GetValue()); 
    }

    if(button == m_CWsendGrid) {
      // repeat my grid
      if(!tx_on) OnTXOnOff(nullCE); 
      sendCWText(from_grid, m_RepeatCount->GetValue()); 
    }

    if(button == m_CWsendQSL) {
      if(!tx_on) OnTXOnOff(nullCE); 
      // send a QSL  -- repeat as often as we need to
      sendCWText(wxT("R R R  QSL QSL QSL"), 1, false);
    }
      
    if(button == m_CWsendBK) {
      if(!tx_on) OnTXOnOff(nullCE); 
      // send a break and also terminate TX... 
      sendCWText(wxT("_bk "), 1, true);
      // terminate TX
      sendCWMarker(1); 
    }
  
    if(button == m_CWsend73) {
      if(!tx_on) OnTXOnOff(nullCE); 
      // send a break and also terminate TX... 
      sendCWText(wxT(" TNX es 73 de "), 1, true);
      sendCWText(from_callsign, m_RepeatCount->GetValue());
      sendCWText(wxT("_sk "), 1, true);
      // terminate TX
      sendCWMarker(1);     
    }
  
    if(button == m_CWsendV) {
      if(!tx_on) OnTXOnOff(nullCE); 
      // send Vs
      sendCWText(wxT("VVVVVVVVVVVV"), m_RepeatCount->GetValue());
    }
  }

  void SoDaRadio_Top::OnAntChoice( wxCommandEvent& event )
  {
    wxString ant_string = m_AntChoice->GetStringSelection();
    setRXAnt(std::string(ant_string.mb_str()));
  }

  void SoDaRadio_Top::OnModeChoice( wxCommandEvent& event )
  {
    // m_ModeBox
    wxString mode_string = m_ModeBox->GetStringSelection();

    SoDa::Command::ModulationType mt;
  
    if(mode_string == wxT("CW_L")) {
      mt = SoDa::Command::CW_L;
      cw_mode = true;
      cw_upper = false; 
    }
    if(mode_string == wxT("CW_U")) {
      mt = SoDa::Command::CW_U;
      cw_mode = true; 
      cw_upper = true;
    }
    if(mode_string == wxT("LSB")) {
      mt = SoDa::Command::LSB;
      cw_mode = false;
    }
    if(mode_string == wxT("USB")) {
      mt = SoDa::Command::USB;
      cw_mode = false;
    }
    if(mode_string == wxT("AM")) {
      mt = SoDa::Command::AM;
      cw_mode = false;
    }
    if(mode_string == wxT("WBFM")) {
      mt = SoDa::Command::WBFM;
      cw_mode = false;
    }
    if(mode_string == wxT("NBFM")) {
      mt = SoDa::Command::NBFM;
      cw_mode = false;
    }

    modulation_type = mt;
    
    SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::RX_MODE, (int) mt);
    SoDa::Command ncmd2(SoDa::Command::SET, SoDa::Command::TX_MODE, (int) mt);
    sendMsg(&ncmd);
    sendMsg(&ncmd2);

    // update the spectrum display.
    updateMarkers();
  }

  void SoDaRadio_Top::OnAFBWChoice( wxCommandEvent& event )
  {
    int sel = m_AFBWChoice->GetCurrentSelection();
    SoDa::Command::AudioFilterBW bw;
    switch (sel) {
    case 0:
      bw = SoDa::Command::BW_100;
      break; 
    case 1:
      bw = SoDa::Command::BW_500;
      break; 
    case 2:
      bw = SoDa::Command::BW_2000;
      break; 
    case 3:
    default:
      bw = SoDa::Command::BW_6000;
      break; 
    }

    audio_filter = bw; 

    // update the marker on the spectrum
    updateMarkers();
    
    SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::RX_AF_FILTER, (int) bw);
    sendMsg(&ncmd);
  
  }

  void SoDaRadio_Top::OnTXRXLock( wxCommandEvent& event )
  {
    tx_rx_locked = m_TXRXLocked->GetValue();
  }

  void SoDaRadio_Top::CheckLockedTuning(bool rx_updatep)
  {
    if(tx_rx_locked) {
      // the tx and rx widgets are locked.
      if(rx_frequency != tx_frequency) {
	if(rx_updatep) {
	  // we already updated the rx register.. now do tx.
	  UpdateTXFreq(rx_frequency);
	}
	else {
	  // we already updated the Tx register.. now do rx.
	  UpdateRXFreq(tx_frequency);
	}
      }
    }
  }

  void TuningDialog::OnDigitUp( wxCommandEvent& event )
  {
    wxWindow * w;
    w = (wxWindow*) event.GetEventObject();

    // find w in the map.
    TunerDigit * td;

    if(rx_tuner.find(w) != rx_tuner.end()) {
      td = rx_tuner[w];
      td->up();
      rx[0]->update(0.0);

      radio_top->UpdateRXFreq(radio_top->rx_frequency);
      radio_top->CheckLockedTuning(true);
    }
    else if(tx_tuner.find(w) != tx_tuner.end()) {
      td = tx_tuner[w]; 
      td->up(); 
      tx[0]->update(0.0);
      radio_top->UpdateTXFreq(radio_top->tx_frequency);
      radio_top->CheckLockedTuning(false);
    }
    else {
    }
  }

  void TuningDialog::OnDigitDown( wxCommandEvent& event )
  {
    wxWindow * w;
    w = (wxWindow*) event.GetEventObject();

    // find w in the map.
    TunerDigit * td;

    if(rx_tuner.find(w) != rx_tuner.end()) {
      td = rx_tuner[w];
      td->down(); 
      rx[0]->update(0.0); 
      radio_top->UpdateRXFreq(radio_top->rx_frequency);
      radio_top->CheckLockedTuning(true);
    }
    else if(tx_tuner.find(w) != tx_tuner.end()) {
      td = tx_tuner[w]; 
      td->down();
      tx[0]->update(0.0); 
      radio_top->UpdateTXFreq(radio_top->tx_frequency);
      radio_top->CheckLockedTuning(false);
    }
    else {
    }
  }

  void TuningDialog::OnTuningDone(wxCommandEvent & event)
  {
    // close the tuning dialog.
    if(IsModal()) {
      EndModal(wxID_OK);
    }
    else {
      SetReturnCode(wxID_OK);
      this->Show(false);
    }
  }

  void ControlsDialog::OnCtrlDone(wxCommandEvent & event)
  {
    // close the dialog.
    if(IsModal()) {
      EndModal(wxID_OK);
    }
    else {
      SetReturnCode(wxID_OK);
      this->Show(false);
    }
  }

  void TuningDialog::OnCopyTXtoRX( wxCommandEvent& event )
  {
    radio_top->OnCopyTXtoRX(event);
    rx[0]->newFreq(); 
  }


  void SoDaRadio_Top::OnCopyTXtoRX( wxCommandEvent& event )
  {
    last_rx_frequency = rx_frequency;
    UpdateRXFreq(tx_frequency);
  }


  void TuningDialog::OnCopyRXtoTX( wxCommandEvent& event )
  {
    radio_top->OnCopyRXtoTX(event);
    tx[0]->newFreq(); 
  }

  void SoDaRadio_Top::OnCopyRXtoTX( wxCommandEvent& event )
  {
    last_tx_frequency = tx_frequency;
    UpdateTXFreq(rx_frequency);
  }

  void ControlsDialog::OnTXPower( wxScrollEvent & event)
  {
    int itxpow_out = m_TXPower->GetValue();
    radio_top->tx_rf_outpower = powerToTXSetting((float) itxpow_out);
    SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::TX_RF_GAIN, radio_top->tx_rf_outpower); 
    radio_top->sendMsg(&ncmd);
  }

  void TuningDialog::OnLastTX( wxCommandEvent& event )
  {
    radio_top->OnLastTX(event);
    tx[0]->newFreq(); 
  }

  void SoDaRadio_Top::OnLastTX( wxCommandEvent& event )
  {
    UpdateTXFreq(last_tx_frequency);   
    CheckLockedTuning(false);
  }


  void TuningDialog::OnLastRX( wxCommandEvent& event )
  {
    radio_top->OnLastRX(event);
    rx[0]->newFreq(); 
  }

  void SoDaRadio_Top::OnLastRX( wxCommandEvent& event )
  {
    UpdateRXFreq(last_rx_frequency);
    CheckLockedTuning(true);
  }

  void ControlsDialog::OnCWSpeed(wxScrollEvent & event)
  {
    int ispd = m_CWSpeed->GetValue();
    SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::TX_CW_SPEED, ispd);
    radio_top->sendMsg(&ncmd); 
  }

  void ControlsDialog::OnSTGainScroll(wxScrollEvent & event)
  {
    int igain = m_STGain->GetValue();
    double  gain = ((double) igain); 
    SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::RX_AF_SIDETONE_GAIN, gain);
    radio_top->sendMsg(&ncmd); 
  }

  void SoDaRadio_Top::OnAFGainScroll(wxScrollEvent & event)
  {
    int igain = m_AFGain->GetValue();
    double dgain = ((double) igain) ;
    double gain = 50.0 * (log10(dgain) / log10(50.0)); 
    SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::RX_AF_GAIN, gain);
    sendMsg(&ncmd); 
  }

  void SoDaRadio_Top::OnRFGainScroll(wxScrollEvent & event)
  {
    int igain = m_RFGain->GetValue(); 
    double gain = ((double) igain);
    SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::RX_RF_GAIN, gain);
    sendMsg(&ncmd); 
  }


  void SoDaRadio_Top::OnSetFromCall(wxCommandEvent & event)
  {
    wxTextEntryDialog dialog(this,
			     wxT("Enter This Station's Call Sign"),
			     wxT("Station Call Sign"),
			     wxT(""),
			     wxOK | wxCANCEL);

    if(dialog.ShowModal() == wxID_OK) {
      from_callsign = dialog.GetValue();
      // now set the from call field. 
    }
  }

  void SoDaRadio_Top::OnSetFromGrid(wxCommandEvent & event)
  {
    wxTextEntryDialog dialog(this,
			     wxT("Enter This Station's Current Grid Location"),
			     wxT("Station QTH"),
			     wxT(""),
			     wxOK | wxCANCEL);

    if(dialog.ShowModal() == wxID_OK) {
      from_grid = dialog.GetValue();
      //    if(CheckGridSquare(from_grid.mb_str(wxConvUTF8)) != 0) {
      std::string fg = std::string(from_grid.mb_str()); 
      if(CheckGridSquare(fg) != 0) {
	m_MyGrid->SetLabel(wxT("????"));
	from_grid = wxT("");
      }
      else {
	// now set the from grid field.
	m_MyGrid->SetLabel(from_grid);
	if(to_grid.length() != 0) {
	  UpdateNavigation(); 
	}
      }
    }
  }

  void SoDaRadio_Top::OnQSOMenuSet( wxCommandEvent & event)
  {
    int id = event.GetId();
    if(id == ID_GOTOCALL) {
      m_ToCall->SetFocus();
    }
    else if(id == ID_GOTOGRID) {
      m_ToGrid->SetFocus();
    }
    else if(id == ID_GOTOLOG) {
      m_LogCommentBox->SetFocus();
    }
    else if(id == ID_GOTOMSG) {
      m_CWTextEntry->SetFocus();
    }
    else if(id == ID_TOGGLETX) {
      wxCommandEvent nullCE; 
      OnTXOnOff(nullCE); 
    }
  }


  void SoDaRadio_Top::OnEditLog( wxCommandEvent& event)
  {
    logdialog->Show();
    logdialog->scrollToBottom(); 
  }

  void SoDaRadio_Top::OnLogContact( wxCommandEvent& event )
  {
    // print the date
    wxDateTime now = wxDateTime::Now();

    wxString mode = getModeString();
    wxString timedate = now.Format(wxT(" %F %H:%M:%S"), wxDateTime::GMT0);
    wxString fndate = now.Format(wxT("_%F_%Hx%Mx%S.png"), wxDateTime::GMT0);
    logdialog->SaveContact(timedate,
			   from_callsign, from_grid, to_callsign, to_grid,
			   mode, 
			   tx_frequency, rx_frequency,
			   m_RangeText->GetLabel());
    wxString rx_freqstring = wxString::Format(wxT("RX freq: %9.4f"), rx_frequency * 1.0e-6);
    wfall_plot->Print(to_callsign + fndate,
		      to_callsign + wxString(wxT(" ")) +
		      to_grid + wxString(wxT(" ")) +
		      from_grid +
		      timedate,
		      rx_freqstring);
  }

  void LogDialog::saveLog()
  {
    // for each row, dump it in a CSV.
    if(logst.is_open()) {
      logst.seekg(0);
      int i, j;
      for(i = 0; i < m_LogGrid->GetNumberRows(); i++) {
	for(j = 0; j < m_LogGrid->GetNumberCols(); j++) {
	  if(j != 0) logst << "`"; 
	  logst << m_LogGrid->GetCellValue(i, j).mb_str(); 
	}
	logst << std::endl; 
      }
    }
  }

  void LogDialog::readLog(const std::string & basename)
  {
    std::ifstream inlog;
    inlog.open(basename.c_str(), std::fstream::in);

    if(inlog.is_open()) {
      // read each line into the table.
      int row = 0; 
      while(!inlog.eof()) {
	std::string strbuf; 
	getline(inlog, strbuf, '\n');
	if(strbuf.length() != 0) {
	  std::vector<std::string> elis;
	  boost::split(elis, strbuf, boost::is_any_of("`"));
      
	  int col = 0;
	  for(std::vector<std::string>::iterator el = elis.begin();
	      el != elis.end();
	      ++el) {
	    wxString v((*el).c_str(), wxConvUTF8);
	    if(row >= m_LogGrid->GetNumberRows()) {
	      m_LogGrid->AppendRows(1 + (row - m_LogGrid->GetNumberRows()));
	    }
	    m_LogGrid->SetCellValue(row, col, v);
	    col++; 
	  }
	  row++;
	}
      }
    }
  
    inlog.close();
  }


  void LogDialog::SaveContact(const wxString & time,
			      const wxString & from_call,
			      const wxString & from_grid,
			      const wxString & to_call,
			      const wxString & to_grid,
			      const wxString & mode,
			      double tx_freq,
			      double rx_freq,
			      const wxString & dist)
  {
    // first append a row to the grid.
    m_LogGrid->AppendRows(1);
    int currow = getNumEntries() - 1;

    wxString tx_f_str = wxString::Format(wxT("%12f"), tx_freq);
    wxString rx_f_str = wxString::Format(wxT("%12f"), rx_freq);

    m_LogGrid->SetCellValue(currow, 0, time);
    m_LogGrid->SetCellValue(currow, 1, from_call);
    m_LogGrid->SetCellValue(currow, 2, from_grid);
    m_LogGrid->SetCellValue(currow, 3, to_call);
    m_LogGrid->SetCellValue(currow, 4, to_grid);
    m_LogGrid->SetCellValue(currow, 5, mode);
    m_LogGrid->SetCellValue(currow, 6, tx_f_str); 
    m_LogGrid->SetCellValue(currow, 7, rx_f_str); 
    m_LogGrid->SetCellValue(currow, 8, dist);

    // if the chkp log is open, write to it.
    if(ckpst.is_open()) {
      ckpst
	<< time.mb_str() << "`"
	<< from_call.mb_str() << "`"
	<< from_grid.mb_str() << "`"
	<< to_call.mb_str() << "`"
	<< to_grid.mb_str() << "`"
	<< mode.mb_str() << "`"
	<< tx_f_str.mb_str() << "`"
	<< rx_f_str.mb_str() << "`"
	<< dist.mb_str() << std::endl; 
    }
  }

  void SoDaRadio_Top::OnSaveComment( wxCommandEvent& event)
  {
    wxTextCtrl * w = (wxTextCtrl *) event.GetEventObject(); 
    logdialog->SaveComment(w->GetValue());
    w->SetValue(wxT(""));
  }

  void LogDialog::SaveComment(const wxString & comment)
  {
    // first append a row to the grid.
    m_LogGrid->AppendRows(1);
    int currow = getNumEntries() - 1;

    m_LogGrid->SetCellValue(currow, 9, comment);
  }

  void LogDialog::OnLogOK( wxCommandEvent& event)
  {
    saveLog(); 
    if(IsModal() ) {
      EndModal(wxID_OK);
    }
    else {
      SetReturnCode(wxID_OK);
      this->Show(false); 
    }
  
  }

  void LogDialog::OnLogCellChange( wxGridEvent& event)
  {
  }

  void SoDaRadio_Top::SaveCurrentBand() {
    if(current_band != NULL) {
      current_band->last_rx_freq = rx_frequency * 1e-6;
      current_band->last_tx_freq = tx_frequency * 1e-6;

      current_band->af_gain = m_AFGain->GetValue();
      current_band->rf_gain = m_RFGain->GetValue();
      current_band->af_bw = m_AFBWChoice->GetSelection();

      current_band->tx_rf_outpower = tx_rf_outpower;
      current_band->tx_rx_locked = tx_rx_locked;
    }
  }
  
  void SoDaRadio_Top::SetCurrentBand(SoDaRadio_Band * band)
  {
    // save the old band's last tx/rx and other settings
    SaveCurrentBand();
    
    // now we have a band pointer -- save it
    current_band = band;

    if(band->transverter_mode) {
      actual_lo_base_freq = nominal_lo_base_freq = band->transverter_lo_freq * 1e6; 
      lo_multiplier = band->transverter_multiplier;
      setLOOffset(0.0);
      if(band->transverter_local_lo) {
	debugMsg(boost::format("Enable Transverter LO freq = %lf power = %lf") % actual_lo_base_freq % 1.0); 
	// send two messages -- set the LO freq and power
	SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::TVRT_LO_CONFIG,
			   actual_lo_base_freq, 1.0);
	sendMsg(&ncmd);

	// enable the LO
	SoDa::Command ncmd2(SoDa::Command::SET, SoDa::Command::TVRT_LO_ENABLE);
	sendMsg(&ncmd2);
      }
      else {
	// disable the LO
	SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::TVRT_LO_DISABLE);
	sendMsg(&ncmd);
      }
    }
    else {
      actual_lo_base_freq = nominal_lo_base_freq = 0.0;
      lo_multiplier = 0.0;
      setLOOffset(0.0);
      // disable the LO if any
      SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::TVRT_LO_DISABLE);
      sendMsg(&ncmd);
    }

    m_PTT->Enable(band->enable_transmit);
    m_CWsendEx->Enable(band->enable_transmit);
    m_CWsendInfo->Enable(band->enable_transmit);    
    m_CWsendCall->Enable(band->enable_transmit);    
    m_CWsendGrid->Enable(band->enable_transmit);    
    m_CWsendQSL->Enable(band->enable_transmit);    
    m_CWsendBK->Enable(band->enable_transmit);    
    m_CWsend73->Enable(band->enable_transmit);    
    m_CWsendV->Enable(band->enable_transmit);    
    m_CWsendCarrier->Enable(band->enable_transmit);
    

    // set the mode select
    wxCommandEvent nullCE;
    wxScrollEvent nullSE;
    m_ModeBox->SetStringSelection(wxString::FromUTF8(band->default_mode.c_str()));
    OnModeChoice(nullCE);

    // update the spectrum freq.
    UpdateCenterFreq(band->last_rx_freq * 1e6);
    OnPerBandSpread(nullCE);

    // the the antenna
    debugMsg(boost::format("Setting ant choice to [%s]\n") % band->rx_antenna_choice);
    setRXAnt(band->rx_antenna_choice);

    // set the transmit parameters
    controls->setTXPower(band->tx_rf_outpower);
    tx_rx_locked = band->tx_rx_locked;
    OnTXRXLock(nullCE);
    
    // now the af gain params
    // these are the gain controls.
    m_AFGain->SetValue(band->af_gain);
    m_AFBWChoice->SetSelection(band->af_bw);
    OnAFGainScroll(nullSE);
    OnAFBWChoice(nullCE);

    // set the RF gain
    m_RFGain->SetValue(band->rf_gain);
    OnRFGainScroll(nullSE);
    
    // now -- set the last tx/rx/freq
    UpdateRXFreq(band->last_rx_freq * 1e6);
    UpdateTXFreq(band->last_tx_freq * 1e6);
  }
  
  void SoDaRadio_Top::OnBandSelect( wxCommandEvent& event)
  {
    // find out which choice we made.
    wxObject * m = event.GetEventObject();
    // The band select wxID needs to be mapped to the index into the table. 
    int ev_id = event.GetId(); 
    if(wxID_to_band_idx_map.find(ev_id) == wxID_to_band_idx_map.end()) {
      std::cerr << boost::format("Couldn't find Band Index from window ID %d\n") % ev_id; 
      return; 
    }

    SoDaRadio_Band * newband = bandset->getByIndex(wxID_to_band_idx_map[ev_id]); 
    std::string band_name = newband->getName(); 
    
    // set the new band
    SetCurrentBand(newband);
  }

  void SoDaRadio_Top::OnTXAudioSel(wxCommandEvent & event, bool disable_noise) 
  {
    bool sel_mic = true; 
    if(!disable_noise) {
      wxRadioBox * m = (wxRadioBox* ) event.GetEventObject();
      int sel = m->GetSelection(); 
      sel_mic = (sel != 1); // 0 is the default (mic) selection. 
    }

    // now send out the command.
    int insel; 
    if(sel_mic) insel = SoDa::Command::MIC; 
    else insel = SoDa::Command::NOISE; 

    SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::TX_AUDIO_IN, insel); 
    sendMsg(&ncmd);
  }

  void SoDaRadio_Top::OnTXAudioFilterEnable(wxCommandEvent & event)
  {
    wxCheckBox * m = (wxCheckBox* ) event.GetEventObject();    
    int ena = m->IsChecked() ? 1 : 0; 

    SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::TX_AUDIO_FILT_ENA, ena); 
    sendMsg(&ncmd);
  }
 
  
  void SoDaRadio_Top::OnConfigBand( wxCommandEvent& event)
  {
    // init the list of configured bands in the bandconf dialog
    bandconf->initBandList(bandset);
    bandconf->clearTextBoxes(); 
    bandconf->Show();
  }

  void TuningDialog::OnTransvLOCal( wxCommandEvent & event)
  {
    SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::LO_CHECK,
		       radio_top->nominal_lo_base_freq);
    radio_top->sendMsg(&ncmd);
  }

  void SoDaRadio_Top::setLOOffset(double v)
  {
    wxMutexLocker lock(ctrl_mutex);
    actual_lo_base_freq = nominal_lo_base_freq + v; 
    tx_transverter_offset = actual_lo_base_freq * lo_multiplier; 
    rx_transverter_offset = actual_lo_base_freq * lo_multiplier;
  }


  void TuningDialog::OnExtRefEna( wxCommandEvent & event)
  {
    bool ena = m_ExtRefEn->GetValue();
    SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::CLOCK_SOURCE, ena ? 1 : 0);
    radio_top->sendMsg(&ncmd);
  }

  void SoDaRadio_Top::OnNewToCall( wxCommandEvent & event)
  {
    wxTextCtrl * w = (wxTextCtrl *) event.GetEventObject(); 

    to_callsign = w->GetValue();


  }

  void SoDaRadio_Top::OnNewToGridEnter( wxCommandEvent & event)
  {
    OnNewToGrid(event);
    m_ToCall->SetFocus();
  }

  void SoDaRadio_Top::OnNewToGrid( wxCommandEvent & event)
  {
    wxTextCtrl * w = (wxTextCtrl *) event.GetEventObject(); 
    wxString wsgrid = w->GetValue();

    std::string tg = std::string(wsgrid.mb_str());

    if(CheckGridSquare(tg) == 0) {
      to_grid = wsgrid;
      w->SetForegroundColour(*wxBLACK);
    }
    else {
      w->SetForegroundColour(*wxRED);
      // w->SetValue(wxT(""));
      // to_grid = wxT("");
      return; 
    }
  
    UpdateNavigation(); 

  }


  void SoDaRadio_Top::SetRXFreqDisp(double freq, bool display_only)
  {
    last_rx_frequency = rx_frequency;
    // adjust freq based on mode
    if(cw_mode) {
      // if we're upper sideband, depress the freq.... 
      freq = freq + (cw_upper ? -500.0 : 500.0); 
    }
  
    UpdateRXFreq(freq, display_only);
    tuner->newRXFreq();
    if(tx_rx_locked) {
      UpdateTXFreq(freq, display_only);
    }
  }

  void SoDaRadio_Top::SetTXFreqDisp(double freq, bool display_only)
  {
    last_tx_frequency = tx_frequency;
    // adjust freq based on mode
    if(cw_mode) {
      // if we're upper sideband, depress the freq.... 
      freq = freq + (cw_upper ? -500.0 : 500.0); 
    }
  
    UpdateTXFreq(freq, display_only);
    tuner->newTXFreq();
    if(tx_rx_locked) {
      UpdateRXFreq(freq, display_only);
    }
  }

  void SoDaRadio_Top::UpdateNavigation()
  {
  
    if((from_grid.length() != 0) && (to_grid.length() != 0)) {
      float bearing, rbearing, distance;
      std::string fg = std::string(from_grid.mb_str());
      std::string tg = std::string(to_grid.mb_str());
      if(GetBearingDistance(fg,
			    tg, 
			    bearing, rbearing, distance) == 0) {
	m_BearingText->SetLabel(wxString::Format(wxT("%3.0f T"), bearing));
	m_RevBearingText->SetLabel(wxString::Format(wxT("%3.0f T"), rbearing));
	m_RangeText->SetLabel(wxString::Format(wxT("%5.0f km"), distance));
      }
      else {
	std::cerr << "problem with get bearing" << std::endl; 
      }
    }
  }

  void SoDaRadio_Top::UpdateAxes()
  {
    double xmin, xmax, ymin, ymax;
    xmin = spectrum_center_freq - (spectrum_bandspread / 2);
    xmax = spectrum_center_freq + (spectrum_bandspread / 2);
    ymax = spectrum_y_reflevel;
    ymin = ymax - 10.0 * spectrum_y_scale;
    debugMsg(boost::format("In UpdateAxes with x = [%g,%g] y=[%g,%g]\n")
	     % xmin % xmax % ymin % ymax);
    debugMsg(boost::format("pgram_plot = %p  wfall_plot = %p\n") % pgram_plot % wfall_plot);
    pgram_plot->SetScale(xmin, xmax, ymin, ymax);
    wfall_plot->SetScale(xmin, xmax);
    updateMarkers(); 

    debugMsg("Sending axis update message\n");
    // now tell the radio
    SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::SPEC_CENTER_FREQ,
		       applyRXTVOffset(spectrum_center_freq));
    sendMsg(&ncmd); 
    debugMsg("Sent axis update message\n");
  }

  void SoDaRadio_Top::SetSpectrum(double bandspread)
  {
    spectrum_bandspread = spect_config->setBandSpread(bandspread);
    debugMsg(boost::format("got bandspread = %g\n") % spectrum_bandspread); 
    UpdateAxes();
  }

  double SoDaRadio_Top::GetSpread()
  {
    return spect_config->getBandSpread(); 
  }

  void SoDaRadio_Top::SetYScale(double v)
  {
    spect_config->setdBScale(v); 
  }

  double SoDaRadio_Top::GetYScale()
  {
    return spect_config->getdBScale(); 
  }

  void SoDaRadio_Top::OnPerCFreqStep( wxSpinEvent & event)
  {
    wxSpinCtrl * w = (wxSpinCtrl*) event.GetEventObject();
    // did it go up or down?
    unsigned long new_cfreq = w->GetValue();
    if(new_cfreq != last_cfreq) {
      // take the difference and make new steps.
      unsigned long diff = new_cfreq - last_cfreq;
      last_cfreq += diff * cfreq_step;
      unsigned long min = w->GetMin(); 
      unsigned long max = w->GetMax();
      if(last_cfreq < min) last_cfreq = min;
      if(last_cfreq > max) last_cfreq = max;
      spectrum_center_freq = last_cfreq * 1e3;
      UpdateAxes();
      w->SetValue(last_cfreq);
    }
  }

  void SoDaRadio_Top::OnPerRefLevel( wxSpinEvent & event)
  {
    float yref = spect_config->getRefLevel();
    if(yref != spectrum_y_reflevel) {
      spectrum_y_reflevel = yref;
      UpdateAxes();
    }
  }

  void SoDaRadio_Top::OnPerBandSpread(wxCommandEvent & event )
  {
    float spread = spect_config->getBandSpread();
    if(spread != spectrum_bandspread) {
      spectrum_bandspread = spread;
      UpdateAxes(); 
    }
  }

  void SoDaRadio_Top::OnPerYScaleChoice( wxCommandEvent & event)
  {
    float yscale = spect_config->getdBScale();
    if(yscale != spectrum_y_scale) {
      spectrum_y_scale = yscale;
      UpdateAxes(); 
    }
  }

  void SoDaRadio_Top::OnPerRxToCentFreq( wxCommandEvent & event)
  {
    double rxfreq = GetRXFreq();

    UpdateCenterFreq(rxfreq); 
  }

  void SoDaRadio_Top::UpdateCenterFreq(double cfreq)
  {
    // round the rxfreq to the nearest 25KHz.
    // and turn it into kHz
    unsigned long spin_center = 25 * round(cfreq / 25e3);
  
    // and set the spin box.
    unsigned long spin_low, spin_high, spin_step;
    unsigned long spread = (unsigned long) (GetSpread() / 1.0e3);

    spin_low = spin_center - spread * 10;
    spin_high = spin_center + spread * 10;

    cfreq_step = 25; 

    spect_config->setFreqSpinner(spin_low, spin_high, spin_center);
    
    last_cfreq = spin_center;
    spectrum_center_freq = last_cfreq * 1e3;

    // now update.
    UpdateAxes();   
  }

  void SoDaRadio_Top::SetPerVals(double cfreq, double reflevel, float yscale, float bspread)
  {

    spect_config->setFreqSpinner(cfreq - (bspread * 0.5e-3),
				 cfreq + (bspread * 0.5e-3),
				 cfreq);

    spect_config->setRefLevel(reflevel); 

    spect_config->setBandSpread(bspread * 1.0e-3); 

    spect_config->setdBScale(yscale); 
  }

  void SoDaRadio_Top::OnTunePopup( wxCommandEvent & event)
  {
    // popup a modal tuning dialog.
    tuner->Show();
  }

  
  void SoDaRadio_Top::OnCtrlPopup( wxCommandEvent & event)
  {
    // popup a modal tuning dialog.
    controls->Show();
  }

  void BandConfigDialog::setChoiceBox(wxChoice * box, std::string & sel)
  {
    // roll through choices looking for one that matches sel.
    int i;
    wxString wsel = wxString(sel.c_str(), wxConvUTF8);
    
    for(i = 0; i < box->GetCount(); i++) {
      if(wsel == box->GetString(i)) {
	box->SetSelection(i);
	return; 
      }
    }
  }
  
  void BandConfigDialog::initBandList(SoDaRadio_BandSet * bandset)
  {
    std::string nm;
    m_BandChoiceBox->Clear();
    for(nm = bandset->getFirstName(); nm != ""; nm = bandset->getNextName()) {
      m_BandChoiceBox->Append(wxString(nm.c_str(), wxConvUTF8));
    }
    m_BandChoiceBox->Append(wxString(wxT("Create New Band"))); 
    m_BandChoiceBox->SetStringSelection(wxString(wxT("Create New Band")));

    bands = bandset;

    radio_top->setupBandSelect(bandset); 
  }

  void BandConfigDialog::OnConfigChoice(wxCommandEvent & event)
  {
    // what band did we choose?
    // if we already know the band, fill in the blanks.
    std::string bcb = std::string(m_BandChoiceBox->GetStringSelection().mb_str());
    if(bcb != "Create New Band") {
      // find the band choice.
      SoDaRadio_Band * choice = bands->getByName(bcb);

      if(choice != NULL) {
	// set the band name
	m_BandName->SetValue(wxString(bcb.c_str(), wxConvUTF8));
	// set the low and high edges
	m_low_edge->SetValue(wxString((boost::format("%f") % choice->lower_band_edge).str().c_str(), wxConvUTF8));
	m_high_edge->SetValue(wxString((boost::format("%f") % choice->upper_band_edge).str().c_str(), wxConvUTF8));
	// set the RX ant choice
	setChoiceBox(m_RXAntChoice, choice->rx_antenna_choice);
	// set the modulation choice
	setChoiceBox(m_ModChoice, choice->default_mode);
	// can we TX on this band?
	m_TXEna->SetValue(choice->enable_transmit);
	// set the band ID
	m_BandID->SetValue(choice->band_id);
	// is this a transverter band?
	m_TransverterMode->SetValue(choice->transverter_mode);
	OnTransverterModeSel(event); 
	if (choice->transverter_mode) {
	  // set the injection side
	  m_InjectionSel->SetSelection(choice->low_side_injection ? 0 : 1);
	  // set the lo freq
	  m_TransFreqEntry->SetValue(wxString((boost::format("%f") % choice->transverter_lo_freq).str().c_str(), wxConvUTF8));	  
	  // set the multiplier
	  m_TransMultEntry->SetValue(wxString((boost::format("%f") % choice->transverter_multiplier).str().c_str(), wxConvUTF8));
	  // set the LO generation mode
	  m_LOGenMode->SetValue(choice->transverter_local_lo);
	}
      }
    }
  }
  
  void BandConfigDialog::OnProblem(std::string const & probstring ) {
    BandConfigProblem * bcp = new BandConfigProblem(this, probstring.c_str()); 
    bcp->Show(); 
  }

  void BandConfigProblem::OnBandErrorOK( wxCommandEvent & event)
  {
    if(IsModal()) EndModal(wxID_OK);
    else {
      SetReturnCode(wxID_OK);
      this->Show(false); 
    }
  }
  
  void BandConfigDialog::OnBandOK( wxCommandEvent & event)
  {
    std::string bcb = std::string(m_BandChoiceBox->GetStringSelection().mb_str());

    bool found_problem = false;
    std::string problem_string = "";
    
    std::string bandname; 
    if(bcb == "Create New Band") {
      bandname = std::string(m_BandName->GetValue().mb_str());
      if(bandname == "") {
	// need to pop something up.
	problem_string = "No Band Name supplied.";
	found_problem = true; 
      }
    }
    else {
      bandname = bcb; 
    }

    // now find the band entry.
    SoDaRadio_Band * newband = bands->getByName(bandname);
    if(newband == NULL) {
      newband = new SoDaRadio_Band();
    }

    double le, ue;
    m_low_edge->GetValue().ToDouble(&le);
    if(le == 0.0) {
      found_problem = true;
      problem_string = problem_string + "\nPlease specify lower band edge (non zero).";
    }
    m_high_edge->GetValue().ToDouble(&ue);
    if(ue == 0.0) {
      found_problem = true;
      problem_string = problem_string + "\nPlease specify upper band edge (non zero).";
    }

    bool lsi; ///< low side injection.
    double tr_lo; ///< transverter local osc freq.
    double tr_mult; ///< transverter multiplier
    bool local_lo;
    // now check the transverter stuff.
    if (m_TransverterMode->IsChecked()) {
      m_TransFreqEntry->GetValue().ToDouble(&tr_lo);
      m_TransMultEntry->GetValue().ToDouble(&tr_mult);
      local_lo = m_LOGenMode->IsChecked();

      if(tr_lo <= 0.0) {
	found_problem = true;
	problem_string = problem_string + "\nPlease specify transverter local osc. frequency (non zero).";
      }
      if(tr_mult <= 0.0) {
	found_problem = true;
	problem_string = problem_string + "\nPlease specify transverter local osc. multiplier (non zero).";
      }

      lsi = (m_InjectionSel->GetSelection() == wxString(wxT("Low Side")));
    }
    
    std::string mode = std::string(m_ModChoice->GetStringSelection().mb_str());
    std::string ant = std::string(m_RXAntChoice->GetStringSelection().mb_str());
    bool ena = m_TXEna->IsChecked();
    unsigned char bid = (unsigned char) (m_BandID->GetValue() & 0xff);

    if(!found_problem) {
      newband->setupBand(bandname, le, ue, mode, ant, bid, ena);
      if(m_TransverterMode->IsChecked()) {
	newband->setupTransverter(tr_lo, tr_mult, lsi, local_lo); 
      }
      
      bands->add(newband); 

      m_BandName->Clear();
    
      if(IsModal()) {
	EndModal(wxID_OK); 
      }
      else {
	SetReturnCode(wxID_OK);
	this->Show(false); 
      }
      radio_top->setupBandSelect(bands);
    }
    else {
      // there was a problem of some sort.  popup the dialog and return.
      OnProblem(problem_string); 
    }
  }

  void BandConfigDialog::OnBandCancel( wxCommandEvent & event)
  {
    if(IsModal()) {
      EndModal(wxID_OK); 
    }
    else {
      SetReturnCode(wxID_OK);
      this->Show(false); 
    }
  }

  void BandConfigDialog::OnTransverterModeSel( wxCommandEvent & event)
  {
    // enable the other fields.
    bool ena = m_TransverterMode->IsChecked();
    m_InjectionSel->Enable(ena);

    m_LOGenMode->Enable(ena);
    
    m_TransFreqLabel->Enable(ena); 
    m_TransFreqLabel2->Enable(ena); 
    m_TransFreqEntry->Enable(ena); 

    m_TransMultLabel->Enable(ena); 
    m_TransMultEntry->Enable(ena); 
  }
    
  void BandConfigDialog::OnBandActivate( wxCommandEvent & event)
  {
  }

  void SoDaRadio_Top::OnUpdateSpectrumPlot(wxCommandEvent & event)
  {
    if(SpectrumDisplay->GetSelection() == 0) {
      // update the waterfall
      if((wfall_plot != NULL) && (wfall_plot->IsShownOnScreen())) {
	wfall_plot->DrawNew();
      }
    }
    else if((pgram_trace != NULL) && (pgram_plot->IsShownOnScreen())) {
      // otherwise, update the periodogram
      pgram_plot->Draw();
    }
  }

  void SoDaRadio_Top::OnUpdateGPSLoc(wxCommandEvent & event)
  {
    m_GPSGrid->SetLabel(GPS_Grid_Str); 
    m_GPSLat->SetLabel(GPS_Lat_Str); 
    m_GPSLon->SetLabel(GPS_Lon_Str); 
  }

  void SoDaRadio_Top::OnUpdateGPSTime(wxCommandEvent & event)
  {
    m_UTC->SetLabel(GPS_UTC_Str); 
  }


  void SoDaRadio_Top::OnClrCWBuffer( wxCommandEvent& event) {
    //  tell the radio to flush all outstanding CW text
    SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::TX_CW_FLUSHTEXT, 
		       0);
    sendMsg(&ncmd); 
  }

  
}
