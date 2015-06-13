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

#include "TRControl.hxx"
#include "N200Control.hxx"
#include "B200Control.hxx"

#include <uhd/version.hpp>
#include <uhd/device.hpp>
#include <uhd/types/ranges.hpp>
#include <uhd/property_tree.hpp>
#include <boost/algorithm/string.hpp> //for split
#include <uhd/usrp/dboard_id.hpp>
#include <uhd/usrp/mboard_eeprom.hpp>
#include <uhd/usrp/dboard_eeprom.hpp>

namespace SoDa { 

  TRControl * TRControl::makeTRControl(uhd::usrp::multi_usrp::sptr usrp, int mboard) {
    // first figure out what kind of device we are... 
    uhd::property_tree::sptr tree = usrp->get_device()->get_tree(); 
    std::string mbname = tree->list("/mboards").at(mboard); 
    std::string modelname = tree->access<std::string>("/mboards/" + mbname + "/name").get();

    // now do something different for each one... 
    if ((modelname == std::string("N200")) || 
	(modelname == std::string("N210"))) {
      // find the IP address. 
      std::string ip_address; 
      ip_address = tree->access<uhd::usrp::mboard_eeprom_t>("/mboards/" + mbname + "/eeprom").get()["ip-addr"];       
      // std::cerr << boost::format("Got address = %s\n") % ip_address; 
      std::string gpsdo = tree->access<uhd::usrp::mboard_eeprom_t>("/mboards/" + mbname + "/eeprom").get()["gpsdo"];     
      if(gpsdo == std::string("none")) {
	return new N200Control(usrp, mboard, ip_address);
      }
      else {
	return new NoopControl;
      }

    }
    else if ((modelname == std::string("B200")) || 
	     (modelname == std::string("B210"))) {
      return new B200Control(usrp, mboard); 
    }
    else {
      return new NoopControl; 
    }
  }
}
