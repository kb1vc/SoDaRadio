/*
  Copyright (c) 2012, Matthew H. Reilly (kb1vc)
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

#include "SoDaBase.hxx"
#include <string>
#include <iostream>
#include <chrono>
#include "SoDaThread.hxx"

namespace SoDaTest {
  class ThN : public SoDa::Thread {
  public:
    ThN(int st) : SoDa::Thread("ThN") {
      sleep_time = st; 
    };
    std::string name;
    int sleep_time; 
    void run() override  {
      {
	sleep_ms(sleep_time);	
	std::cerr << name << std::endl;
      }
      return; 
    }
  };

  class Th0 : public ThN {
  public:
    Th0(int st) : ThN(st) { name = "zero\n"; }
  }; 
  class Th1 : public ThN {
  public:
    Th1(int st) : ThN(st) { name = "one\n"; }
  }; 
}

int main()
{
  SoDaTest::Th0 t0(1000);
  SoDaTest::Th1 t1(3000);

  t0.start();
  t1.start();

  t1.join();
  t0.join();
}
