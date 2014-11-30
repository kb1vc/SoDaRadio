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

#include "IPSockets.hxx"

#include <iostream>
#include <unistd.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <boost/format.hpp>
#include <cstdio>

SoDa::IP::ServerSocket::ServerSocket(int portnum, bool localhost_only) :
  SoDa::Debug((boost::format("Server:%d") % portnum).str())
{
  int stat; 
  // create the socket. 
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
    
  if(server_socket < 0) {
    std::cerr << "Failed to create server socket... I quit." << std::endl;
    exit(-1); 
  }

  int x = fcntl(server_socket, F_GETFL, 0);
  fcntl(server_socket, F_SETFL, x | O_NONBLOCK);
  
  // setup the server address
  bzero((char*) &server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;
  if(localhost_only) {
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
  }
  else {
    server_address.sin_addr.s_addr = INADDR_ANY;
  }
  server_address.sin_port = htons(portnum);

  // now bind it
  if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
    std::cerr << "Couldn't bind socket at port number " << portnum << " I quit." << std::endl;
    exit(-1); 
  }

  // now let the world know that we're ready for one and only one connection.
  stat = listen(server_socket, 5);
  if(stat < 0) {
    std::cerr << "Couldn't listen on port number " << portnum << " got " << errno << " I quit." << std::endl; 
    exit(-1); 
  }

  // mark the socket as "not ready" for input -- it needs to accept first. 
  ready = false; 
}

SoDa::IP::ClientSocket::ClientSocket(const char * hostname, int portnum) :
  SoDa::Debug((boost::format("Client:%d") % portnum).str())
{
  conn_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(conn_socket < 0) {
    std::cerr << "Failed to create client socket... I quit." << std::endl;
    exit(-1); 
  }

  server = gethostbyname(hostname);
  if(server == NULL) {
    std::cerr << "Couldn't find server named [" << hostname << "]... I quit." << std::endl;
    exit(-1); 
  }

  bzero((char*) &server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;
  bcopy((char *) server->h_addr, (char*) &server_address.sin_addr.s_addr, server->h_length);
  server_address.sin_port = htons(portnum);

  if(connect(conn_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
    std::cerr << "Couldn't connect to host [" << hostname << "] at port number " << portnum << " I quit." << std::endl;
    perror("oops.");
    exit(-1); 
  }

  int x = fcntl(conn_socket, F_GETFL, 0);
  fcntl(conn_socket, F_SETFL, x | O_NONBLOCK);

}

bool SoDa::IP::ServerSocket::isReady()
{
  if(ready) return true;
  else {
    socklen_t ca_len = sizeof(client_address);
    // note that we've set the server_socket to non-block, so if nobody is here,
    // we should get an EAGAIN or EWOULDBLOCK.
    int ns = accept(server_socket, (struct sockaddr *) & client_address, &ca_len);
    if(ns < 0) {
      ready = false; 
    }
    else {
      debugMsg("Got Connection.");
      conn_socket = ns;
      int x = fcntl(conn_socket, F_GETFL, 0);
      fcntl(conn_socket, F_SETFL, x | O_NONBLOCK);
      ready = true; 
    }
  }
  return ready; 
}

int SoDa::IP::NetSocket::loopWrite(int fd, const void * ptr, unsigned int nbytes)
{
  char * bptr = (char*) ptr;
  int left = nbytes;
  int stat;
  while(left > 0) {
    stat = write(fd, bptr, left);
    if(stat < 0) {
      if((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
	continue; 
      }
      else {
	return stat; 
      }
    }
    else {
      left -= stat;
      bptr += stat; 
    }
  }
}

int SoDa::IP::NetSocket::put(const void * ptr, unsigned int size)
{
  // we always put a buffer of bytes, preceded by a count of bytes to be sent.
  int stat;
  
  stat = loopWrite(conn_socket, &size, sizeof(unsigned int));
  if(stat < 0) return stat; 

  stat = loopWrite(conn_socket, ptr, size);

  return stat; 
}

int SoDa::IP::NetSocket::get(void * ptr, unsigned int size)
{
  int stat;
  unsigned int rsize;

  stat = read(conn_socket, &rsize, sizeof(unsigned int));
  if(stat <= 0) {
    if((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
      //      std::cerr << ">>>" << std::endl; 
      return 0; 
    }
    else {
      perror("Oops -- socket get -- "); 
      return stat;
    }
  }

  int got = 0;
  int left = rsize;
  char * bptr = (char*) ptr; 
  while(left > 0) {
    int ls = read(conn_socket, bptr, left);
    if(ls < 0) {
      if((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
	continue; 
      }
      else {
	perror("Ooops -- read buffer continued");
	return ls;
      }
    }
    else {
	left -= ls;
	bptr += ls; 
    }
  }
  
  if(rsize > size) {
    char dmy[100];
    unsigned int left = rsize - size;
    while(left != 0) {
      int ls; 
      ls = read(conn_socket, dmy, (left > 100) ? 100 : left);
      if(ls < 0) {
	if((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
	  continue; 
	}
	else {
	  perror("Ooops -- read buffer continued");
	  return ls;
	}
      }
      left -= ls; 
    }
  }

  return size; 

}


int SoDa::IP::NetSocket::writeBuf(const void * ptr, unsigned int size)
{
  // This writes a raw buffer, without the length as a preamble
  int stat;
  
  stat = loopWrite(conn_socket, ptr, size);

  // return the number of bytes written (if stat > 0). 
  return stat; 
}

int SoDa::IP::NetSocket::readBuf(void * ptr, unsigned int size)
{
  // this gets a raw buffer, without the length as a preamble

  int stat;

  stat = read(conn_socket, ptr, size);
  if(stat == 0) return -1;
  if(stat <= 0) {
    if((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
      return 0; 
    }
    else {
      perror("Oops -- socket get -- "); 
      return stat;
    }
  }

  return stat; 

}
