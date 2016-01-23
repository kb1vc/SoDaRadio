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
#include <boost/version.hpp>

using namespace std; 
namespace SoDaRadio_GUI {
  
  void SoDaRadio_Top::SaveSoDaConfig(const wxString & fname)
  {

    config_tree = config_tree_alloc = new boost::property_tree::ptree();
    debugMsg(boost::format("Allocated config tree = %p\n") % config_tree);

    config_tree->put("SoDaRadio.af.gain", 3.0);
    //std::cerr << "completed get tree put" << std::endl; 

    // query all the relevant widgets and record their values
    config_tree->put("SoDaRadio.rx.mode", m_ModeBox->GetStringSelection().mb_str(wxConvUTF8));
    config_tree->put("SoDaRadio.rx.freq", rx_frequency);
    config_tree->put("SoDaRadio.rx.prev_freq", last_rx_frequency);
  
    config_tree->put("SoDaRadio.tx.freq", tx_frequency);
    config_tree->put("SoDaRadio.tx.prev_freq", last_tx_frequency);
    config_tree->put("SoDaRadio.tx.rf_outpower", tx_rf_outpower);
    debugMsg(boost::format("config tree = %p A\n") % config_tree);  
    config_tree->put("SoDaRadio.station.call", from_callsign.mb_str(wxConvUTF8));
    config_tree->put("SoDaRadio.station.qth", from_grid.mb_str(wxConvUTF8));

    config_tree->put("SoDaRadio.af.gain", m_AFGain->GetValue());
    config_tree->put("SoDaRadio.af.bw", m_AFBWChoice->GetSelection());  
    debugMsg(boost::format("config tree = %p B\n") % config_tree);
    config_tree->put("SoDaRadio.cw.speed", controls->getCWSpeed());
    config_tree->put("SoDaRadio.cw.sidetone", controls->getSTGain());

    config_tree->put("SoDaRadio.spectrum.center_freq", spectrum_center_freq);
    config_tree->put("SoDaRadio.spectrum.yscale", spectrum_y_scale);
    config_tree->put("SoDaRadio.spectrum.bandspread", spectrum_bandspread);
    config_tree->put("SoDaRadio.spectrum.reflevel", spectrum_y_reflevel);
    debugMsg(boost::format("config tree = %p C\n") % config_tree);
    if(SpectrumDisplay->GetSelection() == 0) {
      config_tree->put("SoDaRadio.spectrum.display", "waterfall");
    }
    else {
      config_tree->put("SoDaRadio.spectrum.display", "periodogram");
    }
    debugMsg(boost::format("config tree = %p D\n") % config_tree);
    config_tree->put("SoDaRadio.reference.source", tuner->getExtRefEna());
    config_tree->put("SoDaRadio.tx.tx_rx_locked", tx_rx_locked);
#if (BOOST_VERSION >= 105600)
    boost::property_tree::xml_writer_settings<std::string> wset('\t',1);
#else
    boost::property_tree::xml_writer_settings<char> wset('\t',1);
#endif
    
    // save the band configurations to the tree.
    config_tree->put("SoDaRadio.current_band", current_band->getName());
    
    SaveCurrentBand();
    bandset->save(config_tree);
    
    debugMsg(boost::format("Saved config tree = %p before writexml\n") % config_tree);
    write_xml((const char*) fname.mb_str(wxConvUTF8), *config_tree, std::locale(), wset);

    debugMsg(boost::format("Saved config tree = %p after writexml\n") % config_tree);
  }

  void SoDaRadio_Top::CreateDefaultConfig(boost::property_tree::ptree * config_tree)
  {
    std::stringstream config_stream;

    // write the default config file into the config stream. -- tell doxygen to ignore it
    /** 
     * @cond
     */
#include "Default.soda_cfg.h"
    /** 
     * @endcond
     */

    read_xml(config_stream, *config_tree, boost::property_tree::xml_parser::trim_whitespace);    
  }
  

  void SoDaRadio_Top::SetConfigFileName(const wxString & fname) {
    save_config_file_name = fname; 
  }
  
