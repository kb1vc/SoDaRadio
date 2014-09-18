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

#include "TSIP.hxx"
#include <stdio.h>
#include <boost/format.hpp>
#include <boost/asio.hpp>
#include <math.h>
#include <stdexcept>

// TODO:
//   1. Force a self survey on startup -- 8E-45 and 8E-A6/A9
// Send 8E-45 with seg num (uint8) == 7?  or 8?
// Send 8E-A6 ssc (uint8) == 0
//   it will respond with 8F-A6
// First, try with just 8E-A6 command

namespace bs = boost::asio; 

using namespace TSIP;

Reader::Reader(const std::string & devname,
	       std::list<Report *> & _replist)
{
  // remember our list of reports
  replist = _replist;

  try {
    // open the device.
    tbolt_port = new bs::serial_port(ioser, devname);
  
    // set the baud rate and such
    tbolt_port->set_option(bs::serial_port_base::baud_rate(9600));
    tbolt_port->set_option(bs::serial_port_base::parity(bs::serial_port_base::parity::none)); // none
    tbolt_port->set_option(bs::serial_port_base::flow_control(bs::serial_port_base::flow_control::none)); // none
    tbolt_port->set_option(bs::serial_port_base::character_size(8));
    tbolt_port->set_option(bs::serial_port_base::stop_bits(bs::serial_port_base::stop_bits::one));

    parse_state = INIT_WAIT_FOR_NORMCHAR;

    // tell it to do an initial survey
    std::cerr << "About to init survey" << std::endl;
    initializeSurvey(); 
    std::cerr << "started survey" << std::endl;
    reader_open = true; 
  }
  catch ( const std::exception & e) {
    std::cerr << " Problem opening GPS device: "
	      << devname << " "
	      << e.what() << " GPS is now disabled." << std::endl; 
    reader_open = false; 
  }
}

void Reader::initializeSurvey()
{
  if(!reader_open) return;
#if 0  
  // Tell the unit to forget its stored location
  RevertPositionCommand rposc;
  rposc.sendCommand(tbolt_port);
  
  // Send an initiate self survey command
  // look for an 8F-A6 response.
  IssueSelfSurveyCommand issc;
  issc.sendCommand(tbolt_port); 
#endif
}

void Command::sendCommand(boost::asio::serial_port * portp)
{
  char buf[Reader::BUFSIZE];

  // first send a DLE
  unsigned char c = Reader::DLE; 
  bs::write(*portp, bs::buffer(&c,1));

  // iterate through the buffer... looking for byte stuffing
  unsigned int idx = 0; 
  while(getByte(c, idx)) {
    if(c == Reader::DLE) {
      // if the character is a DLE, stuff an extra DLE before it....
      bs::write(*portp, bs::buffer(&c,1)); 
    }
    bs::write(*portp, bs::buffer(&c,1));
    idx++; 
  }
  
  // last send a DLE ETX
  c = Reader::DLE; 
  bs::write(*portp, bs::buffer(&c,1)); 
  c = Reader::ETX; 
  bs::write(*portp, bs::buffer(&c,1)); 
}

Report * Reader::readStream()
{
  if(!reader_open) return NULL; 

  int bufidx = 0;
  bool got_packet = false; 
  while(!got_packet) {
    unsigned char c; 
    size_t len = bs::read(*tbolt_port,
			  bs::buffer(&c, 1)); 
    if(len == 1) {
      switch (parse_state) {
      case INIT_WAIT_FOR_NORMCHAR:
	if ((c != DLE) && (c != ETX)) {
	  parse_state = INIT_WAIT_FOR_DLE; 
	}
	break;
      case INIT_WAIT_FOR_DLE:
	if (c == DLE) {
	  parse_state = INIT_WAIT_FOR_ETX; 
	}
	break;
      case INIT_WAIT_FOR_ETX:
	if (c != ETX) {
	  parse_state = INIT_WAIT_FOR_DLE; 
	}
	else {
	  parse_state = WAIT_DLE; 
	}
	break;

      case WAIT_DLE:
	if(c == DLE) {
	  parse_state = IN_PACKET_SAW_DLE; 
	}
	
	break;
      case IN_PACKET:
	if(c == DLE) {
	  parse_state = IN_PACKET_SAW_DLE; 	  
	}
	else {
	  inbuf[bufidx++] = c;	  
	}
	break; 
      case IN_PACKET_SAW_DLE:
	if(c == ETX) {
	  got_packet = true;
	  parse_state = WAIT_DLE; 
	}
	else {
	  inbuf[bufidx++] = c;
	  parse_state = IN_PACKET; 
	}
	break; 
      }
    }
  }

  // Now go through each report and see if it parses
  for(std::list<Report*>::iterator ip = replist.begin();
      ip != replist.end();
      ++ip) {
    // if the report parses, return a pointer to it.
    if((*ip)->parseBuf(inbuf, bufidx)) return (*ip); 
  }

  buflen = bufidx;
  
  return NULL; 
}

