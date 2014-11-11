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
#include "MultiMBox.hxx"
#include "Command.hxx"
#include "Debug.hxx"

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>

namespace SoDa {
  namespace IP {
    class NetSocket {
    public:
      NetSocket() {
	timeout.tv_sec = 0;
	timeout.tv_usec = 5; 
      }

      /** PUT a message into the socket where the SIZE
       * of the message in an unsigned int at the very
       * start of each packet.
       *
       * @param ptr pointer to a buffer
       * @param size length of buffer in bytes
       */
      int put(const void * ptr, unsigned int size);

      /** Get a message from the socket where the SIZE
       * of the message in an unsigned int at the very
       * start of each packet.
       *
       * @param ptr pointer to a buffer
       * @param size not-to-exceed length of buffer in bytes
       */
      int get(void * ptr, unsigned int size);
    
      /**
       * Write raw data to the socket, return the number
       * of bytes written.
       *
       * @param ptr pointer to a buffer
       * @param size length of buffer in bytes
       */
      int writeBuf(const void * ptr, unsigned int size);

      /**
       * Read raw data from the socket, return the number
       * of bytes read.
       *
       * @param ptr pointer to a buffer
       * @param size not-to-exceed length of buffer in bytes
       */
      int readBuf(void * ptr, unsigned int size);
    
      int server_socket, conn_socket, portnum;
      struct sockaddr_in server_address, client_address;

      struct timeval timeout; 
    private:
      int loopWrite(int fd, const void * ptr, unsigned int nbytes);
    };

    class ServerSocket : public NetSocket, public Debug {
    public:
      ServerSocket(int portnum, bool localhost_only=false);
      ~ServerSocket() { close(conn_socket);  close(server_socket); }

      /**
       * is this socket active?
       * @return true if a client has connected to the socket
       */
      bool isReady();

      /** Get a message from the socket where the SIZE
       * of the message in an unsigned int at the very
       * start of each packet.
       *
       * @param ptr pointer to a buffer
       * @param size not-to-exceed length of buffer in bytes
       */
      int get(void *ptr, unsigned int size) {
	int rv = NetSocket::get(ptr, size);
	if(rv < 0) ready = false;
	return rv; 
      }
      
      /** PUT a message into the socket where the SIZE
       * of the message in an unsigned int at the very
       * start of each packet.
       *
       * @param ptr pointer to a buffer
       * @param size length of buffer in bytes
       */
      int put(const void *ptr, unsigned int size) {
	if(!ready) return 0; 
	int rv = NetSocket::put(ptr, size);
	if(rv < 0) ready = false;
	return rv; 
      }

      /**
       * Read raw data from the socket, return the number
       * of bytes read.
       *
       * @param ptr pointer to a buffer
       * @param size not-to-exceed length of buffer in bytes
       */
      int readBuf(void *ptr, unsigned int size)
      {
	if(!ready) return 0;
	int rv = NetSocket::readBuf(ptr, size);
	if(rv < 0) ready = false;
	return rv; 
      }
      
      /**
       * Write raw data to the socket, return the number
       * of bytes written.
       *
       * @param ptr pointer to a buffer
       * @param size length of buffer in bytes
       */
      int writeBuf(void *ptr, unsigned int size)
      {
	if(!ready) return 0;
	int rv = NetSocket::writeBuf(ptr, size);
	if(rv < 0) ready = false;
	return rv; 
      }
      
    private:
      bool ready; 
    };

    class ClientSocket : public NetSocket, public Debug {
    public:
      ClientSocket(const char * hostname, int portnum);
      ~ClientSocket() { close(conn_socket); }
    private:
      struct hostent * server; 
    };
  }
}


#endif