  bool SoDaRadio_Top::LoadSoDaConfig(const wxString & fname)
  {
    debugMsg(boost::format("About to delete config tree = %p\n") % config_tree);
    if(config_tree_alloc != NULL) delete config_tree_alloc;

    debugMsg("About to create config tree\n");
	
    config_tree_alloc = new boost::property_tree::ptree();
    debugMsg(boost::format("Got new config tree = %p\n") % config_tree_alloc);
    
    // does the file exist?
    if(!wxFile::Exists(fname.c_str())) {
      // then we need to load the default config.
      debugMsg(boost::format("Creating default config tree = %p\n") % config_tree_alloc);
      CreateDefaultConfig(config_tree_alloc);
      // also pop up the save config dialog box.
      NewConfigDialog * ncd = new NewConfigDialog(this, this);
      ncd->Show();
    }
    else {
      debugMsg(boost::format("Reading config file config tree = %p from %s\n") % config_tree_alloc % fname.c_str());
      std::ifstream config_stream((const char *) fname.mb_str(wxConvUTF8));
      read_xml(config_stream, *config_tree_alloc,
	       boost::property_tree::xml_parser::trim_whitespace);
      config_stream.close();
    }

    debugMsg(boost::format("filled in config tree = %p\n") % config_tree_alloc);
    // The original config format wasn't really proper XML... 
    if(config_tree_alloc->get_child_optional("SoDaRadio")) {
      debugMsg(boost::format("new format config tree = %p\n") % config_tree_alloc);
      config_tree = & config_tree_alloc->get_child("SoDaRadio"); 
    } else {
      debugMsg(boost::format("old format config tree = %p\n") % config_tree_alloc);
      config_tree = config_tree_alloc; 
    }
    
    debugMsg(boost::format("Got the config tree %p\n") % config_tree);
    // now call all the event updates.
    wxCommandEvent nullCE;
    wxScrollEvent nullSE;
    wxSpinEvent nullSPE;

    debugMsg(boost::format("Loading band information config tree %p\n") % config_tree);
    // load the band information
    bandset = new SoDaRadio_BandSet(config_tree);
    setupBandSelect(bandset); 
    
    debugMsg("Loaded band information\n");
    
    // now go through each widget and read its value, and update it.
    m_ModeBox->SetStringSelection(wxString::FromUTF8(config_tree->get<std::string>("rx.mode").c_str()));
    OnModeChoice(nullCE);
  
    debugMsg("Updating frequencies\n");
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

    debugMsg("Setting transmit power\n");
    // query all the relevant widgets and record their values
    tx_rf_outpower = config_tree->get<float>("tx.rf_outpower");
    controls->setTXPower(tx_rf_outpower);

    
    debugMsg("Setting QSO fields\n");
    from_callsign = wxString::FromUTF8(config_tree->get<std::string>("station.call").c_str());
    from_grid = wxString::FromUTF8(config_tree->get<std::string>("station.qth").c_str());
    to_grid = wxT("FN42bl");
    m_MyGrid->SetLabel(from_grid);
    if(to_grid.length() != 0) {
      UpdateNavigation(); 
    }

    debugMsg("Setting Gain controls\n");
    // these are the gain controls.
    m_AFGain->SetValue(config_tree->get<float>("af.gain"));
    m_AFBWChoice->SetSelection(config_tree->get<int>("af.bw"));
    OnAFGainScroll(nullSE);  
    OnAFBWChoice(nullCE);
  

    controls->setCWSpeed(config_tree->get<float>("cw.speed"));
  
    controls->setSTGain(config_tree->get<float>("cw.sidetone"));


    tuner->setExtRefEna(config_tree->get<bool>("reference.source"));

    debugMsg("Updating spectrum widget\n");
    // update the spectrum widget
    spectrum_bandspread = config_tree->get<float>("spectrum.bandspread");
    spectrum_center_freq = config_tree->get<float>("spectrum.center_freq");
    
    spectrum_y_reflevel = config_tree->get<float>("spectrum.reflevel");
    spectrum_y_scale = config_tree->get<float>("spectrum.yscale");
    SetPerVals(spectrum_center_freq, spectrum_y_reflevel, spectrum_y_scale, spectrum_bandspread);
    UpdateCenterFreq(spectrum_center_freq);
    OnPerBandSpread(nullCE);
    if(config_tree->get<std::string>("spectrum.display") == "waterfall") {
      SpectrumDisplay->SetSelection(0);
    }
    else {
      SpectrumDisplay->SetSelection(1);
    }

    try {
      tx_rx_locked = config_tree->get<bool>("tx.tx_rx_locked");
    } catch (boost::exception const & ex) {
      tx_rx_locked = true;
    }
    OnTXRXLock(nullCE); 

    debugMsg("Setting Current Band\n");
    // now set the current band.
    SoDaRadio_Band * defband;
    try {
      defband = bandset->getByName(config_tree->get<std::string>("current_band"));
    } catch (boost::exception const & ex) {
      defband = bandset->getByIndex(0);
    }
    SetCurrentBand(defband);

    debugMsg("Completed LoadConfig\n");
    
    return true; 
  }

  void SoDaRadio_Top::setupBandSelect(SoDaRadio_BandSet * bandset)
  {
    int i, ct;
    ct = m_bandSelect->GetMenuItemCount();
    for(i = ct-1; i >= 0; i--) {
      wxMenuItem * mitem = m_bandSelect->FindItemByPosition(i);
      m_bandSelect->Delete(mitem);
    }
    // clear the band ID map. 
    wxID_to_band_idx_map.clear();

    int band_list_idx = 0;
    BOOST_FOREACH(SoDaRadio_BandSet::BandMapEntry b, bandset->band_map) {
      SoDaRadio_Band * v = b.second;
      debugMsg(boost::format("Creating band menuitem with string [%s]")
	       % v->getName().c_str()); 
      
      wxMenuItem * newItem = new wxMenuItem( m_bandSelect, wxID_ANY,
					     wxString(v->getName().c_str(), wxConvUTF8),
					     wxEmptyString, wxITEM_NORMAL); 
      m_bandSelect->Append(newItem);

      int id = newItem->GetId();
      // remember the correspondence between menu id and list index. 
      wxID_to_band_idx_map[id] = band_list_idx; 

      this->Connect(id, wxEVT_COMMAND_MENU_SELECTED,
		    wxCommandEventHandler(SoDaRadio_Top::OnBandSelect ));

      band_list_idx++; 
    }
  }
}
