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
//#include <error.h>
#include <stdio.h>
#include <iostream>

void usage()
{
  fprintf(stderr, "Sockets_Test is portnum\n");
  fprintf(stderr, "Sockets_Test ic portnum hostname\n");
  fprintf(stderr, "Sockets_Test us pathname\n");
  fprintf(stderr, "Sockets_Test uc pathname\n");
  exit(-1); 
}

int main(int argc, char * argv[])
{
  int portnum;

  if(argc < 3) usage(); 

  int empty_count = 0;
  int iter_count = 0;
  int found_count = 0;
  
  SoDa::IP::ServerSocket * s;
  SoDa::IP::ClientSocket * c; 
  SoDa::UD::ServerSocket * us;
  SoDa::UD::ClientSocket * uc; 
  if(strncmp(argv[1], "is", 2) == 0) {
    // server.
    portnum = atoi(argv[2]);
    s = new SoDa::IP::ServerSocket(portnum);
    int con_count = 0; 
    while(con_count < 20) {
      while(!s->isReady()) {
	usleep(1000); 
      }
      con_count++; 
      char buf[1024];
      int rsize;
      while((rsize = s->get(buf, 1024)) >= 0) {
	iter_count++; 
	if(rsize == 0) {
	  empty_count++;
	  usleep(10000);
	}
	if(rsize > 0) {
	  found_count++; 
	  const char * okmsg = "ok!";
	  std::cout << "message: [" << buf << "] length = " << rsize << std::endl;
	  std::cout << "iter_count = " << iter_count
		    << " empty_count = " << empty_count
		    << " found_count = " << found_count
		    << " con_count = " << con_count
		    << std::endl; 
	  s->put(okmsg, 4);
	  std::cout << "Hmmmm>" << std::endl; 
	}
	if(rsize < 0) {
	  perror("Hmmm... get returned a negative result."); 
	}
      }
    }
    std::cout << "FINAL iter_count = " << iter_count
	      << " empty_count = " << empty_count
	      << " found_count = " << found_count
	      << std::endl; 
    
  }
  else if (strncmp(argv[1], "ic", 2) == 0) {
    portnum = atoi(argv[2]);
    c = new SoDa::IP::ClientSocket(argv[3], portnum);
    const char * msg1 = "SET RX_IF_FREQ D 110e3\n";
    const char * msg2 = "SET STOP I 2\n";
    char buf[1024]; 
    c->put(msg1, strlen(msg1)+1);
    c->put(msg2, strlen(msg2)+1);
    int rs;
    while((rs = c->get(buf, 1024)) <= 0) {
      std::cout << "waiting" << std::endl ;
      usleep(1000000); 
    }
    
    std::cout << "message: [" << buf << "] length = " << rs << std::endl;
    //    rs = c->get(buf, 1024);
    std::cout << "message: [" << buf << "] length = " << rs << std::endl;
    delete c; 
  }
  else if(strncmp(argv[1], "us", 2) == 0) {
    // unix domain server.
    SoDa::UD::ServerSocket us(argv[2]);
    int con_count = 0; 
    while(con_count < 4) {
      us.put("Hey there sailor\n\n\n", 17);
      while(!us.isReady()) {
	usleep(1000); 
      }
      std::cerr << "About to get buffer\n"; 
      con_count++; 
      char buf[1024];
      int rsize;
      while((rsize = us.get(buf, 1024)) >= 0) {
	iter_count++; 
	if(rsize == 0) {
	  if(!us.isReady()) {
	    std::cerr << "socket no longer ready\n";
	    break; 
	  }
	  empty_count++;
	  usleep(10000);
	  if((empty_count % 256) == 0) std::cerr << "Empty buffer\n"; 
	}
	else if(rsize > 0) {
	  found_count++; 
	  const char * okmsg = "ok!";
	  std::cout << "message: [" << buf << "] length = " << rsize << std::endl;
	  std::cout << "iter_count = " << iter_count
		    << " empty_count = " << empty_count
		    << " found_count = " << found_count
		    << " con_count = " << con_count
		    << std::endl; 
	  us.put(okmsg, 4);
	  std::cout << "Hmmmm>" << std::endl; 
	}
	else if(rsize < 0) {
	  perror("Hmmm... get returned a negative result."); 
	}
      }
    }
    std::cout << "FINAL iter_count = " << iter_count
	      << " empty_count = " << empty_count
	      << " found_count = " << found_count
	      << std::endl; 
  }
  else if (strncmp(argv[1], "uc", 2) == 0) {
    SoDa::UD::ClientSocket uc(argv[2]);
    const char * msg1 = "SET RX_IF_FREQ D 110e3\n";
    const char * msg2 = "SET STOP I 2\n";
    char buf[1024]; 
    uc.put(msg1, strlen(msg1)+1);
    uc.put(msg2, strlen(msg2)+1);
    int rs;
    while((rs = uc.get(buf, 1024)) <= 0) {
      std::cout << "waiting" << std::endl ;
      usleep(1000000); 
    }
    
    std::cout << "message: [" << buf << "] length = " << rs << std::endl;
    //    rs = c->get(buf, 1024);
    std::cout << "message: [" << buf << "] length = " << rs << std::endl;
  }

}
