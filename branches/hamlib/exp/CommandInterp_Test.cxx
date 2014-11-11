/*
Copyright (c) 2014, Matthew H. Reilly (kb1vc)
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

#include "CommandInterpreter.hxx"
#include <iostream>
#include <boost/format.hpp>
 
class MyTest { 
public:
  
  MyTest() { 
    ci.setHandlerObj(this);
    ci.makeCommand(std::string("F"), std::string("set_freq"), &MyTest::setFreq);
    ci.makeCommand(std::string("f"), std::string("get_freq"), &MyTest::getFreq); 
  }

  void parse(const std::string & cmdbuf) {
    ci.parse(cmdbuf);
  }

  bool setFreq(std::vector<std::string> & cmd_line) {
    std::cerr << boost::format("Got setFreq [%s]\n") % cmd_line[0]; 
  }

  bool getFreq(std::vector<std::string> & cmd_line) {
    std::cerr << boost::format("Got getFreq [%s]\n") % cmd_line[0]; 
  }

  SoDa::CommandInterpreter<MyTest> ci; 
}; 

int main(int argc, char * argv[])
{
  MyTest mt; 

  mt.parse(std::string("F 3.145e6"));
  mt.parse(std::string("f 3.145e6"));
  mt.parse(std::string("set_freq 3.145e6"));  
  mt.parse(std::string("get_freq 3.145e6"));  
}
