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

#include "TSIP.hxx"
#include <iostream>
#include <stdlib.h>
#include <string>
#include <list>
int main(int argc, char ** argv)
{
  if(argc < 2) {
    std::cerr << "Usage:  TSIPTest <commportforgps>" << std::endl;
    exit(-1); 
  }

  TSIP::PrimaryTimingReport ptr;
  TSIP::SuplementalTimingReport str;
  std::list<TSIP::Report*> replist;
  replist.push_back(&ptr);
  replist.push_back(&str); 
  
  TSIP::Reader tr(std::string(argv[1]), replist);
  
  while(1) {
    TSIP::Report * r = tr.readStream();
    if(r != NULL) {
      std::cerr << "About to report" << std::endl;
      r->updateStatus();
    }
    else {
      std::cerr << "readStream returned null pointer" << std::endl;
      tr.dumpBuffer(std::cerr);
    }
  }
  
}
