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

#ifndef TSIP_HDR
#define TSIP_HDR

#include <string>
#include <list>
#include <boost/asio/serial_port.hpp>
#include <boost/format.hpp>
#include <iostream>

namespace TSIP {

  template<typename T> T convertFromBuf(unsigned char * buf, int pos) {
    T ret;
    int i;
    int Nbytes = sizeof(T); 
    unsigned char * revp = ((unsigned char*) &ret);
    for(i = Nbytes - 1; i >= 0; i--) {
      revp[i] = buf[pos++]; 
    }
    return ret;
  }
  // All report formats are as described in the 
  // Trimble Standard Interface Protocol for Thunderbolt
  // from the ThunderBolt GPS Disciplined Clock User Guide Version 5.0
  // Trimble part number 35326-30 November 2003
  //
  class Report {
  public:
    Report() {
    }

    virtual bool parseBuf(unsigned char * buf, int buflen) = 0;

    virtual bool updateStatus() = 0;
  };

  class Command {
  public:
    Command() {
    }

    // return 0 if idx is beyond the end of this command's buffer. 
    virtual unsigned int getByte(unsigned char & c, unsigned int idx) = 0;

    void sendCommand(boost::asio::serial_port * port);
  }; 
  

  class Reader {
  public:
    Reader(const std::string & devname, std::list<Report *> & _replist);

    // loop until we find a packet that can be processed
    // then return that packet. 
    Report * readStream(); 

    
    static const int BUFSIZE = 1024;
    static const unsigned char DLE = 0x10;
    static const unsigned char ETX = 0x03; 

    void dumpBuffer(std::ostream & os) {
      
      for(unsigned int i = 0; i < buflen; i++) {
	unsigned int v = inbuf[i]; 
	if((i & 0xf) == 0) {
	  os << boost::format("\n%02x  ") % inbuf; 
	}
	os << boost::format(" %02x") % v; 
      }
      os << std::endl; 
    }
  private:

    bool reader_open; // true if we were able to create a device...

    void initializeSurvey(); 
    unsigned char inbuf[1024];
    unsigned int buflen; 

    enum PS { INIT_WAIT_FOR_NORMCHAR, INIT_WAIT_FOR_DLE, INIT_WAIT_FOR_ETX, WAIT_DLE, IN_PACKET_SAW_DLE, IN_PACKET };
    PS parse_state;

    std::list<Report *> replist;
    
    boost::asio::io_service ioser;
    boost::asio::serial_port * tbolt_port; 
  };

  class IssueSelfSurveyCommand : public Command {
  public:
    IssueSelfSurveyCommand() : Command() {
    }

    unsigned int getByte(unsigned char & c, unsigned int idx) {
      switch(idx) {
      case 0:
	c = 0x8E;
	return 1;
	break; 
      case 1:
	c = 0xA6;
	return 1;
	break;
      case 2:
	c = 0x0;
	return 1;
	break;
      default:
	return 0;
	break; 
      }
    }
  }; 

  class RevertPositionCommand : public Command {
  public:
    RevertPositionCommand() : Command() {
    }

    unsigned int getByte(unsigned char & c, unsigned int idx) {
      switch(idx) {
      case 0:
	c = 0x8E;
	return 1;
	break; 
      case 1:
	c = 0x45;
	return 1;
	break;
      case 2:
	c = 0x07;
	return 1;
	break;
      default:
	return 0;
	break; 
      }
    }
  }; 

  class PrimaryTimingReport : public Report {
  public:
    PrimaryTimingReport() : Report() {
    }

    bool parseBuf(unsigned char * buf, int buflen); 

    virtual bool updateStatus();
  protected:
    uint8_t    Subcode;     // 0xAB for this report (major code is 0x8F)
    uint32_t   TimeOfWeek; // GPS seconds of week
    uint16_t   WeekNumber; // GPS
    int16_t    UTCOffset;  // seconds difference between UTC and GPS time
    uint8_t    TimingFlag; // <0> GPS(0)/UTC(1) time,  <1> GPS/UTC PPS,  <2> time set(0)/notset(1), <3> noUTCinfo(1), <4> time from user
    uint8_t    TimeSeconds;
    uint8_t    TimeMinutes;
    uint8_t    TimeHours; 
    uint8_t    DayOfMonth;
    uint8_t    Month; 
    uint16_t   Year; 
  };

  class SuplementalTimingReport : public Report {
  public:
    SuplementalTimingReport() : Report() {
    }
  
    bool parseBuf(unsigned char * buf, int buflen);
  
    virtual bool updateStatus();
    
  protected:
    uint8_t    Subcode;     // 0xAC for this report (major code is 0x8F)
    uint8_t    RxMode;
    uint8_t    DisciplineMode;
    uint8_t    SelfSurveyProgress;
    uint32_t   HoldoverDuration; // in seconds
    uint16_t   CriticalAlarms;
    uint16_t   MinorAlarms;
    uint8_t    GPSDecodeStatus;
    uint8_t    DisciplineActivity;
    float      PPSOffset; // estimate UTC/GPS offset in nS
    float      Ref10MHzOffset;  // estimate UTC/GPS offset in parts per billion
    uint32_t   DACValue;
    float      DACVoltage;
    float      Temperature;
    double     Latitude; // in degrees
    double     Longitude;
    double     Altitude;     // in meters
    uint64_t   EndSpare; 
  }; 

}
#endif
