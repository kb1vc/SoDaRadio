#pragma once
/*
Copyright (c) 2012,2013,2014, 2026 Matthew H. Reilly (kb1vc)
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
#include "SoDaBase.hxx"
#include "SoDaThread.hxx"
#include "Command.hxx"
#include "Params.hxx"
#include "QuadratureOscillator.hxx"

#include <SoDa/MailBox.hxx>

#include <memory>

namespace SoDa {
  /**
   * The Transmit RF Path
   *
   * @image html SoDa_Radio_TX_Signal_Path.svg
   *
   * In SSB/AM/FM modes, the RadioTX unit accepts an I/Q audio
   * stream from the BaseBandTX unit and forwards it to the USRP.
   * In CW mode, the RadioTX unit impresses a CW envelope (received
   * from the CW unit) onto a carrier and passes this to the USRP.
   *
   * The hardware specific portion of the RadioTX does very little.
   * It must provide methods to enable and disable the transmit hardware
   * chain and it must pass or enqueue blocks of samples to the hardware
   * transmitter. 
   *
   */
  class RadioTX;
  typedef std::shared_ptr<RadioTX> RadioTXPtr;
  
  class RadioTX : public SoDa::Thread {
  public:
    /**
     * @brief Constructor for RF Transmit/modulator process
     *
     * @param params block describing intial setup of the radio
     * @param name the name of this TX block (typcally indicates radio type)
     *
     */
    RadioTX(ParamsPtr params, const std::string & name);

  private:
    /**
     * @brief RadioTX run loop: handle commands, and modulate the tx carrier
     */
    void run(); 

    /**
     * @brief SoDa Threads must subscribe to at least the control mailbox.
     *
     * @param mailboxes a list of all defined mailboxes. Choose your subscriptions.
     */ 
    virtual void subscribeToMailBoxes(const std::vector<MailBoxBasePtr> & mailboxes) final; 

    /**
     *
     * Some initialization must occurr after all the components are created.
     * Units may optionally implement this method to do that. 
     *
     */
    virtual void init() { }; 
    
  protected:
    // All hardware transmitters must implement these functions:
    /**
     * @brief most transmit chains will require multiple stages to enable the
     * transmitter.  -- switch the antenna, turn on the power amplifier, start
     * the modulator.  Actual control of the antenna is left to the control unit.
     * The transmitter need only provide a "transmitEnable" function that will
     * be called after the antenna switch has flipped. 
     *
     * @param tx_on if true, enable the transmit chain. Disable the transmitter
     * if false.
     *
     * @return true if the unit was in Trasnmit mode, false otherwise. 
     */ 
    virtual bool transmitSwitch(bool tx_on) = 0;


    /**
     * @brief The TX hardware unit doesn't do much at all.  Get bits, Put bits.
     * This is (or may be) a blocking call. 
     *
     * @param buf a shared pointer to a buffer of complex samples. These may be
     * enqueued or sent directly to the hardware.
     *
     * @return true if the buffer was sent directly to the hardware.
     */
    virtual bool put(CBufPtr buf) = 0;

    /**
     * Some hardware radio modules might want to watch or publish
     * to the command mailbox.  They may, but shouldn't; 
     */ 
    virtual void subExecCommand(CommandPtr cmd) { }
    virtual void subExecGetCommand(CommandPtr cmd) { }
    virtual void subExecSetCommand(CommandPtr cmd) { }
    virtual void subExecRepCommand(CommandPtr cmd) { }

    virtual RadioTXPtr getSelfPtr() = 0;

    bool tx_enabled; ///< if true, we're transmitting.

    CBufPtr zero_buf; ///< envelope for dead silence.
    
  private:
    /**
     * @brief execute GET commands from the command channel
     * @param cmd the incoming command
     */
    void execGetCommand(CommandPtr cmd);
    /**
     * @brief handle SET commands from the command channel
     * @param cmd the incoming command
     */
    void execSetCommand(CommandPtr cmd);
    /**
     * @brief handle Report commands from the command channel
     * @param cmd the incoming command
     */
    void execRepCommand(CommandPtr cmd);

    /**
     * @brief set the CW tone frequency to generate an IQ stream
     *
     * @param usb  if true, put the tone above the nominal carrier
     * @param freq frequency of the CW tone (offset from zero beat)
     * 
     */
    void setCWFreq(bool usb, double freq);

    /**
     * @brief given a keying envelope, impose it on the CW tone
     *
     * @param out the output IQ buffer, CW_osc amplitude modulated by
     *        the envelope parameter
     * @param envelope float array of keyed waveform amplitudes
     *
     */
    void doCW(CBufPtr out, FBufPtr envelope);
    
    CDatMBox::Subscription tx_subs;  ///< subscription handle for transmit audio stream (from BaseBandTX)
    CmdMBox::Subscription cmd_subs;  ///< subscription handle for command stream
    FDatMBox::Subscription cw_subs;  ///< subscription handle for cw envelope stream (from CW unit)

    CDatMBoxPtr tx_stream;  ///< transmit audio stream 
    FDatMBoxPtr cw_env_stream; ///< envelope stream from text-to-CW converter (CW unit)
    CmdMBoxPtr cmd_stream; ///< command stream
    

    double CW_tone_freq;
    QuadratureOscillator CW_osc; ///< CW tone IQ oscillator

    FBufPtr beacon_env; ///< steady constant amplitude envelope
    FBufPtr zero_env;   ///< zero amplitude envelope
    CBufPtr cw_buf;     ///< working buffer for CW output
    bool beacon_mode;   ///< if true, we're transmitting a steady carrier

    Command::ModulationType tx_modulation; ///< current modulation mode

    double tx_sample_rate; ///< sample rate for buffer going to USRP (UHD)
    unsigned int tx_buffer_size; ///< size of buffer going to USRP
    float cw_env_amplitude;  ///< used to set CW output envelope, constant at 0.7

    bool waiting_to_run_dry; ///< When set, we should send out a report when we run out of CW buffer
  }; 

}
