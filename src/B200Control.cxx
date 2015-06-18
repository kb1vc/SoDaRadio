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

#include "B200Control.hxx"
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/assign.hpp>
#include <math.h>
#include <stdexcept>


#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <time.h>

#include <strings.h>


namespace SoDa 
{
  /* borrowed from Ettus examples gpio.cpp */
 static std::string to_bit_string(boost::uint32_t val, const size_t num_bits)
  {
    std::string out;
    for (int i = num_bits - 1; i >= 0; i--)
      {
        std::string bit = ((val >> i) & 1) ? "1" : "0";
        out += "  ";
        out += bit;
      }
    return out;
  }
  
  static void output_reg_values(
				const std::string bank,
				const uhd::usrp::multi_usrp::sptr &usrp,
				const size_t num_bits)
  {
    std::vector<std::string> attrs = 
      boost::assign::list_of("CTRL")("DDR")("ATR_0X")("ATR_RX")
      ("ATR_TX")("ATR_XX")("OUT")("READBACK");

    std::cout << (boost::format("%10s ") % "Bit");
    for (int i = num_bits - 1; i >= 0; i--)
      std::cout << (boost::format(" %2d") % i);
    std::cout << std::endl;
    BOOST_FOREACH(std::string &attr, attrs)
      {
        std::cout << (boost::format("%10s:%s")
		      % attr % to_bit_string(boost::uint32_t(usrp->get_gpio_attr(bank, attr)), num_bits))
		  << std::endl;
      }
  }

 
  
  B200Control::B200Control(uhd::usrp::multi_usrp::sptr _usrp, int _mboard) : TRControl() {
    usrp = _usrp;
    mboard = _mboard; 

    std::vector<std::string> port = usrp->get_gpio_banks(0); 

    std::cerr << "Here in b200control init" << std::endl; 

    std::vector<std::string> attrs; 
    attrs.push_back("CTRL");
    attrs.push_back("DDR");
    attrs.push_back("OUT");
    attrs.push_back("ATR_0X");
    attrs.push_back("ATR_RX");
    attrs.push_back("ATR_TX");
    attrs.push_back("ATR_XX");
    attrs.push_back("READBACK");

    unsigned int st; 

    usrp->set_gpio_attr("FP0", "DDR", 0x300, 0x300);
    usrp->set_gpio_attr("FP0", "OUT", 0, 0x300);
    usrp->set_gpio_attr("FP0", "CTRL", 0x0, 0x300);  

    usrp->set_gpio_attr("FP0", "DDR", 0xffffffff, 0x300);    
    output_reg_values("FP0", usrp, 11); 
    while(1) {
      sleep(5); 
      std::cerr << "ON 1?" << std::endl; 
      usrp->set_gpio_attr("FP0", "OUT", 0x100, 0x300);
      output_reg_values("FP0", usrp, 11);  
      sleep(5);
      std::cerr << "ON 2?" << std::endl;       
      usrp->set_gpio_attr("FP0", "OUT", 0x200, 0x300);
      output_reg_values("FP0", usrp, 11);     

    }
    std::cerr << "DID IT!" << std::endl;

    sleep(10);
    usrp->set_gpio_attr("FP0", "OUT", 0x300, 0x300);
    output_reg_values("FP0", usrp, 11);     
    std::cerr << "woke up!" << std::endl;
    sleep(10);
    for(int i = 0; i < 11; i++) {
      std::cerr << boost::format("%d ---\n") % i; 
      usrp->set_gpio_attr("FP0", "OUT", (1 << i), 0x3ff);
      sleep(3);       
    }
    usrp->set_gpio_attr("FP0", "OUT", 1, 0x3ff);
    sleep(1); 


    port.push_back("FP0");
    BOOST_FOREACH(std::string ps, port) {
      BOOST_FOREACH(std::string at, attrs) {
	try {
	  st = usrp->get_gpio_attr(ps, at, 0);
	  std::cerr << boost::format("port [%s] attr [%s] = 0x%x\n")	  
	    % ps % at % st; 
	  
	}
	catch (uhd::lookup_error & v) {
	  std::cerr << boost::format("port [%s] attr [%s] lookup error.\n")
	    % ps % at; 
	}
      }
    }
    usrp->set_gpio_attr("FP0", "OUT", 1, 0x3ff);
    sleep(1); 

  }

  bool B200Control::setTXOn()
  {
    return true; 
  }

  bool B200Control::setTXOff()
  {
    return true; 
  }

  bool B200Control::getTX()
  {
    return true; 
  }

  bool B200Control::setBand(unsigned int band, bool state) 
  { return false; }
  bool B200Control::getBand(unsigned int band) 
  { return false; }



}
