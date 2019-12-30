/*
Copyright (c) 2013, Matthew H. Reilly (kb1vc)
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

#include "IPSockets.hxx"
#include "UDSockets.hxx"
#include <error.h>
#include <stdio.h>

void usage()
{
  fprintf(stderr, "IFServer_Test pathname\n");
  exit(-1); 
}

int main(int argc, char * argv[])
{
  int portnum;

  if(argc < 2) usage(); 

  int empty_count = 0;
  int iter_count = 0;
  int found_count = 0;
  
  SoDa::UD::ClientSocket * if_socket; 

  double old_freq = -10.0; 

  if_socket = new SoDa::UD::ClientSocket(argv[1]);
  std::cerr << "Created socket\n";
  int buf_size = 1024 * 1024;
  auto * buf = new char[buf_size];
  
  while(1) {
    int rs;
    while((rs = if_socket->get(buf, buf_size)) <= 0) {
      usleep(1000); 
    }

    unsigned long * len = reinterpret_cast<unsigned long*>(buf);
    double * freq = reinterpret_cast<double*>(&(buf[8])); // buf + sizeof(unsigned int));

    if(*freq != old_freq) {
      std::cerr << boost::format("Got new frequency: %10.6f\n") % (*freq * 1e-6);
      old_freq = *freq; 
    }
  }

  delete if_socket; 
}
