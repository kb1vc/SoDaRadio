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

#ifndef IPSOCKETS_HDR
#define IPSOCKETS_HDR
#include "SoDaBase.hxx"
#include "Command.hxx"

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

#include <stdexcept>

namespace SoDa {
  namespace IP {

    class ReadTimeoutExc : public std::runtime_error {
    public:
      ReadTimeoutExc(std::string sockname) : 
	std::runtime_error((boost::format("Read operation on %s socket timed out") % sockname).str()) {
      }
    }; 

    enum TransportType { UDP, TCP }; 

    class NetSocket {
    public:
      NetSocket() {
	timeout.tv_sec = 0;
	timeout.tv_usec = 5; 
      }

    
      int put(const void * ptr, unsigned int size);
      int get(void * ptr, unsigned int size);

      int putRaw(const void * ptr, unsigned int size);
      
      /**
       * @brief get a raw string of <size> bytes... 
       *
       * @param ptr pointer to a buffer
       * @param size don't return until we've gotten <size> bytes, or timed out
       * @param usec_timeout return FALSE if we wait longer than this, 0 says don't timeout
       * @return 0 if we got all <size> bytes, -1 if we timed out. 
       */
      int getRaw(const void * ptr, unsigned int size, unsigned int usec_timeout = 0);       
    
      /** 
       *
       */
      void setNonBlocking() {
	int x = fcntl(conn_socket, F_GETFL, 0);
	fcntl(conn_socket, F_SETFL, (x  | O_NONBLOCK));
	non_blocking_mode = true; 
      }

      void setBlocking() {
	int x = fcntl(conn_socket, F_GETFL, 0);
	fcntl(conn_socket, F_SETFL, (x & ~O_NONBLOCK));
	non_blocking_mode = false;
      }

      int server_socket, conn_socket, portnum;
      struct sockaddr_in server_address, client_address;
      bool non_blocking_mode; 

      struct timeval timeout; 
    private:
      int loopWrite(int fd, const void * ptr, unsigned int nbytes);
    };

    class ServerSocket : public NetSocket {
    public:
      ServerSocket(int portnum, TransportType transport = TCP);
      ~ServerSocket() { close(conn_socket);  close(server_socket); }
      bool isReady();

      int get(void *ptr, unsigned int size) {
	int rv = NetSocket::get(ptr, size);
	if(rv < 0) ready = false;
	return rv; 
      }
      int put(const void *ptr, unsigned int size) {
	if(!ready) return 0; 
	int rv = NetSocket::put(ptr, size);
	if(rv < 0) ready = false;
	return rv; 
      }
    private:
      bool ready; 
    };

    class ClientSocket : public NetSocket {
    public:
      ClientSocket(const char * hostname, int portnum, TransportType transport = TCP);
      ~ClientSocket() { close(conn_socket); }
    private:
      struct hostent * server; 
    };
  }
}


#endif
