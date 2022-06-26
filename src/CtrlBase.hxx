#pragma once
/*
Copyright (c) 2022, Matthew H. Reilly (kb1vc)
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

/** 
 *   @file CtrlBase.hxx
 *   @brief Base class describing functions 
 *   provided by any supported radio.
 * 
 * 
 *   @author M. H. Reilly (kb1vc)
 *   @date   June 2022
 */


#include "SoDaBase.hxx"
#include "SoDaThread.hxx"
#include "MultiMBox.hxx"
#include "Command.hxx"
#include "Params.hxx"

namespace SoDa {

  /**
   *   @class CtrlBase
   *  
   * CtrlBase listens on the Command Stream message channel for
   * requests from other components (including the SoDa::UI listener)
   * and dumps status and completion reports back onto the command
   * stream channel.
   * 
   * In the ideal case, derived classes would not need to worry about
   * the command stream at all. Any virtual methods that query state
   * return the relevant information, and the CtrlBase will package it
   * up to send out.
   * 
   * Likewise, incoming commands are dispatched by the CtrlBase
   * handlers.  The incoming stream is also passed to handlers in the
   * specific radio, but implementation of any handler is optional.
   */ 
  class CtrlBase : public SoDa::Thread {
  public:
    /**
     *  Constructor Build a CtrlBase thread @param params Pointer to a
     *  parameter object with all the initial settings and
     *  identification for the attached radio
     */
    CtrlBase(Params * params);

    /**
     *  Do the thread's work.  Note that this cannot be over-ridden by the 
     * derived class. If the derived class needs to do something more 
     * than poll the command stream, it should implement "runTick"
     */
    void run() final;

    /**
     * @returns true if "runTick" has done all the work required in a run loop
     * (checked the command stream and handled stuff.)
     */
    virtual bool runTick() { return false; }

    /**
     *  implement the subscription method
     */
    void subscribeToMailBox(const std::string & mbox_name, BaseMBox * mbox_p) final;

    /**
     * @brief dispatch and incoming GET request that is special for
     * this radio, and dispatch.  many of the commands will be handled
     * by the base class with calls to virtual methods.  but in the
     * case of commands that need peculiar handling, the specific
     * radio may provide its own handlers for some messages.
     * 
     * @param cmd a command record
     * @return true if the command was handled, false otherwise. 
     *
     * If the command was handled by the controller, the CtrlBase class
     * will ignore the command. (So if the derived class processes the particular
     * command code, and returns TRUE, the base class will ignore the command.)
     */
    virtual bool execGetCommand(Command * cmd) { return false; }

    /**
     * @brief dispatch and incoming GET request that is special for
     * this radio, and dispatch.  many of the commands will be handled
     * by the base class with calls to virtual methods.  but in the
     * case of commands that need peculiar handling, the specific
     * radio may provide its own handlers for some messages.
     * 
     * @param cmd a command record
     * @return true if the command was handled, false otherwise. 
     *
     * If the command was handled by the controller, the CtrlBase class
     * will ignore the command. (So if the derived class processes the particular
     * command code, and returns TRUE, the base class will ignore the command.)
     */
    virtual bool execSetCommand(Command * cmd) { return false; }

    /**
     * @brief process an incoming Report response (response from GET
     * and also a report from some SET operations) that is special for
     * this radio, and dispatch.  Many of the commands will be handled
     * by the base class with calls to virtual methods.  but in the
     * case of commands that need peculiar handling, the specific
     * radio may provide its own handlers for some messages.
     * 
     * @param cmd a response record
     * @return true if the command was handled, false otherwise. 
     *
     * If the command was handled by the controller, the CtrlBase class
     * will ignore the command. (So if the derived class processes the particular
     * command code, and returns TRUE, the base class will ignore the command.)
     */
    virtual bool execRepCommand(Command * cmd) { return false; }


    /**
    *  get the number of microseconds since the Ctrl widget was created
    *  @return relative time in seconds
    *
    virtual unsigned long  getTime();

    /**
     * report the antennas that are available, send the report on cmd_stream
     */
    virtual std::list<std::string> reportAntennas() = 0;

    /**
     * report the modulation modes that are implemented, send the report on cmd_stream
     */
    virtual std::list<std::string> reportModes() = 0;

    /**
     * Set the antenna choice.  Use "ant" if it is in the list
     * of alternatives. Otherwise, choose the first alternative.
     * @param ant the requested antenna
     * @param sel 'r' for RX, 't' for TX
     */
    void setAntenna(const std::string & ant, Radio::TXRX dir) = 0;

    /*
     *  Set the front-end (LO + DDS) frequency to 'freq'
     *  This includes setting the PLL front end synthesizer
     *  as well as the FPGA resident digital synthesizer.
     *  @param freq target frequency (LO and DDS combined)
     *  @param sel Radio::RX' for RX LO, Radio::TX for TX LO
     */
    virtual void setFreq(double freq, Radio::TXRX sel) = 0;

    /**
     * For some radios, the transmit/receive mode is determined by the 
     * control unit, not the RX or TX stream handler. (USRP f'rinstance). 
     */
    virtual void setTXOn(bool set) { }
    
    virtual bool getTXOn() { return false; }
    
    /**
     * @brief get frequency range for the tuner. 
     *
     * @param sel selects TX or RX
     * @return pair of doubles freq_min, freq_max in Hz. 
     */
    std::pair<double, double> getFreqRange(Radio::TXRX sel); 

    /**
     * @brief get range of gain settings for unit
     *
     * @param sel selects TX or RX
     * @return pair of doubles gain_min, gain_max in dB (relative). 
     */
    std::pair<double, double> getGainRange(Radio::TXRX sel); 
    

      
    
  protected:
    Params * params;

    CmdMBox * cmd_stream; ///< command stream channel
    unsigned int subid;   ///< subscriber ID for this thread's
			  ///connection to the command channel
    
  private:
    /** 
     *  Parse an incoming command and dispatch.
    *  @param cmd a command record
    */
    void execCommand(Command * cmd);
    /**
    *  Dispatch an incoming GET command
    *  @param cmd a command record
    */
    void execGetCommandBase(Command * cmd); 
    /**
    *  Dispatch an incoming SET command
    *  @param cmd a command record
    */

    void execSetCommandBase(Command * cmd);
    
    /**
    *  Dispatch an incoming REPort command
    *  @param cmd a command record
    */
    void execRepCommandBase(Command * cmd); 

  };
}
