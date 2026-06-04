#pragma once
/*
Copyright (c) 2012, Matthew H. Reilly (kb1vc)
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

#include "RadioRX.hxx"
#include "SoDaBase.hxx"
#include "SoDaThread.hxx"

#include "Command.hxx"
#include "Params.hxx"
#include "UI.hxx"
#include "QuadratureOscillator.hxx"
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/stream.hpp>

#include <SoDa/MailBox.hxx>

#include <memory>

namespace SoDa {
  class USRPRX;
  typedef std::shared_ptr<USRPRX> USRPRXPtr;
  
  /**
   * The Receive RF Path
   *
   * @image html SoDa_Radio_RX_Signal_Path.svg
   */
  class USRPRX : public SoDa::RadioRX {
  protected:
    /**
     * The constructor
     *
     * @param params a pointer to a params object that will tell us
     *        about sample rates and other configuration details.
     * @param usrp a pointer to the UHD USRP object that we are streaming data from.
     */
    USRPRX(ParamsPtr params, uhd::usrp::multi_usrp::sptr usrp);
  public:
    static USRPRXPtr make(ParamsPtr params, uhd::usrp::multi_usrp::sptr _usrp) {
      auto ret = std::shared_ptr<USRPRX>(new USRPRX(params, _usrp));
      ret->self = ret;
      ret->registerThread(ret);
      return ret;
    }

    /**
     * Set the last LO (convert to baseband) frequency for the NCO.
     *
     * This is called in response to a SET RX_IF_FREQ message from the
     * RadioControl thread.
     *
     * @param freq LO3 frequency
     */
    void setNCOFreq(double freq);

    /**
     * @brief A receiver must accept input data from the rx_stream
     * and beat it against its NCO to produce output on its if_stream.
     * If the stream_processing flag is off, any rx_input data will be
     * dumped. 
     *
     * @return true if the receiver had input data ready to convert, false otherwise.
     */
    bool downConvert();

    /**
     * @brief flush the input rx data stream and set the stream_processing flag to on.
     */
    void startStream();

    /**
     * @brief we never stop a USRP stream until the radio shuts down
     */
    void stopStream();

    /**
     * @brief stop and close down the USRP input stream
     */
    void closeStream();

    /**
     * @brief test the processing stream flag
     * @return true if rx data should be down converted
     */
    bool streamEnabled();
    
    /**
     * @brief Enable IF streamer - This passes the incoming RF sample buffer onto the
     * IF stream where it can be consumed by the periodogram widget or anybody else.
     *
     * @param enable if true, send RF sample buffer onto if_stream, otherwise, don't. 
     */
    void enableIFStreamer(bool enable);

  private:   
    /**
     * @brief implement a complex down converter with complex multiplication
     *
     * This is a complex downconverter in the manner presented in Lyons pp 457-458
     *
     * @param inout the input/output RF buffer
     */
    void doMixer(SoDa::CBufPtr inout);

    /**
     * @brief The USRP never stops the RX streamer once it starts. 
     */ 
    void actualStartStream(); 

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

    std::weak_ptr<USRPRX> self; 
  }; 
}

