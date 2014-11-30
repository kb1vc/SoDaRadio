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

#include "UDSockets.hxx"

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
#include <string.h>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

/**************************************Tracker*******************************/
SoDa::UD::TrackerSocket::TrackerSocket(const std::string &path)
{
int  stat = -1;
track_path = path;
//Create the socket
track_socket = socket(AF_UNIX,SOCK_STREAM,0);
  if(track_socket < 0) {
    std::cerr << std::endl << "Error:  I failed to create tracker socket... I quit." ;
    exit(1); 
  }else {
	  //Prepare the socket for io
  int x = fcntl(track_socket, F_GETFL, 0);
  fcntl(track_socket, F_SETFL, x | O_NONBLOCK);
  //clear the track server
  bzero((char*) &track_server, sizeof(track_server));
  //Initialize the socket
track_server.sun_family = AF_UNIX;
//strcpy(track_server.sun_path,"/tmp/SoDa_tracker");
 strncpy(track_server.sun_path, path.c_str(), sizeof(track_server.sun_path));
//Delete the file if its already exists
  if(!unlink(track_server.sun_path))
  std::cerr << std::endl << "Deleted existing copy of server" ;
  //Bind to the socket
if(bind(track_socket,(struct sockaddr *) &track_server, sizeof(struct sockaddr_un))){
	std::cerr << std::endl << "Error: Binding stream socket ["<< errno << "]"<<std::endl;
    if((errno == EWOULDBLOCK) || (errno == EAGAIN)) 
    {
	    close(track_msgsock);
	    close(track_socket);
	exit(1);
    }
}
else{
//OnBind()
std::cerr << std::endl << "Bound to server: " << track_server.sun_path ;
stat=listen(track_socket,5);
  if(stat < 0) {
    std::cerr << std::endl << "Error:  I couldn't listen on Unix socket  " << path << " got " << errno << " I quit." ; 
    exit(-1); 
  }else std::cerr << std::endl << "Success: I am now listening on : " << path ;
}
tReady =false;
rc = new RadioCommand();
}
}


bool SoDa::UD::TrackerSocket::isReady()
{
  if(tReady) return true;
  else {
	track_client_addr_size = sizeof(track_server);
track_msgsock = accept(track_socket,(struct sockaddr *) &track_server,&track_client_addr_size);
if(track_msgsock < 0)
{	
perror("Error: Accept stream message:");
	std::cerr << errno <<std::endl; 
	tReady =false;
std::cerr << std::endl << "Please run this command in terminal first: 'socat TCP-LISTEN:4532 UNIX-CONNECT:/tmp/SoDa_tracker &'" <<std::endl <<"Then engage me from gpredict" <<std::endl;
}
    else {
	//Prepare the socket for reading
      int x = fcntl(track_msgsock, F_GETFL, 0);
      fcntl(track_msgsock,F_SETFL, x | O_NONBLOCK);
      tReady = true; 
std::cerr << std::endl << "CONNECTED" ;
    }
  }
  return tReady; 
}




int SoDa::UD::TrackerSocket::rxCounter=0;
SoDa::UD::TrackerSocket::RadioCommand * SoDa::UD::TrackerSocket::getTracker()
{
//return string
std::string ret;
//Clear the track_buf 
bzero(track_buf, sizeof(track_buf));
//Read messages
		do {
	//Start reading
	if((track_rval = read(track_msgsock,&track_buf,sizeof(track_buf))) < 0);
	else if(track_rval == 0)
	{
std::cerr << std::endl << "DISCONNECTED" ;
                exit(-1);
	}
		else
			if(track_rval > 0)//Display only when there is data
		{
	//Debugger("********************************************END (" + (int)rxCounter + ") **************************************************");
	Debugger(dbg);
	//Debugger("-> "+track_buf);
	//Debugger("********************************************END (" + (int)rxCounter + ") **************************************************");
rxCounter++;//Increament counter
}
		//debug
	//std::cerr << std::endl << "//Debugger: Reading" ;
	}while (track_rval > 0);
		
//std::cerr << std::endl << "RETURNING COMMAND: " << ret;
	//save the buff
 ret.append(track_buf);
this->getParser(ret);
return rc;
}




void SoDa::UD::TrackerSocket::Debugger(std::string msg)
{

if(0){
std::cerr << std::endl <<  msg;
}
}

