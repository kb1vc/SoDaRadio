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

#ifndef TR_CONTROL_HDR
#define TR_CONTROL_HDR

#include <uhd/usrp/multi_usrp.hpp>

namespace SoDa {

  /**
   * @brief Generic Control class to activate T/R switching, band 
   * switching, and other control functions. 
   *
   * On some units, this may be provided by GPIO pins on the USRP itself. 
   * (Systems with WBX modules and later model B2xx units have internal
   * GPIO pins.) Others (like my own N200 with a UBX module) need an external
   * widget of some sort. 
   *
   * All members of the control class provide the basic features, but 
   * may choose to treat them as no-ops.  
   *
   * However, ALL subclasses of Control.hxx must do something useful for
   * setTXOn(), setTXOff(), getTX().  
   */
  class TRControl {
  public:
    TRControl() {
    }

    /**
     * @brief activate external controls to enable transmit mode.
     * @return true if the operation completed successfully 
     */
    virtual bool setTXOn() = 0; 

    /**
     * @brief activate external controls to disable transmit mode.
     * @return true if the operation completed successfully 
     */
    virtual bool setTXOff() = 0; 

    /** 
     * @brief report state of transmit-enable. 
     * @return true iff the transmitter is currently enabled.
     */
    virtual bool getTX() = 0; 

    /**
     * @brief turn on/off signal path for selected band
     * note that multiple bands can be enabled at one time(!)
     *
     * @param band band selector (typically index from 1...7)
     * @param state true to enable band, false otherwise. 
     * @return true if band state was changed. 
     */
    virtual bool setBand(unsigned int band, bool state) { (void) band; (void) state; return false; }

    /**
     * @brief query state of signal path for selected band
     *
     * @param band band selector (typically index from 1...7)
     * @return true if band signal path is enabled. 
     */
    virtual bool getBand(unsigned int band) { (void) band; return false; }

    /**
     * @brief set state of selected servo
     *
     * @param servo_sel -- select the actuator that we're wiggling
     * @param val -- the setting for the actuator
     * @return true if the actuator command was executed.
     */
    virtual bool setServo(unsigned int servo_sel, double val) { (void) servo_sel; (void) val; return false; }

    /** 
     * @brief make the appropriate TR control widget given a pointer to a 
     * USRP device.  This will be connected to the selected mboard. 
     */
    static TRControl * makeTRControl(uhd::usrp::multi_usrp::sptr usrp, int mboard = 0); 
    
  }; 

  /**
   * a null control class that returns happily for everyone. 
   */
  class NoopControl : public TRControl {
  public:
    NoopControl() : TRControl() {
    }
    bool setTXOn() { return true; }
    bool setTXOff() { return true; }
    bool getTX() { return true; }
    
  }; 


}

#endif // TR_CONTROL_HDR
