/*
  Copyright (c) 2015, Matthew H. Reilly (kb1vc)
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

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <boost/format.hpp>
#include <string>
#include <list>
#include <uhd/usrp/multi_usrp.hpp>
#include <vector>
#include <string>
#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include <uhd/utils/safe_main.hpp>
#include <uhd/version.hpp>
#include <uhd/device.hpp>
#include <uhd/types/ranges.hpp>
#include <uhd/property_tree.hpp>
#include <boost/algorithm/string.hpp> //for split
#include <uhd/usrp/dboard_id.hpp>
#include <uhd/usrp/mboard_eeprom.hpp>
#include <uhd/usrp/dboard_eeprom.hpp>


std::string getMbName(uhd::usrp::multi_usrp::sptr usrp)
{
  uhd::property_tree::sptr tree = usrp->get_device()->get_tree(); 
  std::string mbname = tree->list("/mboards").at(0); 
  return tree->access<std::string>("/mboards/" + mbname + "/name").get();
}

void dumpProps(uhd::usrp::multi_usrp::sptr usrp)
{
  uhd::property_tree::sptr tree = usrp->get_device()->get_tree(); 
  std::string mbname = tree->list("/mboards").at(0); 

  uhd::usrp::mboard_eeprom_t eeprom = tree->access<uhd::usrp::mboard_eeprom_t>("/mboards/" + mbname + "/eeprom").get();
  BOOST_FOREACH(const std::string & key, eeprom.keys()) {
    if( eeprom[key].empty() ) {
      std::cerr << boost::format("Empty key [%s]\n") % key; 
    }
    else {
      std::cerr << boost::format(" eeprom[%s] = [%s]\n") % key % eeprom[key]; 
    }
  }

  std::cerr << boost::format("went the direct route = [%s]\n")
    % tree->access<uhd::usrp::mboard_eeprom_t>("/mboards/" + mbname + "/eeprom").get()["ip-addr"]; 
}

int main(int argc, char ** argv)
{
  if(argc < 2) {
    std::cerr << "Usage:  IntN_FreqTable  dev-arg-string" << std::endl;
    exit(-1); 
  }

  std::string devaddr(argv[1]); 
  uhd::device_addr_t rad(devaddr);

  uhd::usrp::multi_usrp::sptr usrp; 
  // make the device.
  usrp = uhd::usrp::multi_usrp::make(rad);

  // now do the RX front end LO experiment.
  uhd::freq_range_t rx_rf_freq_range, tx_rf_freq_range; ///< property of the device -- what is the min/maximum RX frequency?
  rx_rf_freq_range = usrp->get_rx_freq_range();
  tx_rf_freq_range = usrp->get_tx_freq_range();

  uhd::tune_result_t tunres_int;
  
  // Now sweep from min to max freq, and find the steps along the way.
  double ff_incr = 1.0e6; // start with a small step...
  double r_st, r_en, target, target2;


  // now look through the range
  for(int i = 0; i < 2; i++) {
    r_st = (i == 0) ? rx_rf_freq_range.start() : tx_rf_freq_range.start();;    
    r_en = (i == 0) ? rx_rf_freq_range.stop() : tx_rf_freq_range.stop();;

    // first find the first setting. 
    uhd::tune_request_t treq(r_st);
    treq.args = uhd::device_addr_t("mode_n=integer");
    tunres_int = (i == 0) ? usrp->set_rx_freq(treq) : usrp->set_tx_freq(treq);
    target = tunres_int.actual_rf_freq;
    
    for(double ff = target + ff_incr;
	ff < r_en;
	ff += ff_incr) {
      uhd::tune_request_t treq(ff);
      treq.rf_freq = ff; 
      treq.rf_freq_policy = uhd::tune_request_t::POLICY_MANUAL;
      treq.args = uhd::device_addr_t("mode_n=integer");
      tunres_int = (i == 0) ? usrp->set_rx_freq(treq) : usrp->set_tx_freq(treq);

      std::string mode = (i == 0) ? "RX" : "TX"; 

      std::cerr << boost::format("%s Range Check RF_actual %lf DDC = %lf target = %lf requested RF = %lf ddc = %lf\n")
	% mode
	% (1e-6 * tunres_int.actual_rf_freq)
	% (1e-6 * tunres_int.actual_dsp_freq)
	% (1e-6 * ff)
	% (1e-6 * treq.rf_freq)
	% (1e-6 * treq.dsp_freq);
    
      if(target != tunres_int.actual_rf_freq) {
	// we found a new one...
	r_st = tunres_int.actual_rf_freq;
	target = r_st;
      }
    }
  }

}
