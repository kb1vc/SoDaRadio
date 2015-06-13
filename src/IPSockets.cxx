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
#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>

SoDa::IP::ServerSocket::ServerSocket(int portnum, TransportType transport)
{
  int stat; 
  // create the socket. 
  if(transport == TCP) {
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
  } 
  else if (transport == UDP) {
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
  }
    
  if(server_socket < 0) {
    std::cerr << "Failed to create server socket... I quit." << std::endl;
    exit(-1); 
  }

  int x = fcntl(server_socket, F_GETFL, 0);
  fcntl(server_socket, F_SETFL, x | O_NONBLOCK);
  
  // setup the server address
  bzero((char*) &server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
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

SoDa::IP::ClientSocket::ClientSocket(const char * hostname, int portnum, TransportType transport)
{
  if(transport == TCP) {
    conn_socket = socket(AF_INET, SOCK_STREAM, 0);
  }
  else if(transport == UDP) {
    conn_socket = socket(AF_INET, SOCK_DGRAM, 0);
  }

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

  setNonBlocking(); 
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




int SoDa::IP::NetSocket::putRaw(const void * ptr, unsigned int size)
{
  int stat = loopWrite(conn_socket, ptr, size);
  return stat; 
}

int SoDa::IP::NetSocket::getRaw(const void * ptr, unsigned int size, unsigned int usec_timeout)
{
  int stat;

  if(usec_timeout != 0) {
    if(non_blocking_mode) setBlocking(); 
    struct timeval tv; 
    tv.tv_sec = 0; 
    tv.tv_usec = usec_timeout; 
    int stat = setsockopt(conn_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)); 
    if (stat < 0) {
      throw std::runtime_error("Failed to put client socket in timeout mode\n");
    }
  }
  else {
    if(!non_blocking_mode) setNonBlocking(); 
  }

  int got = 0;
  int left = size;
  char * bptr = (char*) ptr; 
  int timeout_count = 0; 
  while(left > 0) {
    int ls;
    // ls = read(conn_socket, bptr, left);
    ls = recv(conn_socket, bptr, left, 0);
    if(ls < 0) {
      if((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
        timeout_count++; 
	if(timeout_count > 2) {
	  if(left == size) {
	    throw ReadTimeoutExc("Client");
	  }
	  else return size - (left + ls); 
	}
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
  
  return size; 

}
