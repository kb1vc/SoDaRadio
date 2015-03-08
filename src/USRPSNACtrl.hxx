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

 ///
 ///  @file USRPSNACtrl.hxx
 ///  @brief Thread class that owns the USRP control channel and functions
 ///  This is a specialization of the USRPCtrl object. 
 ///
 ///
 ///  @author M. H. Reilly (kb1vc)
 ///  @date   July 2015
 ///

#ifndef USRPSNACTRL_HDR
#define USRPSNACTRL_HDR
#include "USRPCtrl.hxx"


namespace SoDa {
  ///  @class USRPSNACtrl
  /// 
  ///  Though libuhd is designed to be re-entrant, there are some indications
  ///  that all control functions (set_freq, gain, and other operations)
  ///  should originate from a single thread.  USRPSNACtrl does that.
  ///
  ///  USRPSNACtrl listens on the Command Stream message channel for
  ///  requests from other components (including the SoDa::UI listener)
  ///  and dumps status and completion reports back onto
  ///  the command stream channel. 
  class USRPSNACtrl : public USRPCtrl {
  public:
    /// Constructor
    /// Build a USRPSNACtrl thread
    /// @param params Pointer to a parameter object with all the initial settings
    /// and identification for the attached USRP
    /// @param _cmd_stream Pointer to the command stream message channel.
    USRPSNACtrl(Params * params, CmdMBox * _cmd_stream, double _rx_offset = 100.0e3);

    /// the run loop for an SNA control object is simpler than 
    /// we'd see for a normal radio
    void run(); 

  private:
    // overload just the message handling methods
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

    /// increment the frequency
    void doStep();

    /// correct frequencies to avoid big spurs
    double correctFreq(double freq);

    /// set the RX freq
    void setRXFreq(); 
    /// use DDC tuning where we can, to avoid settling time
    bool rx_need_fe_retune;     
    double rx_last_fe_freq;

    /// set the TX freq
    void setTXFreq(); 
    bool tx_need_fe_retune;
    double tx_last_fe_freq;    






    enum SWEEP_STATE { IDLE, WAIT_FOR_RX } ; 

    SWEEP_STATE sweep_state; ///< what are we doing? 
    double current_sweep_freq; ///< the frequency being tested
    double start_sweep_freq; ///< starting sweep frequency
    double end_sweep_freq;   ///< ending sweep frequency
    double step_sweep_freq;  ///< increment by this at each step
    double time_per_step;    ///< approximate dwell time (in seconds)

    double rx_offset; ///< offset freq between RX and TX oscillators.
  };
}


#endif
