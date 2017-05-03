/*
Copyright (c) 2017, Matthew H. Reilly (kb1vc)
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

 ///
 ///  @file RadioCtrl.hxx
 ///  @brief Base class that owns the control interface to the radio.
 ///
 ///
 ///  @author M. H. Reilly (kb1vc)
 ///  @date   April 2017
 ///

#ifndef RADIOCTRL_HDR
#define RADIOCTRL_HDR
#include "SoDaBase.hxx"
#include "MultiMBox.hxx"
#include "Command.hxx"
#include "Params.hxx"
#include "TRControl.hxx"
#include "PropTree.hxx"

namespace SoDa {


  ///  @class RadioCtrl
  /// 
  ///  Though libuhd is designed to be re-entrant, there are some indications
  ///  that all control functions (set_freq, gain, and other operations)
  ///  should originate from a single thread.  RadioCtrl does that.
  ///
  ///  RadioCtrl listens on the Command Stream message channel for
  ///  requests from other components (including the SoDa::UI listener)
  ///  and dumps status and completion reports back onto
  ///  the command stream channel. 
  class RadioCtrl : public SoDaThread {
  public:
    /// Constructor
    /// Build a RadioCtrl thread
    /// @param params Pointer to a parameter object with all the initial settings
    /// and identification for the attached USRP
    /// @param _cmd_stream Pointer to the command stream message channel.
    RadioCtrl(Params * params, CmdMBox * _cmd_stream);
    /// start the thread
    void run();


    virtual void run_prefix() { 
      // derived classes should put any pre-run-loop operations here. 
      // They will be executed before the thread loop. 
    }

    /// return a pointer to the multi_usrp object -- used by
    /// RX and TX processes to find the associated USRP widget.
    /// @return a pointer to the USRP radio object
    uhd::usrp::multi_usrp::sptr getUSRP() { return usrp; }


    /// This is the more permanent message handler... 
    static void normal_message_handler(uhd::msg::type_t type, const std::string & msg);

    /// This is a singleton object -- the last (and only, we hope) such object
    /// to be created sets a static pointer to itself.  This looks pretty gross, but
    /// it is necessary to provide context to the error message handlers.
    static SoDa::RadioCtrl * singleton_ctrl_obj;
    
  protected:
    Params * params;

    /// get the name of the hardware platform (eg B200, LimeSDR, ...)
    virtual std::string readPlatformName() = 0; 

    /// Parse an incoming command and dispatch.
    /// @param cmd a command record
    void execCommand(Command * cmd);
    /// Dispatch an incoming GET command
    /// @param cmd a command record
    void execGetCommand(Command * cmd); 
    /// Dispatch an incoming SET command
    /// @param cmd a command record
    void execSetCommand(Command * cmd); 
    /// Dispatch an incoming REPort command
    /// @param cmd a command record
    void execRepCommand(Command * cmd); 

    /// get the number of seconds since the "Epoch"
    /// @return relative time in seconds
    double getTime();

    CmdMBox * cmd_stream; ///< command stream channel
    unsigned int subid;   ///< subscriber ID for this thread's connection to the command channel



    // Capability Flags --
    bool supports_tx_gpio; ///< does this unit support GPIO signals?  (B2xx does not as of 3.7.0)
    

    double first_gettime; ///< timestamps are relative to the first timestamp.

    // gain settings
    double rx_rf_gain; ///< rf gain for RX front end amp/attenuator
    double tx_rf_gain; ///< rf gain for final TX amp

    // state of the box
    bool tx_on; ///< if true, we are transmitting.

    double last_rx_req_freq;  ///< remember the last setting -- useful for "calibration check" 
    
    // we horse the TX tuning around when we switch to RX
    // to move the transmit birdie out of band.
    double tx_freq; ///< remember current tx freq 
    double tx_freq_rxmode_offset; ///< when in RX mode, move tx off frequency to put the tx birdie out of band, when in TX mode, this is 0
    static const double rxmode_offset; ///< tx offset when in RX mode

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


#endif
