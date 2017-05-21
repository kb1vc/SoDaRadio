/*
Copyright (c) 2017 Matthew H. Reilly (kb1vc)
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

#ifndef SoapyTX_HDR
#define SoapyTX_HDR
#include "SoDaBase.hxx"
#include "MultiMBox.hxx"
#include "Command.hxx"
#include "Params.hxx"
#include "QuadratureOscillator.hxx"
#include "SoapyCtrl.hxx"
#include "Histogram.hxx"


#include <SoapySDR/Device.hpp>
#include <SoapySDR/Types.hpp>


namespace SoDa {
  /**
   * The Transmit RF Path
   *
   * @image html SoDa_Radio_TX_Signal_Path.svg
   *
   * In SSB/AM/FM modes, the SoapyTX unit accepts an I/Q audio
   * stream from the BaseBandTX unit and forwards it to the USRP.
   * In CW mode, the SoapyTX unit impresses a CW envelope (received
   * from the CW unit) onto a carrier and passes this to the USRP. 
   *
   */
  class SoapyTX : public SoDaThread {
  public:
    /**
     * @brief Constructor for RF Transmit/modulator process
     *
     * @param params block describing intial setup of the radio
     * @param _radio SoapySDR handle for the radio
     * @param _tx_stream audio transmit stream to be used in modulator
     * @param _cw_env_stream envelope stream from text-to-CW converter
     * @param _cmd_stream command stream
     *
     */
    SoapyTX(Params * params, SoDa::SoapyCtrl * _ctrl, 
	    DatMBox * _tx_stream, DatMBox * _cw_env_stream,
	    CmdMBox * _cmd_stream);
    /**
     * @brief SoapyTX run loop: handle commands, and modulate the tx carrier
     */
    void run(); 

  private:

    // the controller
    SoDa::SoapyCtrl * ctrl; 
    

    SoapySDR::Device * radio; ///< the radio.
    
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

  protected:

    /**
     * @brief send a buffer to the transmit streamer. (block on busy)
     * 
     * @param buf pointer to an IQ buffer
     * @param len length of the buffer in samples
     * @param end_burst iff true, mark this as the last buffer in a burst. 
     * @return number of elements sent, or < 0 for error
     */
    int sendBuffer(std::complex<float> * buf, size_t len, bool end_burst = false); 

    /**
     * @brief check the tx stream status and look for an indication that the
     * transmitter has processed the buffer marked with an EndOfBurst flag. 
     * 
     * @param timeout_us return false if no indication is received in timeout_us microseconds
     * @return true if EOB was found, false otherwise.
     */
    bool lookForEOB(unsigned long timeout_us); 

    /**
     * @brief send an EOB marker into the radio and wait for it to make it to
     * the hardware transmit stream.  Then return. 
     */
    bool drainTXStream(); 

    unsigned int tx_subs;  ///< subscription handle for transmit audio stream (from BaseBandTX)
    unsigned int cmd_subs; ///< subscription handle for command stream
    unsigned int cw_subs;  ///< subscription handle for cw envelope stream (from CW unit)

    DatMBox * tx_stream;  ///< transmit audio stream 
    DatMBox * cw_env_stream; ///< envelope stream from text-to-CW converter (CW unit)
    CmdMBox * cmd_stream; ///< command stream
    
    bool tx_enabled; ///< if true, we're transmitting. 
    bool tx_activated; ///< if true, we've activated the stream for the first time.
    SoDa::Command::ModulationType tx_modulation; ///< type of transmit modulation (CW_U,CW_L,USB,LSB...)

    double CW_tone_freq;
    QuadratureOscillator CW_osc; ///< CW tone IQ oscillator

    float * beacon_env; ///< steady constant amplitude envelope
    bool beacon_mode;   ///< if true, we're transmitting a steady carrier

    float * zero_env; ///< envelope for dead silence

    std::complex<float> * cw_buf; ///< CW modulated envelope to send to radio
    std::complex<float> * zero_buf; ///< zero signal envelope to fill in end of transmit stream

    double tx_sample_rate; ///< sample rate for buffer going to radio
    unsigned int tx_buffer_size; ///< size of buffer going to radio
    float cw_env_amplitude;  ///< used to set CW output envelope, constant at 0.7

    double seconds_per_sample; ///< time per sample. 
    long  samples_in_flight; ///< the number of samples currently "in flight"
    long in_flight_limit; ///< number of samples we want to have "in flight"

    bool waiting_to_run_dry; ///< When set, we should send out a report when we run out of CW buffer
    
    SoapySDR::Stream * tx_bits; ///< radio transmit stream handle
    bool first_burst; 

    // transverter local oscillator support
    bool LO_enabled; ///< if true, we're in local transverter mode
    bool LO_configured; ///< if true, the LO has had its gain/freq set.
    bool LO_capable; ///< if true, this hardware model supports LO config

    std::complex<float> * const_buf; ///< envelope for dead silence

    SoDa::Histogram * send_histo;
    SoDa::Histogram * write_stream_histo;
  }; 

}


#endif
