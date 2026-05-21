#pragma once
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
 *  @file RadioControl.hxx
 *
 *  @brief Thread class that provides the common
 *  radio functions for any compatible radio. A model (USRP,
 *  Adalm/Pluto/Lyme/...)  is decribed in a subclass of RadioControl,
 *  and must furnish the virtual methods described here that allow
 *  change in RX or TX frequency, change in transmit state, change in
 *  attenuation or power settings, query for device capabilities and
 *  its device name and address.
 *
 *  @author M. H. Reilly (kb1vc)
 *  @date   May 2026
 */

#include "SoDaBase.hxx"
#include "SoDaThread.hxx"
#include "MultiMBox.hxx"
#include "Command.hxx"
#include "Params.hxx"

namespace SoDa {
  /**
   *  @class RadioControl
   * 
   *  Though libuhd is designed to be re-entrant, there are some indications
   *  that all control functions (set_freq, gain, and other operations)
   *  should originate from a single thread.  RadioControl does that.
   *
   *  RadioControl listens on the Command Stream message channel for
   *  requests from other components (including the SoDa::UI listener)
   *  and dumps status and completion reports back onto
   *  the command stream channel.
   */
  class RadioControl : public SoDa::Thread {
  public:
    /**
     * Constructor
     * Build a RadioControl thread
     * @param params Pointer to a parameter object with all the initial settings
     * and identification for the attached USRP
     */
    RadioControl(Params * params);

    /**
     * @brief start the thread
     *
     * If a subclass needs to perform its own work, it should declare
     * an auxRun method.
     */
    void run() final;

    /// implement the subscription method
    void subscribeToMailBox(const std::string & mbox_name, BaseMBox * mbox_p);
    
  protected:
    Params * params;
    /**
     * @brief Parse an incoming command and dispatch.
     *
     * If a subclass needs to handle any command type, it
     * should declare its own subExecCommand method.
     *
     * @param cmd a command record
     */ 
    virtual void execCommand(Command * cmd) final;

    /**
     * @brief Parse an incoming GET command and dispatch.
     *
     * If a subclass needs to handle any command type, it
     * should declare its own subExecCommand method.
     *
     * @param cmd a command record
     */ 
    virtual void execGetCommand(Command * cmd) final; 

    /**
     * @brief Parse an incoming SET command and dispatch.
     *
     * If a subclass needs to handle any command type, it
     * should declare its own subExecSetCommand method.
     *
     * @param cmd a command record
     */ 
    void execSetCommand(Command * cmd) final; 

    /**
     * @brief Parse an incoming REPort command and dispatch.
     *
     * If a subclass needs to handle any command type, it
     * should declare its own subExecRepCommand method.
     *
     * @param cmd a command record
     */ 
    void execRepCommand(Command * cmd) final; 

    /// get the number of seconds since the "Epoch"
    /// @return relative time in seconds
    double getTime();


  private:
    // These are the command handlers. They call virtual methods provided
    // by specific radio modules. They can't be over-ridden. 
    /**
     * @brief write all antenna names to the report message channel
     * Get these from the listAntennas call. 
     *
     * @param rxtx select which port RX, or TX
     */
    virtual void reportAntennas(SoDa::RXTX rxtx) final;


    /**
     * @brief checked state of front-end LO and back-end IF chain NCO.
     *
     * @param rxtx which is it? RX or TX
     * @return true if the unit control widget reports isLOLocked and the IF chain
     * has reported that it has set its NCO
     */
    bool isLocked(SoDa::RXTX rxtx);
    
    /// Methods that a radio MAY implement
    /// (see above)
    void auxRun() { }
    void subExecGetCommand(Command * cmd) { }
    void subExecSetCommand(Command * cmd) { }
    void subExecRepCommand(Command * cmd) { }

    
    /// Methods that ALL radios must implement.
    /**
    * is the identified (rx or tx) front-end LO locked?
    * If not, set the tuning frequency to "the right thing"
    *
    * @param rxtx RX, TX
    */
    virtual bool isLOLocked(SoDa::RXTX rxtx) = 0; 

