#pragma once
/*
Copyright (c) 2012,2013,2014,2022 Matthew H. Reilly (kb1vc)
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
#include "Thread.hxx"
#include "MailBoxTypes.hxx"
#include "Params.hxx"
#include "Command.hxx"
#include <SoDa/ReSampler.hxx>
#include <SoDa/OSFilter.hxx>
#include "AudioIfc.hxx"

namespace SoDa {
  /**
   * BaseBandTX -- this is the audio processing chain for the transmit path
   *
   * @image html SoDa_Radio_TX_Signal_Path.svg
   *
   * The BaseBandTX unit creates I/Q audio streams for LSB, SSB, FM, and AM modulation.
   * CW (CW_L and CW_U) modes are implemented in the USRPTX module and the CW unit.
   *
   */
  class BaseBandRX;  
  class BaseBandTX : public SoDa::Thread {
  public:
    /**
     * constructor
     *
     * @param params command line parameter object
     * @param resampler pointer to a resampler from the audio stream to the TX RF stream
     * @param audio_ifc pointer to the audio output handler
     */
    BaseBandTX(Params_p params,
	       ReSampler * resampler, 
	       AudioIfc * audio_ifc
	       );

    /// implement the subscription method
    void subscribe();    

    /**
     * @brief the run method -- does the work of the audio transmitter process
     */
    void run();
  private:
    /**
     * @brief execute GET commands from the command channel
     * @param cmd the incoming command
     */
    void execGetCommand(std::shared_ptr<Command> cmd); 
    /**
     * @brief handle SET commands from the command channel
     * @param cmd the incoming command
     */
    void execSetCommand(std::shared_ptr<Command> cmd); 
    /**
     * @brief handle Report commands from the command channel
     * @param cmd the incoming command
     */
    void execRepCommand(std::shared_ptr<Command> cmd); 

    /**
     * @brief create an AM/SSB modulation envelope
     *
     * @param audio_buf the buffer of modulating audio info
     * @param tx_mode is AM, USB, or LSB
     */
    SoDa::CFBuf  modulateAM(FBuf audio_buf, Command::ModulationType tx_mode);

    /**
     * @brief create a narrowband/wideband FM modulation envelope
     *
     * @param audio_buf the buffer of modulating audio info
     * @param deviation the phase shift per audio sample for a maximum amplitude (1.0) input.
     *
     * Note that this modulator varies the mic gain to prevent over-deviation. 
     */
    SoDa::CFBuf  modulateFM(FBuf audio_buf, double deviation);
    double fm_phase;
    double nbfm_deviation; ///< phase advance for 2.5kHz deviation.
    double wbfm_deviation; ///< phase advance for 75kHz deviation
    double fm_mic_gain; ///< separate gain control for FM deviation....

    
    CFMBoxPtr tx_stream; ///< outbound RF stream to USRPTX transmit chain
    MsgMBoxPtr cmd_stream; ///< command stream from UI and other units
    MsgSubs cmd_subs; ///< subscription ID for command stream
    
    // The interpolator
    SoDa::ReSampler * tx_resampler;  ///< Upsample from 48KHz to 625KHz

    // parameters
    unsigned int audio_buffer_size; ///< length (in samples) of an input audio buffer
    double audio_sample_rate; ///< wired in lots of places as 48KHz
    unsigned int tx_buffer_size;  ///< how long is the outbound RF buffer

    SoDa::Command::ModulationType tx_mode; ///< what modulation scheme? USB? LSB? CW_U?...
    bool cw_tx_mode; ///< if true, tx_mode is CW_L or CW_U
    bool tx_on; ///< set by Command::TX_STATE to on or off
    

    float af_gain; ///< local microphone gain. 

    // audio server state
    AudioIfc * audio_ifc; ///< pointer to an AudioIfc object for the microphone input

    bool tx_stream_on; ///< if true, we are transmitting. 

    // we need some intermediate storage for things like
    // the IQ buffer
    std::vector<std::complex<float>> audio_IQ_buf; ///< temporary storage for outbound modulation envelope

    /** 
     * TX audio filter.  We use USB/LSB filters to suppress the carrier and
     * alternate sideband.  Hilbert trannsformers look good in the book, but
     * their bad behavior around DC makes for mediocre supression. 
     */
    SoDa::OSFilter * tx_usb_audio_filter;
    SoDa::OSFilter * tx_lsb_audio_filter;    


    /**
     * mic gain is adjustable, to make sure we aren't noxious.
     */
    float mic_gain;

    bool debug_mode; ///< if true, print extra debug info
    int debug_ctr; 
  }; 
}
