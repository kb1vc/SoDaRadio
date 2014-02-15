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
#include <wx/string.h>
#include <wx/wx.h>
#include <wx/file.h>
#include <wx/colour.h>
#include "../src/Command.hxx"
#include <iostream>
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/format.hpp>

using namespace std; 
namespace SoDaRadio_GUI {
  
  void SoDaRadio_Top::SaveSoDaConfig(const wxString & fname)
  {
    // std::cerr << "In save soda config with config_tree == " << config_tree << std::endl;
    if(config_tree == NULL) {
      config_tree = new boost::property_tree::ptree();
      //std::cerr << "got new config tree" << std::endl; 
      config_tree->put("af.gain", 3.0);
      //std::cerr << "completed get tree put" << std::endl; 
    }

    // query all the relevant widgets and record their values
    config_tree->put("rx.mode", m_ModeBox->GetStringSelection().mb_str(wxConvUTF8));
    config_tree->put("rx.freq", rx_frequency);
    config_tree->put("rx.prev_freq", last_rx_frequency);
  
    config_tree->put("tx.freq", tx_frequency);
    config_tree->put("tx.prev_freq", last_tx_frequency);
    config_tree->put("tx.rf_outpower", tx_rf_outpower);

    config_tree->put("transverter.base.nominal", nominal_lo_base_freq);
    config_tree->put("transverter.base.actual", actual_lo_base_freq);
    config_tree->put("transverter.lo.mult", lo_multiplier);
  
    config_tree->put("disp.band_spread_choice", spectrum_bandspread);

    config_tree->put("station.call", from_callsign.mb_str(wxConvUTF8));
    config_tree->put("station.qth", from_grid.mb_str(wxConvUTF8));

    config_tree->put("af.gain", m_AFGain->GetValue());
    config_tree->put("af.bw", m_AFBWChoice->GetSelection());  

    config_tree->put("cw.speed", controls->getCWSpeed());
    config_tree->put("cw.sidetone", controls->getSTGain());

    config_tree->put("spectrum.center_freq", spectrum_center_freq);
    config_tree->put("spectrum.yscale", spectrum_y_scale);
    config_tree->put("spectrum.bandspread", spectrum_bandspread);
    config_tree->put("spectrum.reflevel", spectrum_y_reflevel);
  
    config_tree->put("reference.source", tuner->getExtRefEna());
    config_tree->put("tx.tx_rx_locked", tx_rx_locked);
    boost::property_tree::xml_writer_settings<char> wset('\t',1);
    // std::cerr << "Got char tab set." << std::endl; 
    write_xml((const char*) fname.mb_str(wxConvUTF8), *config_tree, std::locale(), wset);


  }

  bool SoDaRadio_Top::LoadSoDaConfig(const wxString & fname)
  {
    // std::cerr << "Loading SoDa configuration" << std::endl; 
    if(config_tree != NULL) delete config_tree;

    // does the file exist?
    if(!wxFile::Exists(fname.c_str())) {
      return false; 
    }
  
    config_tree = new boost::property_tree::ptree();
    read_xml((const char *) fname.mb_str(wxConvUTF8), *config_tree, boost::property_tree::xml_parser::trim_whitespace);


    // now call all the event updates.
    wxCommandEvent nullCE;
    wxScrollEvent nullSE;
    wxSpinEvent nullSPE;

    // now go through each widget and read its value, and update it.
    m_ModeBox->SetStringSelection(wxString::FromUTF8(config_tree->get<std::string>("rx.mode").c_str()));
    OnModeChoice(nullCE);
  

    nominal_lo_base_freq = config_tree->get<double>("transverter.base.nominal");
    actual_lo_base_freq = config_tree->get<double>("transverter.base.actual");
    lo_multiplier = config_tree->get<double>("transverter.lo.mult");
  
    tx_transverter_offset = actual_lo_base_freq * lo_multiplier; 
    rx_transverter_offset = actual_lo_base_freq * lo_multiplier;
  
    rx_frequency = config_tree->get<double>("rx.freq");
    last_rx_frequency = config_tree->get<double>("rx.prev_freq");
    UpdateRXFreq(rx_frequency);
    // tuner->newRXFreq();

    tx_frequency = config_tree->get<double>("tx.freq");
    last_tx_frequency = config_tree->get<double>("tx.prev_freq");
    // tuner->newTXFreq();
    UpdateTXFreq(tx_frequency);
  
    // frequency updates
    SoDa::Command rncmd(SoDa::Command::SET, SoDa::Command::RX_RETUNE_FREQ,
			applyRXTVOffset(rx_frequency));
    sendMsg(&rncmd);
    SoDa::Command tncmd(SoDa::Command::SET, SoDa::Command::TX_RETUNE_FREQ,
			applyTXTVOffset(tx_frequency));
    sendMsg(&tncmd);


    // query all the relevant widgets and record their values
    tx_rf_outpower = config_tree->get<float>("tx.rf_outpower");
    controls->setTXPower(tx_rf_outpower);


    // update the widget
    spectrum_bandspread = config_tree->get<float>("disp.band_spread_choice");
    OnPerBandSpread(nullCE);

    from_callsign = wxString::FromUTF8(config_tree->get<std::string>("station.call").c_str());
    from_grid = wxString::FromUTF8(config_tree->get<std::string>("station.qth").c_str());
    to_grid = wxT("FN42bl");
    m_MyGrid->SetLabel(from_grid);
    if(to_grid.length() != 0) {
      UpdateNavigation(); 
    }

    // these are the gain controls.
    m_AFGain->SetValue(config_tree->get<float>("af.gain"));
    OnAFBWChoice(nullCE);
  
    m_AFBWChoice->SetSelection(config_tree->get<int>("af.bw"));
    OnAFGainScroll(nullSE);
  

    controls->setCWSpeed(config_tree->get<float>("cw.speed"));
  
    controls->setSTGain(config_tree->get<float>("cw.sidetone"));

    spectrum_bandspread = config_tree->get<float>("spectrum.bandspread");
    spectrum_center_freq = config_tree->get<double>("spectrum.center_freq");
    last_cfreq = spectrum_center_freq; 
    m_cFreqSpin->SetValue(spectrum_center_freq);
    spectrum_y_scale = config_tree->get<float>("spectrum.yscale");
    SetYScale(spectrum_y_scale); 
    spectrum_y_reflevel = config_tree->get<float>("spectrum.reflevel");
    m_RefLevel->SetValue(spectrum_y_reflevel); 
    SetSpectrum(spectrum_bandspread);
    UpdateAxes();
    // tell the spectrum widget where to plot...
    // OnPerRxToCentFreq(nullCE);
    // UpdateAxes();

    tuner->setExtRefEna(config_tree->get<bool>("reference.source"));

    try {
      tx_rx_locked = config_tree->get<bool>("tx.tx_rx_locked");
    } catch (boost::exception const & ex) {
      tx_rx_locked = true;
    }
    OnTXRXLock(nullCE); 
    return true; 
  }
}