    /**
     * @brief report the antennas that are available. we'll use this
     * to let the GUI know what the choices are. 
     *
     * @param rxtx select which port RX, or TX
     * @return list of antennas for rx/tx
     * This will later be used in the RadioControl initialization command sequence from "reportModes"
     *
     */
    virtual std::vector<std::string> listAntennas(SoDa::RXTX rxtx) = 0; 

    /**
     * Set the antenna choice.  Use "ant" if it is in the list
     * of alternatives. Otherwise, choose the first alternative.
     * @param ant the requested antenna
     * @param rxtx rx/tx selection
     */
    virtual void setAntenna(const std::string & ant, SoDa::RXTX rxtx) = 0;

    /**
     * @brief Set the RX or TX sample rate
     *
     * @param rate sample rate (in samples/sec)
     * @param rxtx choose RX chain or TX chain
     */
    virtual void setSampleRate(float rate, SoDa::RXTX rxtx) = 0;

    /**
     * @brief Get the current setting for the sample rate
     *
     * @param rxtx choose RX chain or TX chain
     * @return the sample rate (samples/sec)
     */
    virtual float getSampleRate(SoDa::RXTX rxtx) = 0;
    
    /**
     * @brief set tain on the RX or TX side
     *
     * @param gain gain - if not in range, we'll pick something good
     * @param rxtx  receive or transmit?
     * @return the actual gain setting
     */
    virtual float setRFGain(float gain, SoDa::RXTX rxtx) = 0;
    
    /**
     * @brief Set the front-end (LO + DDS) frequency to 'freq'
     * This includes setting the PLL front end synthesizer
     * as well as the FPGA resident digital synthesizer.
     * 
     * @param freq target frequency (analog LO and DDS in the FPGA)
     * @param rxtx select receiver or transmitter
     *
     * @return The actual LO frequency.
     *
     * The actual LO frequency may differ from the requested frequency by a small
     * margin. Some radios may have a tuning step that is more than a few Hz. In this
     * case we adjust the LO in the RX and TX objects. (This may be a bigger deal for the
     * TX object as we like to do all the modulation there at baseband and assume the
     * hardware takes care of the carrier frequency. 
     * 
     * Typically, the call for a receive request would look like this:
     *
     * actual_lo_freq = setLOFreq(144.215e6, SoDa::RX)
     *
     * If the target LO can tune in 20 kHz intervals, the actual LO would be 144.100e6 and
     * the if_freq would be 115e3 Hz. This would allow a "clean" window from 144.115 Mhz to
g     * 144.325 MHz in the spectrogram display.  The LO would need to be changed if the
     * next frequency was greater than 144.240 or less than 144.205 (I think)
     *
     * On the transmit side, the offset may be dictated by the frequency stepping increment
     * in the front-end LO.
     *
     * To keep everything civilized, setLOFreq should *never* be called by any other agent
     * than the RadioControl object. (Not even its sub-classes.)
     * 
     */
    virtual double setLOFreq(double freq, SoDa::RXTX rxtx) = 0;


    /**
     * @brief Return the current LO (front end) oscillator setting for RX or TX chain
     * @param rxtx select receiver or transmitter
     * @return frequency (Analog LO and DDS/FPGA)
     */ 
    virtual double getLOFreq(SoDa::RXTX rxtx) = 0;
    
