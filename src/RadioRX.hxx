/*
Copyright (c) 2026 Matthew H. Reilly (kb1vc)
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
 *  @file RadioRX.hxx @brief Thread class that provides the common IF
 *  receive functions for any supported radio
 *  (USRP/Adalm/Pluto/Lyme/...)
 *  The IF processing step (beat incoming RX stream from the IF out of
 *  the radio itself, down to baseband for the BaseBandRX process)
 *  requires the IF chain to maintain a local oscillator of its own.
 *
 * The control thread is responsible for sending the current IF frequency
 * to the RX unit. The RX unit must monitor the current IF and the current
 * tuned frequency to make adjustments to its LO. 
 *
 *  @author M. H. Reilly (kb1vc)
 *  @date   July 2026
 */
#pragma once
#include "SoDaBase.hxx"
#include "SoDaThread.hxx"
#include "Command.hxx"
#include "Params.hxx"

#include <SoDa/MailBox.hxx>

namespace SoDa {
  /**
   *  @class RadioControl
   * 
   *  RadioRX listens on the Command Stream message channel for
   *  requests from other components (including the SoDa::UI listener)
   *  and dumps status and completion reports back onto
   *  the command stream channel.
   *
   *  A Receiver does no processing to the incoming RF signal other than
   * 1. optionally resample the input stream
   * 2. publish it on the rx_stream
   * 3. downsample it and publish that on the if_stream.
   *
   */
  class RadioRX : public SoDa::Thread {
  public:
    /**
     * Constructor
     * Build a RadioRX thread
     * @param params Pointer to a parameter object with all the initial settings
     * and identification for the attached USRP
     */
    RadioRX(ParamsPtr params);

    /**
     * @brief start the thread
     *
     * If a subclass needs to perform its own work, it should declare
     * an auxRun method.
     */
    void run() final;

    /**
     * @brief perform initialization. Not all modules need to implement this.
     */
    virtual void init() { }
    
  protected:
    ParamsPtr params;
    /**
     * @brief Parse an incoming command and dispatch.
     *
     * If a subclass needs to handle any command type, it
     * should declare its own subExecCommand method.
     *
     * @param cmd a command record
     */ 
    /**
     * @brief Parse an incoming GET command and dispatch.
     *
     * If a subclass needs to handle any command type, it
     * should declare its own subExecCommand method.
     *
     * @param cmd a command record
     */
    virtual void execGetCommand(CommandPtr cmd) final;

    /**
     * @brief Parse an incoming SET command and dispatch.
     *
     * If a subclass needs to handle any command type, it
     * should declare its own subExecSetCommand method.
     *
     * @param cmd a command record
     */
    virtual void execSetCommand(CommandPtr cmd) final;

    /**
     * @brief Parse an incoming REPort command and dispatch.
     *
     * If a subclass needs to handle any command type, it
     * should declare its own subExecRepCommand method.
     *
     * @param cmd a command record
     */
    virtual void execRepCommand(CommandPtr cmd) final;

    /// get the number of seconds since the "Epoch"
    /// @return relative time in seconds
    double getTime();


    /**
     * @brief SoDa Threads must subscribe to at least the control mailbox.
     *
     * @param mailboxes a list of all defined mailboxes. Choose your subscriptions.
     */ 
    virtual void subscribeToMailBoxes(const std::vector<MailBoxBasePtr> & mailboxes) final; 
    

  private:
    // These are the command handlers. They call virtual methods provided
    // by specific radio modules. They can't be over-ridden. 
    
    /// Methods that a radio MAY implement
    /// (see above)
    virtual void subExecCommand(CommandPtr cmd) { }
    virtual void subExecGetCommand(CommandPtr cmd) { }
    virtual void subExecSetCommand(CommandPtr cmd) { }
    virtual void subExecRepCommand(CommandPtr cmd) { }
    
    /// Methods that ALL radios must implement.
  public:
    /**
     * Set the last LO (convert to baseband) frequency for the NCO.
     *
     * This is called in response to a RX_NCO_FREQ message from the
     * RadioControl thread.
     *
     * @param freq LO3 frequency
     * its front-end frequency so that it can adjust its own oscillator.
     */
    virtual void setNCOFreq(double freq) = 0;

    /**
     * @brief A receiver must accept input data from the rx_stream
     * and beat it against its NCO to produce output on its if_stream.
     * If the stream_processing flag is off, any rx_input data will be
     * dumped.
     *
     * @return true if the receiver had input data ready to convert, false otherwise.
     */
    virtual bool downConvert() = 0;

    /**
     * @brief flush the input rx data stream and set the stream_processing flag to on.
     */
    virtual void startStream() = 0;

    /**
     * @brief flush the input rx data stream and set the stream_processing flag to off
     */
    virtual void stopStream() = 0;

    /**
     * @brief test the processing stream flag
     * @return true if rx data should be down converted
     */
    virtual bool streamEnabled() = 0;

    /**
     * @brief Enable IF streamer - This passes the incoming RF sample buffer onto the
     * IF stream where it can be consumed by the periodogram widget or anybody else.
     *
     * @param enable if true, send RF sample buffer onto if_stream, otherwise, don't. 
     */
    virtual void enableIFStreamer(bool enable) = 0;
    
  protected:
    
    CDatMBoxPtr rx_stream;
    CDatMBoxPtr if_stream;
    CmdMBoxPtr cmd_stream;
    CmdMBox::Subscription cmd_subs;

    double current_IF_tuning; ///< current IF NCO frequency
  };
}
