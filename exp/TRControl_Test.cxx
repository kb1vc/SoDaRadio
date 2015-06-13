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

#include "../src/N200Control.hxx"
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
    std::cerr << "Usage:  N200Control_Test  dev-arg-string" << std::endl;
    exit(-1); 
  }

  std::string devaddr(argv[1]); 
  uhd::device_addr_t rad(devaddr);

  uhd::usrp::multi_usrp::sptr usrp; 
  // make the device.
  usrp = uhd::usrp::multi_usrp::make(rad);

  std::vector<std::string> banks = usrp->get_gpio_banks(0);
  BOOST_FOREACH(std::string bank, banks) {
    std::cerr << boost::format("GPIO Bank [%s]\n") % bank;
  }


  SoDa::TRControl * ctrl = SoDa::TRControl::makeTRControl(usrp); 
 
  sleep(5);

  for(int i = 0; i < 8; i++) {
    ctrl->setTXOn(); 
    ctrl->setTXOff(); 
    sleep(2); 
  }
}