    /**
     * @brief Set the recieve or transmit frequency to 'freq'
     * This includes setting the PLL front end synthesizer, 
     * the FPGA resident digital synthesizer (if it exists) and
     * the RX baseband or TX baseband oscillator. 
     * 
     * @param freq target frequency
     * @param rxtx select receiver or transmitter
     *
     *
     * This calls the actual radio control object's setLOFreq method.
     * 
     * The actual LO frequency may differ from the requested frequency by a small
     * margin. Some radios may have a tuning step that is more than a few Hz. In this
     * case we adjust the LO in the RX and TX objects. (This may be a bigger deal for the
     * TX object as we like to do all the modulation there at baseband and assume the
     * hardware takes care of the carrier frequency. 
     *
     * The RX LO frequency differs from the requested frequency to move the DC spike
     * in the IF spectrum away from the requested frequency. setFreq ensures 
     * the requested frequency (freq) is at least 100 kHz above the LO frequency
     * and no more than 200 kHz above the LO frequency. (This is less about the
     * RF sample rate and more about keeping a nice window around the tuned frequency
     * so the periodogram or waterfall display is helpful. )
     *
     *
     * So we now have the concept of an IF frequency. The IF frequency is the difference
     * between the LO frequency and the requested frequency. We assume that the TX
     * chain will always have an IF frequency of 0 Hz. The RX chain will have an IF
     * frequency of freq - setLOFreq()
     *
     * In both cases, setFreq will send a SET, RX_IF_OSC or SET, TX_IF_OSC command to the RX or TX
     * threads. The RX or TX thread must send back REP, xx_IF_OSC with the setting that it accepted.
     * RadioControl will not report LO locked until both the hardware device reports that it is
     * locked and the REP, xx_IF_OSC message has returned. 
     *
     * Typically, the call for a receive request would look like this:
     *
     * if_freq = setFreq(144.215e6, SoDa::RX, 0)
     *
     * If the target LO can tune in 20 kHz intervals, the actual LO would be 144.100e6 and
     * the if_freq would be 115e3 Hz. This would allow a "clean" window from 144.115 Mhz to
     * 144.325 MHz in the spectrogram display.  The LO would need to be changed if the
     * next frequency was greater than 144.240 or less than 144.205 (I think)
     *
     * On the transmit side, the offset may be dictated by the frequency stepping increment
     * in the front-end LO.
     *
     * To keep everything civilized, setFreq should *never* be called by any other agent
     * than the RadioControl object. (Not even its sub-classes.)
     * 
     */
    void setFreq(double freq, SoDa::RXTX rxtx);
    
    /// get the state of the TXEna bit
    /// @return true if the TX relay is activated. 
    virtual bool getTXEna() = 0;
    /// get the state of the TX relay confirm bit
    /// @return true if the TX relay sense input is asserted
    virtual bool getTXRelayOn() = 0; 

    ///
    /**
     * @brief Initiate the actions that will turn on the transmitter
     * and (optionally) disable the receiver. Or enable the receiver
     * and turn off the transmitter.
     *
     * This is a multistep process and looks like this:
     *
     * To turn TX ON. (RadioControl receives TX_STATE TX_ON_0, full_duplex_flag
     * (Entries marked with X are performed by RadioControl, others must be implemented
     * in the model controller's setTXRXMode function. 
     *   1. X turn the RX gain to 0 (temporarily)
     *   2. X set the TX gain to its current requested level
     *   3. X Report the tx gain to the world
     *   4. X Set the tx local oscillator (in case it was in "tune remote mode"
     *   5.   Enable the rest of the tx hardware, including the antenna relay. (setTXEna(true, full_duplex))
     *   6. X Send a SET message to put TX_ON_1, full_duplex_flag
     *
     *  On receiving a SET TX_STATE TX_ON_1 CW_TX turns itself on. All other
     * TX_xxx values turn off CW_TX
     *
     * On receiving TX_ON_1 BaseBandTX turns on the audio input stream (unless we're in cw mode)
     * 
     *  On receiving a SET TX_STATE TX_ON_1 BaseBandRX flushes all its buffers unless in full duplex
     *  turn on the sidetone stream, if appropriate
     *
     * To turn TX OFF and RX ON (RadioControl receives TX_STATE TX_OFF_0, full_duplex_flag)
     * 1. X Set tx gain to 0
     * 2.   Disable transmitter / flip antenna switch
     * 3. X Sets rx gain to current level
     * 4. X Sets tx frequency to tx_freq + an offset that gets the tx LO out of the RX passband
     * 5.   Disable the TX hardware and flip antenna relay (setTXEna(false, full_duplex))
     * 6. X Send SET with TX_OFF_1
     *
     *
     * @param rxtxst RXTX (TX on/off) state - one of TX_ON_0, TX_ON_1, TX_OFF_0, TX_OFF_1
     * @param full_duplex if true, leave RX on while transmitter is enabled.
     */
    void setTXRXMode(Command::RxTxState rxtxst, bool full_duplex);


