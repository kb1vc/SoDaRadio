/*
  Copyright (c) 2012, 2013, 2014, Matthew H. Reilly (kb1vc)
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

#ifndef SODARADIO_BAND_HDR
#define SODARADIO_BAND_HDR
#include <string>
#include <iostream>
#include <list>
#include <map>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "../src/Debug.hxx"

class SoDaRadio_Band : public SoDa::Debug {
public:
  SoDaRadio_Band(boost::property_tree::ptree * banditem) :
    SoDa::Debug("SoDaRadio_Band") {
    
    band_name = banditem->get<std::string>("name");
    debugMsg(boost::format("creating band %s") % band_name);
    upper_band_edge = banditem->get<double>("upper_band_edge");
    lower_band_edge = banditem->get<double>("lower_band_edge");

    last_tx_freq = banditem->get<double>("last_tx_freq");
    last_rx_freq = banditem->get<double>("last_rx_freq");

    if(banditem->count("rx_antenna_choice") != 0) {
      rx_antenna_choice = banditem->get<std::string>("rx_antenna_choice");
    } else {
      rx_antenna_choice = "TX/RX"; 
    }

    if(banditem->count("tx_antenna_choice") != 0) {
      tx_antenna_choice = banditem->get<std::string>("tx_antenna_choice");
    } else {
      tx_antenna_choice = "TX/RX"; 
    }

    default_mode = banditem->get<std::string>("default_mode");

    enable_transmit = banditem->get<bool>("enable_transmit");

    band_id = banditem->get<unsigned char>("band_id");

    transverter_mode = banditem->get<bool>("transverter_mode");

    if(transverter_mode) {
      debugMsg("in transverter mode");
      transverter_local_lo = banditem->get<bool>("transverter_local_lo", false);
      debugMsg(boost::format("Transverter mode with transverter_local_lo = %d") % transverter_local_lo);
      transverter_lo_freq = banditem->get<double>("transverter_lo_freq");
      transverter_multiplier = banditem->get<double>("transverter_multiplier");
      low_side_injection = banditem->get<bool>("low_side_injection"); 
      debugMsg("completed transverter mode");
    }
    else {
      transverter_local_lo = false;
    }


    try {
      tx_rx_locked = banditem->get<bool>("tx.rx_locked");
    } catch (boost::exception const & ex) {
      tx_rx_locked = true;
    }
    try {
      tx_rf_outpower = banditem->get<float>("tx.outpower");
    } catch (boost::exception const & ex) {
      tx_rf_outpower = 0.0;
    }
    try {
      af_gain = banditem->get<float>("af.gain");
    } catch (boost::exception const & ex) {
      af_gain = 0.0;
    }
    try {
      af_bw = banditem->get<int>("af.bw");
    } catch (boost::exception const & ex) {
      af_bw = 2;
    }
    try {
      rf_gain = banditem->get<float>("rf.gain");
    } catch (boost::exception const & ex) {
      rf_gain = 0.0;
    }
    
  }

  SoDaRadio_Band() : SoDa::Debug("SoDaRadio_Band-empty") {
     return; 
  }
  
  SoDaRadio_Band(std::string name, double lower, double upper,
		 std::string mode, 
		 std::string rx_ant,
		 std::string tx_ant,		 
		 unsigned char _band_id, 
		 bool tx_ena) : SoDa::Debug("SoDaRadio_Band") {
    setupBand(name, lower, upper, mode, rx_ant, tx_ant,_band_id, tx_ena);
  }

  void setupBand(std::string name, double lower, double upper,
		 std::string mode, 
		 std::string rx_ant,
		 std::string tx_ant,		 
		 unsigned char _band_id,
		 bool tx_ena) {
    transverter_mode = false;
    transverter_local_lo = false; 
    band_name = name;
    upper_band_edge = upper;
    lower_band_edge = lower;
    rx_antenna_choice = rx_ant;
    tx_antenna_choice = tx_ena ? tx_ant : std::string("");    
    default_mode = mode;
    enable_transmit = tx_ena;
    band_id = _band_id; 
    last_tx_freq = lower;
    last_rx_freq = lower; 

    af_gain = 0.0;
    rf_gain = 0.0;
    af_bw = 2; 

    tx_rf_outpower = 0.0;
    tx_rx_locked = true;
  }

  void setupTransverter(double lo_freq, double mult, bool low_side, bool _local_lo) {
    transverter_mode = true;
    transverter_local_lo = _local_lo;
    transverter_lo_freq = lo_freq; 
    transverter_multiplier = mult; 
    low_side_injection = low_side; 
  }

  
  void save(boost::property_tree::ptree * config_tree) {
    boost::property_tree::ptree band;
    
    band.put("name", band_name);
    band.put("upper_band_edge", upper_band_edge);
    band.put("lower_band_edge", lower_band_edge);
    band.put("last_rx_freq", last_rx_freq);
    band.put("last_tx_freq", last_tx_freq);
    band.put("rx_antenna_choice", rx_antenna_choice);
    band.put("tx_antenna_choice", tx_antenna_choice);    
    band.put("enable_transmit", enable_transmit); 
    band.put("default_mode", default_mode);
    band.put("band_id", band_id);

    band.put("af.gain", af_gain);
    band.put("rf.gain", rf_gain);
    band.put("af.bw", af_bw);

    band.put("tx.outpower", tx_rf_outpower);
    band.put("tx.rx_locked", tx_rx_locked);

    if(transverter_mode) {
      band.put("transverter_mode", true);
      band.put("transverter_local_lo", transverter_local_lo);
      band.put("transverter_lo_freq", transverter_lo_freq);
      band.put("transverter_multiplier", transverter_multiplier);
      band.put("low_side_injection", low_side_injection); 
    }
    else {
      band.put("transverter_mode", false); 
    }

    debugMsg(boost::format("sodaradio band sees config_tree = %p\n") % config_tree); 
    config_tree->add_child("SoDaRadio.bands.band", band);
    debugMsg(boost::format("sodaradio band sees config_tree = %p after add_child\n") % config_tree); 
  }
  
  bool inBand(double freq) {
    return ((freq >= lower_band_edge) && (freq <= upper_band_edge)); 
  }
    
  bool getTXEnable() { return enable_transmit; }

  bool isNamed(const std::string & name) { return band_name == name; }

  std::string & getName() { return band_name; }

  void setRXControls(float af, float rf, int bw) {
    af_gain = af;
    rf_gain = rf;
    af_bw = bw; 
  }

  void setTXControls(float power, bool r_t_l) {
    tx_rf_outpower = power;
    tx_rx_locked = r_t_l;
  }
public:

  std::string band_name; ///< The name of this band (10GHz, or Air VHF)
  double upper_band_edge; ///< the lower frequency limit for this band
  double lower_band_edge; ///< the upper frequency limit for this band
  double last_rx_freq;    ///< last RX freq selected for this band
  double last_tx_freq;    ///< last RX freq selected for this band
  bool enable_transmit; 
  std::string default_mode; ///< what is the "default" choice for modulation type?

  bool transverter_mode; ///< if true, we use a transverter for this band.
  bool transverter_local_lo; ///< if true, the LO for the transverter is generated on the USRP 2nd TX channel
  double transverter_lo_freq; ///< this is the LO freq that we'll measure for calibration purposes.
  double transverter_multiplier; ///< actual freq = tuned_freq + lo_freq * mult
  bool low_side_injection;

  // transmitter settings
  float tx_rf_outpower; ///< transmitter power setting
  bool tx_rx_locked; ///< lock transmit and receive frequencies
  
  // receiver gain settings
  float af_gain; ///< receiver AF gain
  float rf_gain; ///< receiver RF gain
  int af_bw; ///< receiver AF bandwidth -- select filter

  std::string rx_antenna_choice; ///< choose a port  for RX
  std::string tx_antenna_choice; ///< choose a port  for TX
  
  unsigned char band_id; ///< an 8 bit specifier to select the band on an external bandswitch.
}; 

class SoDaRadio_BandSet : public SoDa::Debug {
public:
  
  typedef std::pair<std::string, SoDaRadio_Band*> BandMapEntry;
  
  SoDaRadio_BandSet(boost::property_tree::ptree * config_tree) :
    SoDa::Debug("SoDaRadio_BandSet")
  {
    debugMsg(boost::format("sodaradio bandset const sees config_tree = %p\n") % config_tree); 
    if(!config_tree->get_child_optional("bands")) {
      return;
    }

    for(auto v :  config_tree->get_child("bands") ) {
      if(v.first == "band") {
	SoDaRadio_Band * nb = new SoDaRadio_Band(&(v.second));
	add(nb);
      }
    }
    debugMsg(boost::format("sodaradio bandset const sees config_tree = %p at end\n") % config_tree); 
  }
  
  void save(boost::property_tree::ptree * config_tree) {
    debugMsg(boost::format("sodaradio bandset save const sees config_tree = %p\n") % config_tree); 
    for(BandMapEntry & b : band_map) {
      b.second->save(config_tree);
    }
    debugMsg(boost::format("sodaradio bandset save const sees config_tree = %p at end\n") % config_tree); 
  }

  void add(SoDaRadio_Band * band) {
    band_map[band->getName()] = band;
  }

  SoDaRadio_Band * getByName(const std::string name) {
    if(band_map.find(name) != band_map.end()) {
      return band_map[name]; 
    }
    else return NULL; 
  }

  SoDaRadio_Band * getByIndex(int idx) {
    int i = 0;
    for(BandMapEntry & b : band_map) {
      if(i == idx) return b.second;
      i++; 
    }
    return NULL; 
  }
  
  SoDaRadio_Band * getByFreq(double freq) {
    SoDaRadio_Band * ret = NULL;
    double smallest_range = 1e12; // find the best match.
    for(BandMapEntry & b : band_map) {
      SoDaRadio_Band * v = b.second; 
      if((v->lower_band_edge <= freq) && (v->upper_band_edge >= freq)) {
	double diff = v->upper_band_edge - v->lower_band_edge;
	if(diff < smallest_range) {
	  ret = v;
	  smallest_range = diff; 
	}
      }
    }
    return ret; 
  }

  std::string getNextName() {
    if(bli != band_map.end()) {
      SoDaRadio_Band * r = (*bli).second; 
      ++bli; 
      return r->getName(); 
    }
    else return std::string(""); 
  }

  std::string getFirstName() {
    bli = band_map.begin();
    return getNextName(); 
  }

  std::map< std::string, SoDaRadio_Band * > band_map; 
private:
  std::map< std::string,  SoDaRadio_Band * >::iterator bli;
}; 
#endif

