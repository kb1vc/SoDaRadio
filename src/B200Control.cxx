/*
  Copyright (c) 2015, 2022 Matthew H. Reilly (kb1vc)
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
#include <math.h>
#include <stdexcept>


#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <time.h>

#include <strings.h>
#include <iostream>

//new
#include "USRPPropTree.hxx"
#include <uhd/property_tree.hpp>

namespace SoDa 
{
  B200Control::B200Control(uhd::usrp::multi_usrp::sptr _usrp, int _mboard) : TRControl() {
    usrp = _usrp;
    mboard = _mboard; 
  
    //new
    USRPPropTree tree(usrp, "TRControl");
    B200Control::modelname = tree.getStringProp("name", "unknown");
    if ((modelname == std::string("B200")) ) {
        B200_type = SoDa_B200_type_t::eB200;};
    if ((modelname == std::string("B210")) ) {
        B200Control::B200_type = SoDa_B200_type_t::eB210;};
    if ((modelname == std::string("B200mini")) ) {
        B200Control::B200_type = SoDa_B200_type_t::eB200mini;};
    if ((modelname == std::string("B200mini-i")) ) {
        B200Control::B200_type = SoDa_B200_type_t::eB200mini_i;};
    if ((modelname == std::string("B205mini-i")) ) {
        B200Control::B200_type = SoDa_B200_type_t::eB205mini_i;};

    // new debug
    std::vector<std::string> port = usrp->get_gpio_banks(0); 
    std::cerr << "port list :" << port[0] << std::endl;  
      
    switch(B200Control::B200_type) {
      case SoDa_B200_type_t::eB200mini : 
        // GPIO PIN 1 is used for TX/RX Control manual (no ATR)
        // GPIO PIN 2-4 are used for Band Setting manual (no ATR)
        // -> 0x0F is the mask , 0x0 -> manual (0x1 = ATR)
        usrp->set_gpio_attr("FP0", "CTRL", 0x00, 0x0F);  
        // setting Data DiRection 0x1 = output
        usrp->set_gpio_attr("FP0", "DDR", 0x0f, 0x0F);
        // reset GPIO pin 0,1,2,3
        usrp->set_gpio_attr("FP0", "OUT", 0x00, 0x0F);
        /*
        std::cerr << "GPIO Ctrl: " << usrp->get_gpio_attr("FP0","CTRL") << std::endl;
        std::cerr << "GPIO DDR : " << usrp->get_gpio_attr("FP0","DDR") << std::endl;
        std::cerr << "GPIO OUT : " << usrp->get_gpio_attr("FP0","OUT") << std::endl;
        */
        
        setTXOff();
        break;
        
      default: //default runs the original code
        //original code begin
        std::vector<std::string> port = usrp->get_gpio_banks(0); 

        // we set J400 pin 1 HIGH on transmit, LOW on RX.   
        // this is becaus the "default" drive on the pins
        // if the wrong fpga binary is loaded, the transmitter
        // TR switch will be left in the TX position. 
        // This may result in "no signal heard" but at least
        // it is less likely to result in "blown up radio"
        // Best configuratio, is to look at both pin 1 and pin 3
        // TX is when 1 is HIGH and 3 is LOW
        // RX is when 1 is LOW and 3 is HIGH
        // do something safe when they are both HIGH or both LOW.
        usrp->set_gpio_attr("FP0", "DDR", 0x300, 0x300);
        usrp->set_gpio_attr("FP0", "CTRL", 0x0, 0x300);  

        setTXOff();
        //original code end
    }
  }

  bool B200Control::setTXOn()
  {
    std::cerr << "B200Control::setTXOn()" << std::endl;
    
    switch (B200Control::B200_type) {
      case SoDa_B200_type_t::eB200mini : {
        usrp->set_gpio_attr("FP0", "OUT", 0x1, 0x01);
        break;
        }
      default :
        //original code
        usrp->set_gpio_attr("FP0", "OUT", 0x100, 0x300);
    } 
    return true; //original code
  }

  bool B200Control::setTXOff()
  {
    std::cerr << "B200Control::setTXOff()" << std::endl;

    switch (B200Control::B200_type) {
      case SoDa_B200_type_t::eB200mini : {
        usrp->set_gpio_attr("FP0", "OUT", 0x00, 0x01);
        break;        
        }
      default:
        //original code
        usrp->set_gpio_attr("FP0", "OUT", 0x200, 0x300);
    }
    return true; //original
    //return true; // duplicate in original
  }

  bool B200Control::getTX()
  {
    return true; 
  }

  bool B200Control::setBand(unsigned int band, bool state) 
  { (void) band; (void) state; return false; }
  bool B200Control::getBand(unsigned int band) 
  { (void) band; return false; }
}