    /**
     * @brief report the model number and any other interesting features (like freq range)
     * to be displayed on the UI and such.
     *
     * @return a string describing the hardware. 
     */
    virtual std::string getHardwareDescription() = 0; 

  protected:

    /**
     * @brief see setFreq - this is a helper to find a good LO
     * frequency that puts the IF in a range that is convenient for
     * the spectrum display.
     *
     * @param freq desired final frequency
     * @param cur_lo_freq the last LO frequency sent to the front-end.
     *
     * @return -1 if @c freq is within the "good window" of @c cur_lo_freq. Otherwise,
     * return a good LO freq that puts @c freq in the good window.
     */
    double findGoodRXLO(double freq, double cur_lo_freq);
    
    CmdMBox * cmd_stream; ///< command stream channel
    unsigned int subid;   ///< subscriber ID for this thread's connection to the command channel

    double first_gettime; ///< timestamps are relative to the first timestamp.

    // gain settings
    double rx_rf_gain; ///< rf gain for RX front end amp/attenuator
    double tx_rf_gain; ///< rf gain for final TX amp

    // state of the box
    bool tx_on; ///< if true, we are transmitting.

    /// All of the following are indexed by the channel number. 
    double cur_rx_lo_freq; ///< The current receiver LO frequency 
    double new_rx_if_freq; ///< the last setting of the RX LO.
    double cur_rx_if_freq; ///< the RX LO frequency reported by the RadioRX front end.
    double cur_tx_lo_freq; ///< The current *requested* transmitter LO frequency 
    double new_tx_if_freq; ///< the last setting of the TX LO.
    double cur_tx_if_freq; ///< the TX LO frequency reported by the RadioRX front end.

    static const double tx_freq_rxmode_offset; ///< tx offset when in RX mode

    double tx_samp_rate; ///< sample rate to USRP TX chain. 
    std::string tx_ant;  ///< TX antenna choice (usually has to be TX or TX/RX1?

    std::string motherboard_name; ///< The model name of the USRP unit

    // transverter local oscillator support.
    bool tvrt_lo_capable; ///< if true, this unit can implement a local transverter oscillator.
    bool tvrt_lo_mode; ///< if true, set the transmit frequency, with some knowledge of the tvrt LO.
    double tvrt_lo_gain; ///< output power for the second transmit channel (used for transverter LO)
    double tvrt_lo_freq; ///< the frequency of the second transmit channel oscillator
    double tvrt_lo_fe_freq; ///< the frequency of the second transmit channel front-end oscillator
    
    // enables verbose messages
    bool debug_mode; ///< print stuff when we are in debug mode

    // integer tuning mode is helped by a map of LO capabilities.

    /// @brief applyTargetFreqCorrection adjusts the requested frequency,
    /// if necessary, to avoid a birdie caused by a multiple of the step
    /// size within the passband. It will also adjust the stepsize. 
    /// @param target_freq -- target tuning frequency
    /// @param avoid_freq -- the frequency that we must avoid by at least 1MHz
    /// @param tune_req -- tune request record. 
    void applyTargetFreqCorrection(double target_freq, 
				   double avoid_freq, 
				   uhd::tune_request_t * tune_req);


    /// @brief Test for support for integer-N synthesis
    /// @param force_int_N force LO tuning to use integer-N synthesis
    /// @param force_frac_N force LO tuning to use fractional-N synthesis
    void testIntNMode(bool force_int_N, bool force_frac_N);

    bool supports_IntN_Mode;  ///< if true, this unit can tune the front-end LO 
    ///< in integer-N mode (as opposed to fractional-N)
    ///< to improve rejection of spurious signals and 
    ///< drop the noise floor a bit.

    /// external control widget for TR switching and other things. 
    SoDa::TRControl * tr_control; 
  };
}

