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

#include "UDSockets.hxx"

#include <iostream>
#include <stdexcept>

#include <unistd.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <boost/format.hpp>

SoDa::UD::ServerSocket::ServerSocket(const std::string & path)
{
  int stat; 
  // create the socket. 
  server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    
  if(server_socket < 0) {
    throw std::runtime_error("Failed to create server socket.\n");
  }

  int x = fcntl(server_socket, F_GETFL, 0);
  fcntl(server_socket, F_SETFL, x | O_NONBLOCK);
  
  // setup the server address
  bzero((char*) &server_address, sizeof(server_address));
  server_address.sun_family = AF_UNIX;
  strncpy(server_address.sun_path, path.c_str(), sizeof(server_address.sun_path));
  unlink(server_address.sun_path);
  int len = strlen(server_address.sun_path) + sizeof(server_address.sun_family); 


  // now bind it
  //  if (bind(server_socket, (struct sockaddr *) &server_address, len) < 0) {
  if (bind(server_socket, (struct sockaddr *) &server_address, SUN_LEN(&server_address)) < 0) {
    throw std::runtime_error((boost::format("Couldn't bind Unix domain socket at path [%s] I quit.\n") % path).str());
  }

  // now let the world know that we're ready for one and only one connection.
  stat = listen(server_socket, 1);
  if(stat < 0) {
    throw std::runtime_error((boost::format("Couldn't listen on Unix socket  [%s]. Got error number %d. I quit.\n") % path % errno).str());
  }

  // mark the socket as "not ready" for input -- it needs to accept first. 
  ready = false; 
}

SoDa::UD::ClientSocket::ClientSocket(const std::string & path, int startup_timeout_count)
{
  int retry_count;
  conn_socket = socket(AF_UNIX, SOCK_STREAM, 0);
  if(conn_socket < 0) {
    throw std::runtime_error((boost::format("Failed to create client socket on [%s]... I quit.\n") % path).str());
  }

  server_address.sun_family = AF_UNIX;
  strncpy(server_address.sun_path, path.c_str(), sizeof(server_address.sun_path));
  int len = strlen(server_address.sun_path) + sizeof(server_address.sun_family); 

  int stat; 
  for(retry_count = 0; retry_count < startup_timeout_count; retry_count++) {
    stat = connect(conn_socket, (struct sockaddr *) &server_address, len);
    if(stat >= 0) break;
    else {
      // we should wait a little while before we give up.
      sleep(1);
    }
  }

  if(stat < 0) {
    throw std::runtime_error((boost::format("Couldn't connect to Unix domain socket at path [%s] I quit.\n") % path).str());
  }

  int x = fcntl(conn_socket, F_GETFL, 0);
  fcntl(conn_socket, F_SETFL, x | O_NONBLOCK);

}

bool SoDa::UD::ServerSocket::isReady()
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

int SoDa::UD::NetSocket::loopWrite(int fd, const void * ptr, unsigned int nbytes)
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

  return 0;
}
int SoDa::UD::NetSocket::put(const void * ptr, unsigned int size)
{
  // we always put a buffer of bytes, preceded by a count of bytes to be sent.
  int stat;
  
  stat = loopWrite(conn_socket, &size, sizeof(unsigned int));
  if(stat < 0) return stat; 

  stat = loopWrite(conn_socket, ptr, size);

  return stat; 
}

int SoDa::UD::NetSocket::get(void * ptr, unsigned int size)
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
