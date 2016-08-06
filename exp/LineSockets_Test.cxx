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

#include "LineSocket.hxx"
#include <error.h>
#include <stdio.h>

void usage()
{
  fprintf(stderr, "Sockets_Test is portnum\n");
  fprintf(stderr, "Sockets_Test ic portnum hostname\n");
  exit(-1); 
}

int main(int argc, char * argv[])
{
  int portnum;

  if(argc < 3) usage(); 

  int empty_count = 0;
  int iter_count = 0;
  int found_count = 0;
  
  SoDa::IP::LineServerSocket * s;
  SoDa::IP::ClientSocket * c; 
  if(strncmp(argv[1], "is", 2) == 0) {
    // server.
    portnum = atoi(argv[2]);
    s = new SoDa::IP::LineServerSocket(portnum);
    int con_count = 0; 
    while(con_count < 20) {
      while(!s->isReady()) {
	usleep(10000); 
      }
      con_count++; 
      char buf[1024];
      int rsize;
      int retcount = 0; 
      while((rsize = s->getLine(buf, 1024)) >= 0) {
	retcount++; 
	if(rsize > 0) {
	  std::cout << boost::format("Got line [%s] RC = %d\n") % buf % retcount; 
	  retcount = 0; 
	}
	else {
	  usleep(1000000);
	}
      }
      std::cout << "Disconnect." << std::endl; 
    }
  }
  else if (strncmp(argv[1], "ic", 2) == 0) {
    std::cout << "No idea what to do here." << std::endl; 
  }

}
