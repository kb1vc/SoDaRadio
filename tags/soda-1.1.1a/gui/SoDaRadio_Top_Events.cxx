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
#include <wx/string.h>
#include <wx/wx.h>
#include <wx/textdlg.h>
#include <wx/colour.h>
#include "../src/Command.hxx"
#include "Navigation.hxx"
#include <math.h>

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

  void SoDaRadio_Top::OnOpenConfig( wxCommandEvent& event )
  {
    wxString defaultDir = wxT("~/.SoDa");
    wxString defaultFilename = wxT("SoDa.soda_cfg");
    wxString wildcard = wxT("SoDa Config files (*.soda_cfg)|*.soda_cfg");
    wxFileDialog dialog(this, wxT("Load Configuration File"), defaultDir, defaultFilename, wildcard, wxOPEN);
  

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
    wxString defaultDir = wxT("~/.SoDa");
    wxString defaultFilename = wxT("SoDa.soda_cfg");
    wxString wildcard = wxT("SoDa Config files (*.soda_cfg)|*.soda_cfg");
    wxFileDialog dialog(this, wxT("Save to Selected Configuration File"), defaultDir, defaultFilename, wildcard, wxSAVE);

    if (dialog.ShowModal() == wxID_OK) {
      wxString fname = dialog.GetPath();
      save_config_file_name = fname; 
      SaveSoDaConfig(fname);
    }
  }

  void SoDaRadio_Top::OnOpenLogfile( wxCommandEvent& event )
  {
    wxString defaultDir = wxT("~/.SoDa");
    wxString defaultFilename = wxT("SoDa.soda_log");
    wxString wildcard = wxT("SoDa Log files (*.soda_log)|*.soda_log");
    wxFileDialog dialog(this, wxT("Open/Create Log File"), defaultDir, defaultFilename, wildcard, wxSAVE);

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



  void SoDaRadio_Top::OnQuit( wxCommandEvent& event )
  {
    // ask the logger to write the final log
    logdialog->closeLog();

    // This will cause the radio server to abort as well. 
    SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::STOP, 0);
    sendMsg(&ncmd);
    Close(); 
  }

  void SoDaRadio_Top::OnSelectPage( wxNotebookEvent& event )
  {
    // TODO: Implement OnSelectPage
  }

  void SoDaRadio_Top::OnWFallFreqSel( wxMouseEvent& event )
  {
    // TODO: Implement OnWFallFreqSel
  }

  void SoDaRadio_Top::OnPeriodogramFreqSel( wxMouseEvent& event )
  {
    // TODO: Implement OnPeriodogramFreqSel
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
    SoDa::Command  * cmdptr = new SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_CW_MARKER, marker_id);
    sendMsg(cmdptr); 
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
	SoDa::Command  * cmdptr = new SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_CW_TEXT, &(cwbuf[i]));
	sendMsg(cmdptr); 
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

    SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::RX_MODE, (int) mt);
    SoDa::Command ncmd2(SoDa::Command::SET, SoDa::Command::TX_MODE, (int) mt);
    sendMsg(&ncmd);
    sendMsg(&ncmd2);
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
    double gain = ((double) igain) / 10.0;
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
    std::cerr << "Hooooooo!   We changed a cell" << std::endl; 
  }



  void SoDaRadio_Top::OnSetTransverterOffset( wxCommandEvent& event)
  {
    // set the new stuff.
    transconf->UpdateSettings(); 
    transconf->Show(); 
  }

  void TuningDialog::OnTransvLOCal( wxCommandEvent & event)
  {
    SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::LO_CHECK,
		       radio_top->nominal_lo_base_freq);
    radio_top->sendMsg(&ncmd);
  }

  void SoDaRadio_Top::setLOOffset(double v)
  {
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


  void SoDaRadio_Top::SetRXFreqFromDisp(double freq)
  {
    last_rx_frequency = rx_frequency;
    // adjust freq based on mode
    if(cw_mode) {
      // if we're upper sideband, depress the freq.... 
      freq = freq + (cw_upper ? -500.0 : 500.0); 
    }
  
    UpdateRXFreq(freq);
    tuner->newRXFreq();
    if(tx_rx_locked) {
      UpdateTXFreq(freq);
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
    pgram_plot->SetScale(xmin, xmax, ymin, ymax);
    wfall_plot->SetScale(xmin, xmax);

    // now tell the radio
    SoDa::Command ncmd(SoDa::Command::SET, SoDa::Command::SPEC_CENTER_FREQ,
		       applyRXTVOffset(spectrum_center_freq));
    sendMsg(&ncmd); 
  }

  void SoDaRadio_Top::SetSpectrum(double bandspread)
  {
    double spval[] = {25, 50, 100, 200, -1}; //spread in kHz
    int selidx = 2;

    int i;
    for(i = 0; spval[i] > 0; i++) {
      if(spval[i] == bandspread) selidx = i; 
    }
  
    m_BandSpreadChoice->SetSelection(selidx);
    spectrum_bandspread = spval[selidx];
    UpdateAxes();
  }

  double SoDaRadio_Top::GetSpread()
  {
    double spval[] = {25, 50, 100, 200, 500}; //spread in kHz
    int selidx = m_BandSpreadChoice->GetSelection();

    if(selidx > 4) selidx = 4;

    return 1.0e3 * spval[selidx]; 
  }

  void SoDaRadio_Top::SetYScale(double v)
  {
    double spval[] = {1, 5, 10, 20, 0}; // range in dB / box
  
    int selidx = 3;

    int i;
    for(i = 0; spval[i] > 0; i++) {
      if(spval[i] == v) selidx = i; 
    }

    m_dBScale->SetSelection(selidx);
  }

  double SoDaRadio_Top::GetYScale()
  {
    double spval[] = {1, 5, 10, 20}; // range in dB / box
    int selidx = m_dBScale->GetSelection();

    if(selidx > 3) selidx = 3;

    return spval[selidx]; 
  }

  void SoDaRadio_Top::OnPerCFreqStep( wxSpinEvent & event)
  {
    // did it go up or down?
    unsigned long new_cfreq = m_cFreqSpin->GetValue();
    //  std::cerr << "In OnCFreqStep with new = " << new_cfreq << " and old = " << last_cfreq << std::endl; 
    if(new_cfreq != last_cfreq) {
      // take the difference and make new steps.
      unsigned long diff = new_cfreq - last_cfreq;
      last_cfreq += diff * cfreq_step;
      unsigned long min = m_cFreqSpin->GetMin(); 
      unsigned long max = m_cFreqSpin->GetMax();
      if(last_cfreq < min) last_cfreq = min;
      if(last_cfreq > max) last_cfreq = max;
      spectrum_center_freq = last_cfreq * 1e3;
      //    std::cerr << "New center frequency = " << spectrum_center_freq << std::endl;
      UpdateAxes();
      m_cFreqSpin->SetValue(last_cfreq);
    }
  }

  void SoDaRadio_Top::OnPerRefLevel( wxSpinEvent & event)
  {
    float yref = m_RefLevel->GetValue();
    if(yref != spectrum_y_reflevel) {
      spectrum_y_reflevel = yref;
      UpdateAxes();
    }
  }

  void SoDaRadio_Top::OnPerBandSpread(wxCommandEvent & event )
  {
    float spread = GetSpread();
    if(spread != spectrum_bandspread) {
      spectrum_bandspread = spread;
      UpdateAxes(); 
    }
  }

  void SoDaRadio_Top::OnPerYScaleChoice( wxCommandEvent & event)
  {
    float yscale = GetYScale();
    if(yscale != spectrum_y_scale) {
      spectrum_y_scale = yscale;
      UpdateAxes(); 
    }
  }

  // void SoDaRadio_Top::OnPerCFreqSpin()
  // {
  //   double new_cfreq = m_cFreqSpin->GetValue() * 1e3;
  //   if(new_cfreq != spectrum_center_freq) {
  //     spectrum_center_freq = new_cfreq;
  //     UpdateAxes();
  //   }
  // }

  void SoDaRadio_Top::OnPerRxToCentFreq( wxCommandEvent & event)
  {
    double rxfreq = GetRXFreq();

    // round the rxfreq to the nearest 25KHz.
    // and turn it into kHz
    unsigned long spin_center = 25 * round(rxfreq / 25e3);

    // std::cerr << boost::format("rxfreq = %12.10g spin_center = %ld")
    //   % rxfreq % spin_center << std::endl; 
  
    // and set the spin box.
    unsigned long spin_low, spin_high, spin_step;
    unsigned long spread = (unsigned long) (GetSpread() / 1.0e3);

    spin_low = spin_center - spread * 10;
    spin_high = spin_center + spread * 10;

    // std::cerr << boost::format("rxfreq = %12.10g spin_center = %ld spin_low = %ld spin_high = %ld spread = %ld")
    //   % rxfreq % spin_center % spin_low % spin_high % spread << std::endl; 

    cfreq_step = 25; 

    m_cFreqSpin->SetRange(spin_low, spin_high);
    m_cFreqSpin->SetValue(spin_center);
    last_cfreq = spin_center;
    spectrum_center_freq = last_cfreq * 1e3;

    // now update.
    UpdateAxes();   
  }

#if 0
  void ConfigSpectrumDlg::OnRxToCentFreq( wxCommandEvent & event)
  {
    SoDaRadio_Top * parent = (SoDaRadio_Top *) this->GetParent();
    double rxfreq = parent->GetRXFreq();

    // round the rxfreq to the nearest 25KHz.
    // and turn it into kHz
    unsigned long spin_center = 25 * round(rxfreq / 25e3);
  
    // and set the spin box.
    unsigned long spin_low, spin_high, spin_step;
    unsigned long spread = (unsigned long) (GetSpread() / 1.0e3);

    spin_low = spin_center - spread * 10;
    spin_high = spin_center + spread * 10;

    cfreq_step = 25; 

    m_cFreqSpin->SetRange(spin_low, spin_high);
    m_cFreqSpin->SetValue(spin_center);
    last_cfreq = spin_center;

    UpdateAxes();
  
  }

  void ConfigSpectrumDlg::OnSet( wxCommandEvent & event)
  {
    SoDaRadio_Top * parent = (SoDaRadio_Top *) this->GetParent();
    bool update_xyplot = false; 

    // what's the new center freq? 
    double new_cfreq = m_cFreqSpin->GetValue() * 1e3;
    if(new_cfreq != parent->spectrum_center_freq) {
      parent->spectrum_center_freq = new_cfreq;
      update_xyplot = true; 
    }

    // the bandspread
    float spread = GetSpread();
    if(spread != parent->spectrum_bandspread) {
      parent->spectrum_bandspread = spread; 
      update_xyplot = true; 
    }
    
    // the y reference level
    float yref = m_RefLevel->GetValue();
    if(yref != parent->spectrum_y_reflevel) {
      parent->spectrum_y_reflevel = yref;
      update_xyplot = true; 
    }

    // the y scale factor
    float yscale = GetYScale();
    if(yscale != parent->spectrum_y_scale) {
      parent->spectrum_y_scale = yscale;
      update_xyplot = true; 
    }

    if(update_xyplot) {
      parent->UpdateAxes(); 
    }
  }

  void ConfigSpectrumDlg::OnOK( wxCommandEvent & event)
  {
    // this is an implicit "set"
    OnSet(event);
  
    if(IsModal() ) {
      EndModal(wxID_OK);
    }
    else {
      SetReturnCode(wxID_OK);
      this->Show(false); 
    }
  }

  void ConfigSpectrumDlg::OnCancel( wxCommandEvent & event)
  {
    if(IsModal() ) {
      EndModal(wxID_CANCEL);
    }
    else {
      SetReturnCode(wxID_CANCEL);
      this->Show(false); 
    }
  }
#endif

  void SoDaRadio_Top::SetPerVals(double cfreq, double reflevel, float yscale, float bspread)
  {
  
    m_cFreqSpin->SetRange(cfreq - (bspread * 0.5e-3), cfreq + (bspread * 0.5e-3));
    m_cFreqSpin->SetValue(cfreq);

    m_RefLevel->SetValue(reflevel);
    if(bspread < 30e3) {
      m_BandSpreadChoice->SetSelection(0);
    }
    else if(bspread < 70e3) {
      m_BandSpreadChoice->SetSelection(1);
    }
    else if(bspread < 130e3) {
      m_BandSpreadChoice->SetSelection(2);
    }
    else if(bspread < 230e3) {
      m_BandSpreadChoice->SetSelection(3);
    }
    else {
      m_BandSpreadChoice->SetSelection(4);
    }

    if(yscale < 1) {
      m_dBScale->SetSelection(0);
    }
    else if(yscale < 7) {
      m_dBScale->SetSelection(1);
    }
    else if(yscale < 12) {
      m_dBScale->SetSelection(2);
    }
    else {
      m_dBScale->SetSelection(3);
    }
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

  void TransverterConfigDialog::OnTVConfDone( wxCommandEvent & event)
  {
    if(IsModal()) {
      EndModal(wxID_OK); 
    }
    else {
      ReadSettings(); 
      SetReturnCode(wxID_OK);
      this->Show(false); 
    }
  }

  void TransverterConfigDialog::OnTVConfCancel( wxCommandEvent & event)
  {
    if(IsModal()) {
      EndModal(wxID_OK); 
    }
    else {
      SetReturnCode(wxID_OK);
      this->Show(false); 
    }
  }

  void TransverterConfigDialog::OnTVActivate( wxCommandEvent & event)
  {
    std::cerr << "Hey!  we're here in TransverterConfigDialog activate!" << std::endl; 
  }

#if 0
  void SoDaRadio_Top::OnConfigSpectrum( wxCommandEvent & event)
  {

    ConfigSpectrumDlg dialog(this);
    // get the current values and set them.
    dialog.SetVals(spectrum_center_freq * 1e-3, spectrum_y_reflevel, spectrum_y_scale, spectrum_bandspread); 
    if(dialog.ShowModal() == wxID_OK) {
      // std::cerr << "config spectrum returned ok..." << std::endl;
      // std::cerr << "center freq choice is " << spectrum_center_freq << std::endl; 
    }
    else {
      std::cerr << "config spectrum returned not ok..." << std::endl; 
    }
  }
#endif

  void SoDaRadio_Top::OnUpdateSpectrumPlot(wxCommandEvent & event)
  {
    // std::cerr << "Got update spectrum plot message" << std::endl;
    if(pgram_trace != NULL) {
      pgram_plot->Draw();
      if(wfall_plot != NULL) wfall_plot->DrawNew();
    }
    int i;
    int len = pgram_trace->GetLength();
    double * x;
    float * y;
    x = pgram_trace->GetXVec();
    y = pgram_trace->GetYVec();
    double xmin, xmax, ymin, ymax;
    pgram_plot->GetScale(xmin, xmax, ymin, ymax);
    float maxval = -1000.0;
    float maxfreq = 0.0; 
    int maxidx = 0; 
    // for(i = 0; i < len; i++) {
    //   if(y[i] > maxval) {
    //     maxidx = i;
    //     maxval = y[i];
    //     maxfreq = x[i];
    //   }
    // }
    // std::cerr << "max idx = " << maxidx << std::setprecision(10)
    // 	    << " maxval =  " << maxval << " maxfreq =  " << maxfreq << " "
    // 	      << xmin << " " << xmax << " " << ymin << " " << ymax
    // 	      << std::endl; 

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
