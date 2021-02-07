/*
  Copyright (c) 2015, Matthew H. Reilly (kb1vc)
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

#ifndef N200Serial_HDR
#define N200Serial_HDR

#include "TRControl.hxx"
#include "IPSockets.hxx"
#include <string>
#include <list>
#include <iostream>
#include <uhd/usrp/multi_usrp.hpp>



namespace SoDa {
  
  /**
   * @brief Transmit/Receive switch control for N200/N210 via the 
   * GPS serial or EXP serial io port.  (Default is GPS port). 
   */
  class N200Control : public TRControl {
  public:
    /**
     * @brief constructor
     * @param usrp pointer to usrp multi dev
     * @param mboard motherboard index  (0 is good)
     * @param ip_addr_str the ip address of the selected radio
     * @param portnum the port to use
     *
     */
    N200Control(uhd::usrp::multi_usrp::sptr usrp,
		int mboard, 		
		std::string ip_addr_str, 
		unsigned int portnum = 49172
		);
    /**
     * @brief activate external controls to enable transmit mode.
     * @return true if the operation completed successfully 
     */
    bool setTXOn();

    /**
     * @brief activate external controls to disable transmit mode.
     * @return true if the operation completed successfully 
     */
    bool setTXOff();

    /** 
     * @brief report state of transmit-enable. 
     * @return true iff the transmitter is currently enabled.
     */
    bool getTX();

    /**
     * @brief turn on/off signal path for selected band
     * note that multiple bands can be enabled at one time(!)
     *
     * @param band band selector (typically index from 1...7)
     * @param state true to enable band, false otherwise. 
     * @return true if band state was changed. 
     */
    bool setBand(unsigned int band, bool state);

    /**
     * @brief query state of signal path for selected band
     *
     * @param band band selector (typically index from 1...7)
     * @return true if band signal path is enabled. 
     */
    bool getBand(unsigned int band);

  private:
    bool sendCommand(const std::string & cmd, int retry_count); 

    IP::ClientSocket * skt; 

    // For N200's that don't connect to an external serial widget
    // (that is, if we find that the GPSDO is present, or that 
    // there is no answer on the TR switch, 
    uhd::usrp::multi_usrp::sptr usrp; 
    int mboard; 
  };
}
#endif
