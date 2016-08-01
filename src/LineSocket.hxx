/*
  Copyright (c) 2016, Matthew H. Reilly (kb1vc)
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

#ifndef LINESOCKET_HDR
#define LINESOCKET_HDR
#include "SoDaBase.hxx"
#include "IPSockets.hxx"
#include "UDSockets.hxx"
#include "Command.hxx"


#include <stdexcept>

namespace SoDa {

  namespace IP {

    class LineServerSocket : public ServerSocket {
    public:
      LineServerSocket(int portnum, 
		       TransportType transport = TCP) :
	ServerSocket(portnum, transport) {
	ready_lines = 0; 
	empty_count = 0; 

	// we want to use the nonblocking read.
	setNonBlocking();
      }

      static const unsigned int temp_buf_size = 128; 
      /**
       * @brief capture a newline-terminated buffer from the socket
       * This is a non-blocking method. 
       *
       * @param buf pointer to a buffer to receive the string 
       * -- newline is removed.
       * @param maxsize maximum allowed length of string. 
       * @return 0 if no newline terminated string is available, 
       *  length of the buffer, otherwise. 
       */
      int getLine(char * buf, unsigned int maxsize) {
	if(ready_lines == 0) {
	  // get a buffer; 
	  int gotbytes = read(conn_socket, temp_buf, temp_buf_size);
	  std::cerr << boost::format("Got ret = %d errno = %d ready = %c\n")
	     % gotbytes % errno % ((char) (isReady() ? 'T' : 'F')); 
	  if(gotbytes < 0) {
	    if((errno != EWOULDBLOCK) && (errno != EAGAIN)) return -1; 
	  }
	  if(gotbytes == 0) {
	    empty_count++; 
	    if(empty_count == 10) {
	      empty_count = 0; 
	      return checkForClosed(); 
	    }
	  }
	  if(gotbytes > 0) {
	    for(int i = 0; i < gotbytes; i++) {
	      inbuf.push(temp_buf[i]); 
	      if(temp_buf[i] == '\n') ready_lines++; 
	    }
	  }
	}

	if(ready_lines > 0) {
	  // scan the buffer until we get to the newline. 
	  ready_lines--;
	  char ch; 
	  for(unsigned int i = 0; i < maxsize; i++) {
	    ch = inbuf.front(); inbuf.pop();
	    if(ch == '\n') {
	      buf[i] = '\000';
	      return i; 
	    }
	    else {
	      buf[i] = ch; 
	    }
	  }
	  // if we get here, we ran out of buffer space. 
	  buf[maxsize-1] = '\000';
	  // consume the queue up to the newline. 
	  for(ch = '\000'; (ch != '\n') && !inbuf.empty(); ) {
	    ch = inbuf.front(); inbuf.pop();
	  }
	  return maxsize; 
	}
	else {
	  return 0; 
	}
      }


      /**
       * @brief check to see if this socket has been closed by 
       * the client end. 
       * @return 0 for open, -1 for connection closed.
       */
      int checkForClosed() {
	int stat; 
	char b; 
	stat = write(conn_socket, &b, 0);
	std::cout << boost::format("zero write got stat = %d errno = %d\n") % stat % errno; 
	return stat; 
      }

    protected:
      // Do this the simple way -- accumulate characters in a buffer
      // then return each command as it happens. 
      std::queue<char> inbuf; 
      // keep a count of the number of "\n" we've seen. 
      int ready_lines; 

      // keep a count of the number of times we got an empty read buffer.
      int empty_count; 
      // we need a cheap input buffer...
      char temp_buf[temp_buf_size]; 
    }; 
  }
}


#endif
