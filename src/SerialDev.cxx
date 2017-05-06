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

#include "SerialDev.hxx"
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

// Boost free implementation of serial IO.   

namespace SoDa 
{


  SerialDev::SerialDev(const std::string & devname,
		       unsigned int speed, 
		       bool par_ena, 
		       bool par_even)
  {
    port = open(devname.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);

    if(port == -1) {
      // failed to open the port.  Throw an exception
      throw std::runtime_error((boost::format("Failed to open serial port [%s]\n") % devname).str());
    }

    // now set the speeds and such. 
    fcntl(port, F_SETFL, 0);

    struct termios port_settings;

    cfsetispeed(&port_settings, getSpeed(speed));
    cfsetospeed(&port_settings, getSpeed(speed));

    cfmakeraw(&port_settings);
    
    if(par_ena) {
      port_settings.c_cflag |= PARENB;
      if(par_even) {
	port_settings.c_cflag &= ~PARODD;
      }
      else {
	port_settings.c_cflag |= PARODD;
      }
    }
    else port_settings.c_cflag &= ~PARENB;

    
    
    port_settings.c_cflag &= ~CSTOPB;
    port_settings.c_cflag &= ~CSIZE;
    port_settings.c_cflag |= CS8; 

    port_settings.c_lflag &= ~ECHO;
    // port_settings.c_lflag &= ~ECHOK;
    // port_settings.c_lflag &= ~ECHOE;
    // port_settings.c_lflag &= ~ECHONL;
    // port_settings.c_lflag &= ~NOFLSH;
    // port_settings.c_lflag &= ~XCASE;
    // port_settings.c_lflag &= ~TOSTOP;
    // port_settings.c_lflag &= ~ECHOPRT;
    // port_settings.c_lflag &= ~ECHOCTL;
    // port_settings.c_lflag &= ~ECHOKE;

    port_settings.c_lflag &= ~ICANON;

    port_settings.c_cc[VMIN] = 0;
    port_settings.c_cc[VTIME] = 0;

    tcsetattr(port, TCSANOW, &port_settings);
  }


  bool SerialDev::putString(const std::string & str)
  {
    if(!serial_port_open) return false;

    write(port, str.c_str(), str.size());

    return true; 
  }

  bool SerialDev::getString(std::string & str, unsigned int maxlen)
  {
    (void) maxlen; 
    if(!serial_port_open) return false;

    str = std::string("");
    char c; 
    int itercount = 0; 
    while(1) {
      int len;
      c = '\000';
      len = read(port, &c, 1);
      if(len == 1) {
	switch(c) {
	case '\r': break; 
	case '\n': return true;
	default: str += c; 
	}
	itercount = 0; 
      }
      else if(len < 0) {
	std::cerr << boost::format("read returned %d  errno = %d\n")
	  % len % errno;
      }
      else if(len == 0) {
	usleep(1000);
	itercount++; 
	if (itercount > 100000) {
	  std::cerr << itercount << std::endl;
	  return false; 
	}
      }
    }
  }

  int SerialDev::getSpeed(int sp) 
  {
    switch(sp) {
    case 9600: return B9600;
    case 2400: return B2400;
    case 4800: return B4800;
    case 19200: return B19200;
    case 38400: return B38400;
    case 57600: return B57600;
    case 115200: return B115200;
    default: return B9600;
    }
  }


  bool SerialDev::flushInput() { 
    tcflush(port, TCIFLUSH); 
    return true; 
  }

  bool SerialDev::flushOutput() { 
    tcflush(port, TCOFLUSH); 
    return true; 
  }


  bool SerialDev::palindromeCommand(std::string & str) 
  {
    std::string resp;
    int stlen = str.size(); 

    std::string exp = (boost::format("OK [%s]") % (str.substr(0, stlen-1))).str();

    putString(str); 
    int i; 
    for(i = 0; i < 10; i++) { // 10 retries
      std::cerr << boost::format("loop %d start\n") % i; 
      while(!getString(resp, stlen + 5)) {
	usleep(1000);
      }
      int v; 
      if((v = exp.compare(0, stlen + 3, resp)) == 0) {
	std::cerr << "YAY!" << std::endl; 
	return true; 
      }
      else {
	std::cerr << boost::format("Expected [%s] got [%s]. stlen = %d compare = %d\n")
	  % exp % resp % stlen % v; 
	int j; 
	for(j = 0; j < stlen+5; j++) {
	  int cmp = (exp[i] == resp[i]); 
	  std::cerr << boost::format("%c %c (%d %d) =? %d\n")
	    % exp[j] % resp[j] % ((int) exp[j]) % ((int) resp[j]) % cmp;
	}
	flushInput();
	if(resp.compare(0, 3, "BAD") == 0) {
	  std::cerr << "flushing input buffer" << std::endl; 
	  flushInput(); 
	}
	std::cerr << boost::format("Resend [%s]\n") % str;
	putString(str); 	
	std::cerr << boost::format("loop %d end\n") % i; 
      }
    }

    return false; 
  }
}
