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

#ifndef B200Serial_HDR
#define B200Serial_HDR

#include "TRControl.hxx"
#include "IPSockets.hxx"
#include <string>
#include <list>
#include <boost/format.hpp>
#include <iostream>
#include <uhd/usrp/multi_usrp.hpp>


namespace SoDa {
  /**
   * @brief B2xxy-z subtypes for simplified usage (DD0VS)
   * 
   * This is a workaraound, to integrate B200mini (Sept 2019) into 
   * SoDaRadio.
   * All specific action for B200/B210/B2xx are not carried out for
   * B200mini.
   * On specific places special care will be taken for B200mini only.
   * In a first attempt TX/RX relay switching will be implemented via
   * B200mini GPIO pin. Today (16th of Feb 2020, all versions of B2xx 
   * have GPIOs). To avoid interferences with the original code only 
   * B200mini specific actions will be carried, the others are kept
   * functional.
   */
  typedef enum {
    eB200,      // e = enum; B200 seems to be defined somewhere else
    eB210,
    eB200mini,
    eB200mini_i,
    eB205mini_i
    } SoDa_B200_type_t;
  
  /**
   * @brief Transmit/Receive switch control for B200/B210 via the 
   * FX3 debug GPIO pins. 
   *
   * This module requires a special version of the B2x0 fpga firmware. 
   * 
   * On TRANSMIT, pin 1 of J400 will be pulled HIGH.  On RECEIVE
   * pin 1 of J400 will be pulled LOW.  
   * On TRANSMIT, pin 3 of J400 will be pulled LOW.  On RECEIVE
   * pin 1 of J400 will be pulled HIGH.  
   */
  class B200Control : public TRControl {
  public:
    /**
     * @brief constructor
     * @param usrp a pointer to the radio object
     * @param mboard which B200 object in the usrp
     *
     */
    B200Control(uhd::usrp::multi_usrp::sptr usrp, int mboard = 0);
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

    uhd::usrp::multi_usrp::sptr usrp; 
    
    int mboard; 
    SoDa_B200_type_t B200_type;   //see comment above
    std::string modelname;  //is redundant, but used for simpler programming
  };
}
#endif
