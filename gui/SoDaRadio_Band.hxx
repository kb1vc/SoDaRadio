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
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

class SoDaRadio_Band {
public:
  SoDaRadio_Band(boost::property_tree::ptree * banditem) {
    
    std::cerr << "About to read bandname." << std::endl; 
    band_name = banditem->get<std::string>("name");
    std::cerr << boost::format("got band name [%s]\n") % band_name; 
    upper_band_edge = banditem->get<double>("upper_band_edge");
    lower_band_edge = banditem->get<double>("lower_band_edge");

    last_tx_freq = banditem->get<double>("last_tx_freq");
    last_rx_freq = banditem->get<double>("last_rx_freq");

    rx_antenna_choice = banditem->get<std::string>("rx_antenna_choice");
    default_mode = banditem->get<std::string>("default_mode");

    enable_transmit = banditem->get<bool>("enable_transmit");

    band_id = banditem->get<unsigned char>("band_id");

    transverter_mode = banditem->get<bool>("transverter_mode");
    if(transverter_mode) {
      transverter_lo_freq = banditem->get<double>("transverter_lo_freq");
      transverter_multiplier = banditem->get<double>("transverter_multiplier");
      low_side_injection = banditem->get<bool>("low_side_injection"); 
    }
  }

  SoDaRadio_Band() {
    return; 
  }
  
  SoDaRadio_Band(std::string name, double lower, double upper,
		 std::string mode, 
		 std::string rx_ant,
		 unsigned char _band_id, 
		 bool tx_ena) {
    setupBand(name, lower, upper, mode, rx_ant, _band_id, tx_ena);
  }

  void setupBand(std::string name, double lower, double upper,
		 std::string mode, 
		 std::string rx_ant,
		 unsigned char _band_id,
		 bool tx_ena) {
    transverter_mode = false;
    band_name = name;
    upper_band_edge = upper;
    lower_band_edge = lower;
    rx_antenna_choice = rx_ant;
    default_mode = mode;
    enable_transmit = tx_ena;
    band_id = _band_id; 
    last_tx_freq = lower;
    last_rx_freq = lower; 
  }

  void setupTransverter(double lo_freq, double mult, bool low_side) {
    transverter_mode = true;
    transverter_lo_freq = lo_freq; 
    transverter_multiplier = mult; 
    low_side_injection = low_side; 
  }

  void save(boost::property_tree::ptree * config_tree) {

    boost::property_tree::ptree band;
    std::cerr << boost::format("Saving band [%s] ube = %g lbe = %g\n")
      % band_name % upper_band_edge % lower_band_edge; 
    band.put("name", band_name);
    band.put("upper_band_edge", upper_band_edge);
    band.put("lower_band_edge", lower_band_edge);
    band.put("last_rx_freq", last_rx_freq);
    band.put("last_tx_freq", last_tx_freq);
    band.put("rx_antenna_choice", rx_antenna_choice);
    band.put("enable_transmit", enable_transmit); 
    band.put("default_mode", default_mode);
    band.put("band_id", band_id);

    if(transverter_mode) {
      band.put("transverter_mode", true);
      band.put("transverter_lo_freq", transverter_lo_freq);
      band.put("transverter_multiplier", transverter_multiplier);
      band.put("low_side_injection", low_side_injection); 
    }
    else {
      band.put("transverter_mode", false); 
    }

    config_tree->add_child("SoDaRadio.bands.band", band);
  }
  
  bool inBand(double freq) {
    return ((freq >= lower_band_edge) && (freq <= upper_band_edge)); 
  }
    
  bool getTXEnable() { return enable_transmit; }

  bool isNamed(std::string & name) { return band_name == name; }

  std::string & getName() { return band_name; }
  
public:

  std::string band_name; ///< The name of this band (10GHz, or Air VHF)
  double upper_band_edge; ///< the lower frequency limit for this band
  double lower_band_edge; ///< the upper frequency limit for this band
  double last_rx_freq;    ///< last RX freq selected for this band
  double last_tx_freq;    ///< last RX freq selected for this band
  bool enable_transmit; 
  std::string default_mode; ///< what is the "default" choice for modulation type?

  bool transverter_mode; ///< if true, we use a transverter for this band.
  double transverter_lo_freq; ///< this is the LO freq that we'll measure for calibration purposes.
  double transverter_multiplier; ///< actual freq = tuned_freq + lo_freq * mult
  bool low_side_injection; 

  std::string rx_antenna_choice;
  unsigned char band_id; ///< an 8 bit specifier to select the band on an external bandswitch.
}; 

class SoDaRadio_BandSet {
public:
  SoDaRadio_BandSet(boost::property_tree::ptree * config_tree) {
    std::cerr << boost::format("in bandset create:\n");
    if(!config_tree->get_child_optional("bands")) {
      std::cerr << boost::format("in bandset create no children found.\n");      
      return;
    }

    BOOST_FOREACH(boost::property_tree::ptree::value_type & v, config_tree->get_child("bands") ) {
      std::cerr << boost::format("in bandset create with v.first = %s\n") % v.first;
      if(v.first == "band") {
	SoDaRadio_Band * nb = new SoDaRadio_Band(&(v.second));
	add(nb);
      }
    }
  }
  
  void save(boost::property_tree::ptree * config_tree) {
    for(std::list< SoDaRadio_Band * >::iterator bi = band_list.begin();
	bi != band_list.end();
	++bi) {
      std::cerr << boost::format("Saving band [%s]\n") % (*bi)->getName();
      (*bi)->save(config_tree); 
    }    
  }

  void add(SoDaRadio_Band * band) {
    std::cerr << boost::format("Adding band [%s] freq range [%lg %lg]\n")
      % band->getName() % band->lower_band_edge % band->upper_band_edge; 
    band_list.push_back(band); 
  }

  SoDaRadio_Band * getByName(std::string & name) {
    for(std::list< SoDaRadio_Band * >::iterator bi = band_list.begin();
	bi != band_list.end();
	++bi) {
      SoDaRadio_Band * r = *bi;
      if(r->isNamed(name)) return r; 
    }
    return NULL; 
  }

  SoDaRadio_Band * getByIndex(int idx) {
    int i = 0; 
    for(std::list< SoDaRadio_Band * >::iterator bi = band_list.begin();
	bi != band_list.end();
	++bi, i++) {
      SoDaRadio_Band * r = *bi;
      if(i == idx) return r; 
    }
    return NULL; 
  }
  
  SoDaRadio_Band * getByFreq(double freq) {
    SoDaRadio_Band * ret = NULL;
    double smallest_range = 1e12; // find the best match.
    BOOST_FOREACH(SoDaRadio_Band * v, band_list) {
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
    if(bli != band_list.end()) {
      SoDaRadio_Band * r = *bli; 
      ++bli; 
      return r->getName(); 
    }
    else return std::string(""); 
  }

  std::string getFirstName() {
    bli = band_list.begin();
    return getNextName(); 
  }

  std::list< SoDaRadio_Band * > band_list; 
private:
  std::list< SoDaRadio_Band * >::iterator bli; 
}; 
#endif