//Init counter
int SoDa::UD::TrackerSocket::respCounter=0;
//used to reply gpredict
int SoDa::UD::TrackerSocket::setTracker(const void * response)
{
int stat=0;
char* resp = (char *)response;

		//Prepare the socket for reading
      int x = fcntl(track_msgsock, F_GETFL, 0);
      fcntl(track_msgsock,F_SETFL, x | O_NONBLOCK);

	if(track_msgsock > 0)
	{
stat = write(track_msgsock,response,sizeof(response)+1);

if(stat < 0)
{}else//Tell how many bits have been written
{//std::cerr << std::endl  <<"(" << respCounter++ << ") RESPONSE: " << (char *)response << "[" << stat << "]" <<std::endl;
}
}else {
	perror("I can not write to 'track_msgsock'");
	exit(1);
}

return stat;
}



double SoDa::UD::TrackerSocket::_RadioCommand::rxfreq =145000000;
double SoDa::UD::TrackerSocket::_RadioCommand::txfreq = 145000000;
double SoDa::UD::TrackerSocket::_RadioCommand::fromDisplayFreq =145000000;
SoDa::UD::TrackerSocket::RadioCommand * SoDa::UD::TrackerSocket::getParser(std::string command)
{
	//Command extraction
cmdSize=2;
inputCmdSize=command.size();
newLine = command.find_first_of('\n');
if(newLine > inputCmdSize)//You just need this
	newLine = inputCmdSize;

cmds[0] = command.substr(0,newLine);
if(newLine < inputCmdSize)
cmds[1] = command.substr(newLine);//Remove new line

//Debug

//std::cerr << std::endl << "Position of NewLine: " << newLine << std::endl << "Value of cmd[1]= " << cmds[0].c_str() << " :" <<std::endl << "Value of cmd[1]= " << cmds[1].c_str() << " :" << std::endl << "Command Array size(cmds[]): " << cmdSize << std::endl << "Length of input command "  << inputCmdSize << std::endl ;
//End Command extraction


for(int j=0; j<cmdSize;j++){
	
	std::string cmd = cmds[j].c_str();//loop through command
	if(!cmd.empty()){
std::string cmdList="FfMmIiXxSsNnLlUuPpGgAaRrOoCcDdVvTtEeHhBbJjZzYyQq";
//Set response
std::string res("RPRT 0");
//Filter out the command
std::size_t found = cmdList.find_first_of(cmd[0]);
//Debug above filter
//std::cerr << std::endl << "Found command: " << found ;
//Filter the frequency
std::size_t pos = cmd.find_last_of(" ");//find the last space from the end
std::string value="",variable="";
if(pos <= cmd.size()){//sometimes the value of pos can be very large
 value = cmd.substr(pos);//get value portion
 variable = cmd.substr(0,pos);//get variable portion
}
//debug
//std::cerr << std::endl << "Postion of space: " << pos << std::endl << "Value of cmd: " << value << std::endl << "Variable of command: " << variable;
//buf for string conversion
std::string ret_rx;
std::string ret_tx;
std::stringstream toString;
//Break down the commands
switch(found)
{
	case 0://F is passed (set frequency)
	freq= (double)strtold(value.c_str(),NULL);
		if(freq < 60000000)
		{
			std::cerr << std::endl <<"freq is less than 60M : " << std::fixed<<freq;
			freq = freq * 10;
		}
		if(rc->vfo==0){//Set RX
		//Update RX Freq
			rc->rxfreq=freq;
	//Debugger("New RXFreq: "+rc->rxfreq);
	//std::cerr << std::endl << "New RXFreq: " << std::fixed<<std::setprecision(0) << rc->rxfreq;
			}
		else if(rc->vfo==1)//Set Tx
		{
			//Update TX Freq
			rc->txfreq=freq;
	//Debugger("New TXFreq: "+ rc->txfreq);
	//std::cerr << std::endl << "New TXFreq: " << std::fixed<<std::setprecision(0) << rc->txfreq; 
}
//Send RPRT 0
this->setTracker((void *)res.c_str());
		break;

	case 1://f is passed (get frequency)

	if(rc->vfo==0){//Rx 
		//if the frequency is to be set check if the display is on
		if(rc->fromDisplay)
		{
			//Set the rx freq to the frequency from display
			rc->rxfreq =rc->fromDisplayFreq;
			//reset the switch
			rc->fromDisplay = false;
			//debug
	//Debugger("Frequency from display: "+rc->fromDisplayFreq);
		//std::cerr << std::endl << "Frequency from display " << std::fixed<<rc->fromDisplayFreq << std::endl;
		}
		//report freq
		toString <<std::fixed<<std::setprecision(0)<<rc->rxfreq;
		ret_rx = toString.str();
	//Debugger("Reporting (RX) f: "+ret_rx);
		//std::cerr << std::endl << "Reporting (RX) f: " <<  ret_rx;
		this->setTracker((const void *)ret_rx.c_str());
	
	}	else {	       

		if(rc->vfo==1){//Tx
		//report freq
		toString <<std::fixed<<std::setprecision(0)<<rc->txfreq;
		ret_tx = toString.str();
	//Debugger("Reporting (TX) f: "+ret_tx);
		//std::cerr << std::endl << "Reporting (TX) f: " <<  ret_tx;
		this->setTracker((const void *)ret_tx.c_str());
	}
	}
break;
	case 30:// Select VFO
		if(cmd.compare(2,4,"VFOB")==0)
		{
	//Debugger("VFOB (RX) oscillator is selected");
			//std::cerr << std::endl <<"VFOB (RX) oscillator is selected" ;
				//Change the selector rx
			rc->vfo=0;
		}
		else
		if(cmd.compare(2,4,"VFOA")==0)
				{
	//Debugger("VFOA (TX) oscillator is selected");
		//	std::cerr << std::endl <<"VFOA (TX) oscillator is selected";
				//Change the selector tx
			rc->vfo=1;
				}
//Send RPRT 0
this->setTracker((void *)res.c_str());
		break;


	case 31://get VFO
	if(rc->vfo==0){
//Send RPRT 0
this->setTracker((void *)res.c_str());
	}else if(rc->vfo==1){
//Send RPRT 0
this->setTracker((void *)res.c_str());
	}	
	break;
	case 47://Quit
		std::cerr << std::endl << "QUITTING" <<std::endl;
		exit(1);
		break;

	default:
		;
//Send RPRT 0
//this->setTracker((void *)res.c_str());
}//end switch
cmds[j].clear();
cmd.clear();
ret_rx.clear();
ret_tx.clear();
}//end if
}//end for
//slow down
//usleep(500000);
return rc;
}
/**************************************Tracker*******************************/





