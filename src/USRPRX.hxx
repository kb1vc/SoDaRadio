/*
  Copyright (c) 2012, 2025 Matthew H. Reilly (kb1vc)
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
#pragma once 

#include "SoDaBase.hxx"
#include "SoDaThread.hxx"

#include "Command.hxx"
#include "Params.hxx"
#include "UI.hxx"
#include "QuadratureOscillator.hxx"
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/stream.hpp>

#include <SoDa/MailBox.hxx>

namespace SoDa {
  /**
   * The Receive RF Path
   *
   * @image html SoDa_Radio_RX_Signal_Path.svg
   */
  class USRPRX;
  typedef std::shared_ptr<USRPRX> USRPRXPtr;

  
  class USRPRX : public SoDa::Thread {
  private:
    /**
     * The constructor
     *
     * @param params a pointer to a params object that will tell us
     *        about sample rates and other configuration details.
     * @param usrp a pointer to the UHD USRP object that we are streaming data from.
     */
    USRPRX(ParamsPtr params, uhd::usrp::multi_usrp::sptr usrp);

  public:
    static USRPRXPtr make(ParamsPtr params, uhd::usrp::multi_usrp::sptr usrp) {
      auto ret = std::shared_ptr<USRPRX>(new USRPRX(params, usrp));
      ret->registerThread(ret);
      return ret; 
    }

    /**
     * @brief connect to useful mailboxes. 
     * 
     * @param mailboxes list of mailboxes to which we might subscribe.
     */
    void subscribeToMailBoxes(const std::vector<MailBoxBasePtr> & mailboxes);  

    /**
     * USRPRX is a thread -- this is its run loop. 
     */
    void run();
    
  private:   
    void execCommand(CommandPtr cmd); 
    void execGetCommand(CommandPtr cmd); 
    void execSetCommand(CommandPtr cmd); 
    void execRepCommand(CommandPtr cmd);

    void startStream();
    void stopStream(); 

    /**
     * @brief implement a complex down converter with complex multiplication
     *
     * This is a complex downconverter in the manner presented in Lyons pp 457-458
     *
     * @param inout the input/output RF buffer
     */
    void doMixer(SoDa::CBufPtr inout);
    void set3rdLOFreq(double IF_tuning);

    CDatMBoxPtr rx_stream;
    CDatMBoxPtr if_stream; 
    CmdMBoxPtr cmd_stream;
    CmdMBox::Subscription cmd_subs; 

    // state for the USRP widget
    uhd::rx_streamer::sptr rx_bits;
    uhd::usrp::multi_usrp::sptr usrp; 

    unsigned int rx_buffer_size;

    // are we collecting?
    bool audio_rx_stream_enabled;

    SoDa::Command::ModulationType rx_modulation;
    
    // pointer to user interface box -- we send it FFT snapshots
    UI * ui;

    // IF tuner
    QuadratureOscillator IF_osc;
    double current_IF_tuning;
    double rx_sample_rate;

    // spectrum reporting
    bool enable_spectrum_report; 
    
    //debug hooks
    int outf[2];
    int scount;

    std::ofstream rf_dumpfile;
    std::ofstream if_dumpfile; 
    
  }; 
}

