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
#include <SoDa/Format.hxx>

#include <stdexcept>

namespace SoDa {

  namespace IP {

    /**
     * This is an IP socket that returns newline-delimited
     * strings arriving on the socket.  LineServerSocket builds
     * on the IP::ServerSocket class. 
     */
    class LineServerSocket : public ServerSocket {
    public:
      /**
       * @brief constructor
       * 
       * @param portnum the port number for this server
       * @param transport one of IP::UDP or IP::TCP 
       */ 
      LineServerSocket(int portnum, 
		       TransportType transport = TCP) :
	ServerSocket(portnum, transport) {
	ready_line_count = 0; 

	// we want to use the nonblocking read.
	setNonBlocking();
      }

      /**
       * This is the size of the running buffer that we read into
       * from the socket.  This is NOT the maximum length of an 
       * unterminated string, as characters are read from the temp_buf
       * and then pushed onto a queue. 
       */
      static const unsigned int temp_buf_size = 128;
      
      /**
       * @brief capture a newline-terminated buffer from the socket
       * This is a non-blocking method. 
       *
       * @param buf pointer to a buffer to receive the string 
       * -- newline is removed.
       * @param maxsize maximum allowed length of string. 
       * @return 0 if no newline terminated string is available, 
       *         -1 if the client has disconnected, 
       *         and length of the buffer, otherwise. 
       */
      int getLine(char * buf, unsigned int maxsize) {
	if(ready_line_count == 0) {
	  // get a buffer; 
	  int gotbytes = read(conn_socket, temp_buf, temp_buf_size);
	  std::cerr << SoDa::Format("READ Got ret = %0 errno = %1 ready = %2\n")
	    .addI(gotbytes)
	    .addI(errno)
	    .addC((char) (isReady() ? 'T' : 'F')); 
	  if(gotbytes < 0) {
	    if((errno != EWOULDBLOCK) && (errno != EAGAIN)) {
	      std::cerr << SoDa::Format("READ got ret = %0  errno = %1\n")
		.addI(gotbytes)
		.addI(errno); 
	      perror("READ: ");
	      return -1; 
	    }
	  }
	  if(gotbytes == 0) {
	    // a nonblocking read that returns 0 is directed at 
	    // a closed socket.  An open socket with nothing in it
	    // will return -1 with EWOULDBLOCK.  So, if we get here, 
	    // the remote connection has been closed.
	    close(conn_socket);
	    ready = false; 
	    return -1; 
	  }
	  if(gotbytes > 0) {
	    for(int i = 0; i < gotbytes; i++) {
	      inbuf.push(temp_buf[i]); 
	      if(temp_buf[i] == '\n') ready_line_count++; 
	    }
	  }
	}

	if(ready_line_count > 0) {
	  // scan the buffer until we get to the newline. 
	  ready_line_count--;
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


    protected:
      /**
       * Do this the simple way -- accumulate characters in a queue
       * then return each "\n" terminated substring as we find it.
       */
      std::queue<char> inbuf; 

      /**
       * keep a count of the number of "\n" delimited strings
       * we've seen that haven't yet been returned from getLine()
       */
      int ready_line_count; 

      /**
       * This is the running buffer that we read into
       * from the socket.  This is NOT the maximum length of an 
       * unterminated string, as characters are read from the temp_buf
       * and then pushed onto a queue. 
       */
      char temp_buf[temp_buf_size]; 
    }; 
  }
}


#endif
