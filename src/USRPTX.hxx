/*
Copyright (c) 2012,2013,2014 Matthew H. Reilly (kb1vc)
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

#ifndef USRPTX_HDR
#define USRPTX_HDR
#include "SoDaBase.hxx"
#include "MultiMBox.hxx"
#include "Command.hxx"
#include "Params.hxx"
#include "QuadratureOscillator.hxx"
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/stream.hpp>

namespace SoDa {
  /**
   * The Transmit RF Path
   *
   * @image html SoDa_Radio_TX_Signal_Path.svg
   *
   * In SSB/AM/FM modes, the USRPTX unit accepts an I/Q audio
   * stream from the AudioTX unit and forwards it to the USRP.
   * In CW mode, the USRPTX unit impresses a CW envelope (received
   * from the CW unit) onto a carrier and passes this to the USRP. 
   *
   */
  class USRPTX : public SoDaThread {
  public:
    /**
     * @brief Constructor for RF Transmit/modulator process
     *
     * @param params block describing intial setup of the radio
     * @param usrp libuhd handle for the USRP radio
     * @param _tx_stream audio transmit stream to be used in modulator
     * @param _cw_env_stream envelope stream from text-to-CW converter
     * @param _cmd_stream command stream
     * @param transverter_capable true for B210 and others that have a second TX/RX channel
     *
     */
    USRPTX(Params * params, uhd::usrp::multi_usrp::sptr _usrp,
	   DatMBox * _tx_stream, DatMBox * _cw_env_stream,
	   CmdMBox * _cmd_stream,
	   bool transverter_capable = false);
    /**
     * @brief USRPTX run loop: handle commands, and modulate the tx carrier
     */
    void run(); 

  private:

    uhd::usrp::multi_usrp::sptr usrp; ///< the radio.
    
    /**
     * @brief start/stop transmit stream
     * @param tx_on if true, go into transmit mode
     */
    void transmitSwitch(bool tx_on);

    /**
     * @brief setup transmit streamer.
     */
    void getTXStreamer();
    
    /**
     * @brief execute GET commands from the command channel
     * @param cmd the incoming command
     */
    void execGetCommand(Command * cmd); 
    /**
     * @brief handle SET commands from the command channel
     * @param cmd the incoming command
     */
    void execSetCommand(Command * cmd); 
    /**
     * @brief handle Report commands from the command channel
     * @param cmd the incoming command
     */
    void execRepCommand(Command * cmd);

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
     * @param env_len length of envelope array
     *
     */
    void doCW(std::complex<float> * out, float * envelope, unsigned int env_len);
    
    unsigned int tx_subs;  ///< subscription handle for transmit audio stream (from AudioTX)
    unsigned int cmd_subs; ///< subscription handle for command stream
    unsigned int cw_subs;  ///< subscription handle for cw envelope stream (from CW unit)

    DatMBox * tx_stream;  ///< transmit audio stream 
    DatMBox * cw_env_stream; ///< envelope stream from text-to-CW converter (CW unit)
    CmdMBox * cmd_stream; ///< command stream
    
    bool tx_enabled; ///< if true, we're transmitting. 
    SoDa::Command::ModulationType tx_modulation; ///< type of transmit modulation (CW_U,CW_L,USB,LSB...)

    double CW_tone_freq;
    QuadratureOscillator CW_osc; ///< CW tone IQ oscillator

    float * beacon_env; ///< steady constant amplitude envelope
    bool beacon_mode;   ///< if true, we're transmitting a steady carrier

    float * zero_env; ///< envelope for dead silence

    std::complex<float> * cw_buf; ///< CW modulated envelope to send to USRP
    std::complex<float> * zero_buf; ///< zero signal envelope to fill in end of transmit stream
    std::complex<float> * tvtr_buf; ///< constant signal envelope for transverter LO

    double tx_sample_rate; ///< sample rate for buffer going to USRP (UHD)
    unsigned int tx_buffer_size; ///< size of buffer going to USRP
    float cw_env_amplitude;  ///< used to set CW output envelope, constant at 0.7
    
    bool waiting_to_run_dry; ///< When set, we should send out a report when we run out of CW buffer
    
    uhd::stream_args_t * stream_args;
    uhd::tx_streamer::sptr tx_bits; ///< USRP (UHD) transmit stream handle
    uhd::tx_metadata_t md; ///< metadata describing USRP transmit buffer

    // transverter support -- for B210 
    bool transverter_capable;     
  }; 

}


#endif
