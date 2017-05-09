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
 ///  @file SoapySDRCtrl.hxx
 ///  @brief Thread class that owns the SoapySDR control channel and functions
 ///
 ///
 ///  @author M. H. Reilly (kb1vc)
 ///  @date   May 2017
 ///

#ifndef SoapyCTRL_HDR
#define SoapyCTRL_HDR
#include "SoDaBase.hxx"
#include "MultiMBox.hxx"
#include "Command.hxx"
#include "Params.hxx"
#include "TRControl.hxx"
#include "PropTree.hxx"

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Types.hpp>
#include <SoapySDR/Logger.hpp>

namespace SoDa {


  ///  @class SoapyCtrl
  /// 
  ///  Though libuhd is designed to be re-entrant, there are some indications
  ///  that all control functions (set_freq, gain, and other operations)
  ///  should originate from a single thread.  SoapyCtrl does that.
  ///
  ///  SoapyCtrl listens on the Command Stream message channel for
  ///  requests from other components (including the SoDa::UI listener)
  ///  and dumps status and completion reports back onto
  ///  the command stream channel. 
  class SoapyCtrl : public SoDaThread {
  public:
    /// Constructor
    /// Build a SoapyCtrl thread
    /// @param params Pointer to a parameter object with all the initial settings
    /// and identification for the attached SoapySDR
    /// @param _cmd_stream Pointer to the command stream message channel.
    SoapyCtrl(const std::string & driver_name, Params * params, CmdMBox * _cmd_stream);

    void close() {
      SoapySDR::Device::unmake(radio);
    }

    /// start the thread
    void run();

    /// return a pointer to the Soapy object -- used by
    /// RX and TX processes to find the associated SoapySDR widget.
    /// @return a pointer to the SoapySDR radio object
    SoapySDR::Device * getSoapySDR() { return radio; }

    bool isReady() { return is_ready; }
  private:
    Params * params;

    double last_rx_tune_freq;
    double last_tx_tune_freq; 

    bool is_ready; /// true if we're done with the post START initialization
    bool tx_on; 

    float rx_rf_gain, tx_rf_gain; 
    
    double tx_freq, rx_freq; 
    double tx_freq_rxmode_offset; 
    double rxmode_offset; 
    double tx_samp_rate; 

    std::string tx_ant; 

    std::string model_name; 

    SoDa::CmdMBox * cmd_stream; 
    int subid; 


    // SoapySDR stuff.
    SoapySDR::Device * radio; ///< to which SoapySDR unit is this connected?
    SoapySDR::Range tx_rf_gain_range, rx_rf_gain_range; 
    SoapySDR::Range tx_freq_range, rx_freq_range; 

    std::vector<std::string> GPIO_list;
    std::string tr_control_reg; 

    /// external control widget for TR switching and other things. 
    SoDa::TRControl * tr_control; 

    void makeRadio(const std::string & driver_name); 

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

    void initControlGPIO();

    void setTXEna(bool tx_on);

    void set1stLOFreq(double freq, int sel, bool set_if_freq);
  };
}


#endif
