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
 *   @file TXBase.hxx
 *   @brief Base class describing receiver functions provided by any supported radio. 
 * 
 * 
 *   @author M. H. Reilly (kb1vc)
 *   @date   July 2022
 */


#include "SoDaBase.hxx"
#include "Thread.hxx"
#include "MailBoxTypes.hxx"
#include "Command.hxx"
#include "Params.hxx"

namespace SoDa {

  /**
   *   @class TXBase
   *  
   * TXBase listens on the Command Stream message channel for
   * requests from other components (including the SoDa::UI listener)
   * and dumps status and completion reports back onto the command
   * stream channel.
   *
   * An RX class gets sample buffers from the associated radio and
   * translates the samples into complex float before placing a buffer
   * on the IF stream.  optinally "beats it down" and forwards them to
   * the BaseBandRX unit via the rx_stream.
   * 
   */ 
  class TXBase : public SoDa::Thread {
  public:
    /**
     *  Constructor Build a TXBase thread
     */
    TXBase(const std::string & name); 

    /**
     * @brief perform any actions that need to occur before the first
     * call to runTick(). This is called *after* all mailboxes are subscribed. 
     * The init method should NOT loop and should not attempt to *read* a mailbox. 
     */
    virtual void init() {}
    
    /**
     * @brief perform any actions that need to occur *after* the final 
     * call to runTick. (This isn't really a destructor, as it probably 
     * makes sense to do this before any wrap up.  We might need to do
     * things in a particular order?  Idunno.)
     */
    virtual void cleanUp() {}
    
    /**
     *  Do the thread's work.  Note that this cannot be over-ridden by the 
     * derived class. If the derived class needs to do something more 
     * than poll the command stream, it should implement "runTick"
     */
    void run() final;

    /**
     *
     * @brief do one interval of work.  This method *must* return whenever it
     * has done a "finite" amount of work. At minumum, it should not run for longer
     * than 10 ms before returning to the run method. 
     *
     * @returns true if "runTick" has done all the work required in a run loop
     * If true, the run loop will sleep for the duration in microseconds specified by
     * getSleepDuration(). If false, run will check the command queue and then 
     * immediately call runTick().
     * 
     */
    virtual bool runTick() = 0;

    /**
     * @brief report the RX stream sample rate.  This is the sample rate
     * for both the IF and RX streams. The run method will use this value
     * to notify the Ctrl, BaseBandRX, UI, and spectrogram widgets. 
     */
    virtual double sampleRate() = 0;
    
    /**
     * @brief tell the run loop how long to sleep between work intervals. 
     * @returns the number of microseconds that this thread may sleep
     * between doing work in "runTick"
     */
    virtual unsigned int getSleepDuration() { return 1000; }

    /**
     *  @brief implement the subscription method
     * 
     * Subclasses may override this method in order to connect to mailboxes 
     * other than Cmd, RF, IF, but should call back to 
     * TXBase::subscribe to get connected to the cmd, rf, and if mailboxes. 
     */
    virtual void subscribe();

    /** 
     *  Parse an incoming command and dispatch.
    *  @param cmd a command record
    */
    virtual void execCommand(CmdMsg cmd);
    
    /**
     * @brief dispatch and incoming GET request that is special for
     * this radio, and dispatch.
     * 
     * @param cmd a command record
     */
    virtual void execGetCommand(CmdMsg cmd) = 0;

    /**
     * @brief dispatch and incoming SET request that is special for
     * this radio, and dispatch. 
     * 
     * @param cmd a command record
     */
    virtual void execSetCommand(CmdMsg cmd) = 0;

    /**
     * @brief process an incoming Report response (response from GET
     * and also a report from some SET operations).
     * 
     * @param cmd a response record
     *
     */
    virtual void execRepCommand(CmdMsg cmd) = 0;
      
    
  protected:
    Params * params;

    bool is_initialized;

    MsgMBoxPtr cmd_stream; ///< command stream    
    MsgSubs cmd_subs; ///< subscription handle for command stream

    CFMBoxPtr tx_stream;  ///< transmit audio stream     
    CFSubs tx_subs;  ///< subscription handle for transmit audio stream (from BaseBandTX)

    FMBoxPtr cw_env_stream; ///< envelope stream from text-to-CW converter (CW unit)    
    FSubs cw_subs;  ///< subscription handle for cw envelope stream (from CW unit)
  };
}
