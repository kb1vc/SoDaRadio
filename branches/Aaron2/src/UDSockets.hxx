/*
  Copyright (c) 2010,2014,Matthew H. Reilly (kb1vc)
  Copyright (c) 2014, Aaron Yankey Antwi (aaronyan2001@gmail.com)

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

#ifndef UDSOCKETS_HDR
#define UDSOCKETS_HDR
#include "SoDaBase.hxx"
#include "MultiMBox.hxx"
#include "Command.hxx"
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/un.h>


namespace SoDa {
  namespace UD 	{  // Unix Domain sockets. 
    class NetSocket {
    public:
      NetSocket() {
	timeout.tv_sec = 0;
	timeout.tv_usec = 5; 
      }
    
      int put(const void * ptr, unsigned int size);
      int get(void * ptr, unsigned int size);
    
      int server_socket, conn_socket, portnum,track_socket,track_msgsock,track_rval;
      struct sockaddr_un server_address, client_address,track_server,track_client;
      
      struct timeval timeout; 
    private:
      int loopWrite(int fd, const void * ptr, unsigned int nbytes);
    };


    class ClientSocket : public NetSocket {
    public:
      ClientSocket(const std::string & path, int startup_timeout_count = 1);
      ~ClientSocket() { close(conn_socket); }
    private:
      struct hostent * server; 

    };
    
    //This object is added to enable tuning for the radio via gpredict
    class TrackerSocket : public NetSocket {
	    public: 
		    TrackerSocket(const std::string & path);
		    ~TrackerSocket() {
		close(track_socket);
		unlink(track_path.c_str());//track as in another name for path
		delete rc;
		    }
		    //Structure to pass to the tuner
typedef struct _RadioCommand{//This is object saves the state of the tracker and the radio
int vfo;
bool fromDisplay;
static double rxfreq;
static double txfreq;
static double fromDisplayFreq;
}RadioCommand;
RadioCommand *rc;
char track_buf[20];
 RadioCommand* getTracker();
	int setTracker(const void * response);
	RadioCommand *getParser(std::string cmd);
	 bool tReady;
bool isReady();
    private:
	 std::string track_path;
	 socklen_t track_client_addr_size;
	void Debugger(std::string msg);
	std::string dbg;
double freq;
static int rxCounter;
static int respCounter;
std::string cmds [2];
int cmdSize;
int inputCmdSize;
std::size_t newLine;

    	};



    
    class ServerSocket : public NetSocket {
    public:
      ServerSocket(const std::string & path);
      ~ServerSocket() {
	close(conn_socket);
	close(server_socket);
	unlink(mailbox_pathname.c_str()); 
      }
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
      std::string mailbox_pathname; 
    };





    
  }
}


#endif