bool PrimaryTimingReport::parseBuf(unsigned char * buf, int buflen)
{
  // check the buffer. The first char must be 0xAB
  if((buf[0] != 0x8F) || (buf[1] != 0xAB)) return false;

  uint8_t * ubuf = (uint8_t*) buf;
  
  // sanity check
  if((ubuf[11] > 60) ||
     (ubuf[12] > 60) ||
     (ubuf[13] > 24)) return false;
  
  Subcode = ubuf[1];
  TimeOfWeek = convertFromBuf<uint32_t>(buf, 2);
  WeekNumber = convertFromBuf<uint16_t>(buf, 6);
  UTCOffset = convertFromBuf<int16_t>(buf, 8);
  TimingFlag = ubuf[10];

  // the time
  TimeSeconds = ubuf[11];
  TimeMinutes = ubuf[12];
  TimeHours = ubuf[13];

  // the date
  DayOfMonth = ubuf[14];
  Month = ubuf[15];
  Year = convertFromBuf<uint16_t>(buf, 16); 


  return true; 
}

bool PrimaryTimingReport::updateStatus()
{
  std::cout << boost::format("Time: %02d:%02d:%02d") % ((unsigned int) TimeHours) %
    ((unsigned int) TimeMinutes) % ((unsigned int) TimeSeconds)
	    << std::endl; 
}

bool SuplementalTimingReport::parseBuf(unsigned char * buf, int buflen)
{
  // check the buffer. The first char must be 0xAB
  if((buf[0] != 0x8F) || (buf[1] != 0xAC)) return false;

  uint8_t * ubuf = (uint8_t*) buf; 
  Subcode = ubuf[1];

  // Sanity check
  double mylat, mylon; 
  mylat = convertFromBuf<double>(buf, 37) * 180.0 / M_PI; 
  mylon = convertFromBuf<double>(buf, 45) * 180.0 / M_PI;

  if((mylat < -90.0) ||
     (mylat >  90.0) ||
     (mylon < -180.1) ||
     (mylon >  180.0)) {
    return false; 
  }

  
  Latitude = mylat;
  Longitude = mylon;

  RxMode = ubuf[2];
  DisciplineMode = ubuf[3];
  SelfSurveyProgress = ubuf[4];
  HoldoverDuration = convertFromBuf<uint32_t>(buf, 5);

  CriticalAlarms = convertFromBuf<uint16_t>(buf, 9);
  MinorAlarms = convertFromBuf<uint16_t>(buf, 11);

  GPSDecodeStatus = ubuf[13];
  DisciplineActivity = ubuf[14]; 

  PPSOffset = convertFromBuf<float>(buf, 17);
  Ref10MHzOffset = convertFromBuf<float>(buf, 21);

  DACValue = convertFromBuf<uint32_t>(buf, 25);
  DACVoltage = convertFromBuf<float>(buf, 29);

  Temperature = convertFromBuf<float>(buf, 33);
  Altitude = convertFromBuf<double>(buf, 53);
  
  return true; 
}

bool SuplementalTimingReport::updateStatus()
{
  std::cout << boost::format("Temperature: %5f  Altitude %lf Lat = %f  Lon = %lf GPDdecode = %02x") % Temperature % Altitude % Latitude % Longitude % ((unsigned int) GPSDecodeStatus)
	    << std::endl; 
}
