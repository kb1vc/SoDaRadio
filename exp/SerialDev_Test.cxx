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

#include "../src/SerialDev.hxx"
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <boost/format.hpp>
#include <string>
#include <list>
int main(int argc, char ** argv)
{
  if(argc < 2) {
    std::cerr << "Usage:  SerialDev_Test <serial_dev>" << std::endl;
    exit(-1); 
  }

  std::string devname(argv[1]); 
  SoDa::SerialDev sport(devname);
 
  sleep(5);
 
  std::string on("ON00NO");
  std::string off("ST00TS");

  char ctlj = 0xa; 
  on = on + ctlj; 
  off = off + ctlj;
  std::string repl; 
  std::cerr << "Need to add synchronization here --- for arduino... send a challenge/response that gives us a starting point for synchronization" << std::endl;

  while(1) {
    for(int i = 0; i < 4; i++) {
      on = (boost::format("ON%d%dNO%c") % i % i % ctlj).str();
      sport.putString(on); 
      sport.getString(repl, 100); 
      std::cerr << boost::format("on: got reply [%s]\n") % repl; 
      sleep(2);
      off = (boost::format("ST%d%dTS%c") % i % i % ctlj).str();      
      sport.putString(off); 
      sport.getString(repl, 100); 

      if(repl.compare(0, 3, "BAD") == 0) {
	std::cerr << "flushing input buffer" << std::endl; 
	sport.flushInput(); 
      }

      std::cerr << boost::format("off: got reply [%s]\n") % repl;
      sleep(2); 
    }
  }
  
}