SoDa::UD::ServerSocket::ServerSocket(const std::string & path)
{
  int stat; 
  // create the socket. 
  server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    
  if(server_socket < 0) {
    std::cerr << std::endl << "Failed to create server socket... I quit." ;
    exit(-1); 
  }

  int x = fcntl(server_socket, F_GETFL, 0);
  fcntl(server_socket, F_SETFL, x | O_NONBLOCK);
  
  // setup the server address
  bzero((char*) &server_address, sizeof(server_address));
  server_address.sun_family = AF_UNIX;
  strncpy(server_address.sun_path, path.c_str(), sizeof(server_address.sun_path));
  unlink(server_address.sun_path);
  int len = strlen(server_address.sun_path) + sizeof(server_address.sun_family); 

  mailbox_pathname = path; 

  // now bind it
  if (bind(server_socket, (struct sockaddr *) &server_address, len) < 0) {
    std::cerr << std::endl << "Couldn't bind Unix domain socket at path " << path << " I quit." ;
    exit(-1); 
  }

  // now let the world know that we're ready for one and only one connection.
  stat = listen(server_socket, 1);
  if(stat < 0) {
    std::cerr << std::endl << "Couldn't listen on Unix socket  " << path << " got " << errno << " I quit." ; 
    exit(-1); 
  }

  // mark the socket as "not ready" for input -- it needs to accept first. 
  ready = false; 
}

SoDa::UD::ClientSocket::ClientSocket(const std::string & path, int startup_timeout_count)
{
  int retry_count;
  conn_socket = socket(AF_UNIX, SOCK_STREAM, 0);
  if(conn_socket < 0) {
    std::cerr << std::endl << boost::format("Failed to create client socket on [%s]... I quit.\n")
			       % path; 
    exit(-1); 
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
      sleep(2);
    }
  }

  if(stat < 0) {
    std::cerr << std::endl << "Client couldn't connect to UNIX socket [" << path << "].  I quit." ;
    perror("oops: client could not connect to the socket: ClientSocket::ClientSocket()");
    exit(-1); 
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
      //      std::cerr << std::endl << ">>>" ; 
      return 0; 
    }
    else {
      perror("Oops -- socket get --: NetSocket::get() "); 
      exit(-1);
      //return stat;
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
	perror("Ooops -- read buffer continued: NetSocket::get()");
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
	  perror("Ooops -- read buffer continued: NetSocket::get()");
	  return ls;
	}
      }
      left -= ls; 
    }
  }

  return size; 

}

