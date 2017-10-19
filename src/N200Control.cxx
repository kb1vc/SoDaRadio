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

#include "N200Control.hxx"
#include <boost/format.hpp>
#include <boost/asio.hpp>
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
  N200Control::N200Control(uhd::usrp::multi_usrp::sptr _usrp,
			   int _mboard,
			   std::string ip_addr_str, 
			   unsigned int portnum) : TRControl() {

    mboard = _mboard; 
    usrp = _usrp; 
    

    skt = new IP::ClientSocket(ip_addr_str.c_str(), portnum, IP::UDP); 

    skt->setBlocking();

    // now check to see if it is for real. 
    try {
      setTXOff(); 
    }
    catch (IP::ReadTimeoutExc & ex) {
      // if we get here, then the device is not connected!  
      std::cerr << "No Serial TR switch control device found." << std::endl; 
      delete skt; 
      skt = NULL; 
    }
  }

  bool N200Control::setTXOn()
  {
    if(skt == NULL)  return false; 
    else {
      try {
	const char * cmd = "ON00NO\r";
	skt->putRaw((const void*) cmd, 7);
	char buf[20]; 
	skt->getRaw((void *) buf, 13, 100000); 
	// std::cerr << boost::format("\n\nON sent [%s]\n\n\t got [%s]\n\n")
	//   % cmd % buf; 
      } 
      catch (IP::ReadTimeoutExc & ex) {
	// if we get here, then the device is not connected!  
	std::cerr << "No Serial Controlled TR switch is connected to this N2xx....." << std::endl; 
	delete skt; 
	skt = NULL; 
	return false; 
      }
    }

    return true; 
  }

  bool N200Control::setTXOff()
  {
    if(skt == NULL) { return false; }
    else {
      try {
      const char * cmd = "ST00TS\r";
      skt->putRaw((const void *) cmd, 7);
      char buf[20]; 
      bzero(buf, 20);
      skt->getRaw((void *) buf, 13, 100000);
      // std::cerr << boost::format("\n\nOFF sent [%s]\n\n\t got [%s]\n\n")
      // 	% cmd % buf; 
      }
      catch (IP::ReadTimeoutExc & ex) {
	// if we get here, then the device is not connected!
	std::cerr << "No Serial Controlled TR switch is connected to this N2xx....." << std::endl; 	
	delete skt; 
	skt = NULL; 
	return false; 
      }
    }
    return true; 
  }

  bool N200Control::getTX()
  {
    return true; 
  }

  bool N200Control::setBand(unsigned int band, bool state) 
  { (void) band; (void) state; return false; }
  bool N200Control::getBand(unsigned int band) 
  { (void) band; return false; }

}
