#pragma once
/*
  Copyright (c) 2012,2024 Matthew H. Reilly (kb1vc)
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


#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/un.h>
#include <unistd.h>

#include <string>
#include <iostream>
#include <memory>

namespace SoDa {
  namespace UD {  // Unix Domain sockets.

    class NetSocket;
    typedef std::shared_ptr<NetSocket> NetSocketPtr;     
    class ServerSocket;
    typedef std::shared_ptr<ServerSocket> ServerSocketPtr; 
    class ClientSocket;
    typedef std::shared_ptr<ClientSocket> ClientSocketPtr; 
    
    class NetSocket {
    public:
      NetSocket() {
	timeout.tv_sec = 0;
	timeout.tv_usec = 5; 
      }

    
      int put(const void * ptr, unsigned int size, bool len_prefix = true);
      int get(void * ptr, unsigned int size, bool len_prefix = true);
    
      int server_socket, conn_socket, portnum;
      struct sockaddr_un server_address, client_address;

      struct timeval timeout;
    private:
      int loopWrite(int fd, const void * ptr, unsigned int nbytes);
    };

    class ServerSocket : public NetSocket {
    public:
      ServerSocket(const std::string & path);
      ~ServerSocket() {
	close(conn_socket);
	close(server_socket);
	unlink(mailbox_pathname.c_str()); 
	std::cerr << "Closing server socket [" << mailbox_pathname << "]\n";
      }
      bool isReady();

      int get(void *ptr, unsigned int size, bool len_prefix = true) {
	int rv = NetSocket::get(ptr, size, len_prefix);
	if(rv < 0) ready = false;
	return rv; 
      }
      int put(const void *ptr, unsigned int size, bool len_prefix = true) {
	if(!ready && !isReady()) {
	  return 0; 
	}
	int rv = NetSocket::put(ptr, size, len_prefix);
	if(rv < 0) ready = false;
	return rv; 
      }
      void setDebug(bool v) {
	debug = v;
      }

      static ServerSocketPtr make(const std::string & path) {
	return std::make_shared<ServerSocket>(path);
      }
      
    private:
      bool debug; 
      bool ready;
      std::string mailbox_pathname; 
    };

    class ClientSocket : public NetSocket {
    public:
      ClientSocket(const std::string & path, int startup_timeout_count = 1);
      ~ClientSocket() { 
	close(conn_socket); 
      }

      static ClientSocketPtr make(const std::string & path) {
	return std::make_shared<ClientSocket>(path);
      }
      
    private:
      struct hostent * server; 
      std::string mailbox_pathname; 
    };
  }
}

