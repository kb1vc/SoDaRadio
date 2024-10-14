/*
Copyright (c) 2012,2013,2014,2023, 2024 Matthew H. Reilly (kb1vc)
All rights reserved.

  FM modulator features based on code contributed by and 
  Copyright (c) 2014, Aaron Yankey Antwi (aaronyan2001@gmail.com)

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
#include "MultiMBox.hxx"
#include "Params.hxx"
#include "Command.hxx"
#include "ReSamplers625x48.hxx"
#include "HilbertTransformer.hxx"
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
  class BaseBandTX : public Thread {
  public:
    /**
     * constructor
     *
     * @param params command line parameter object
     * @param audio_ifc pointer to the audio output handler
     */
    BaseBandTX(Params * params,
	       AudioIfc * audio_ifc
	       );

    /// implement the subscription method
    void subscribeToMailBoxList(MailBoxMap & mailboxes);

    /**
     * @brief the run method -- does the work of the audio transmitter process
     */
    void run();
  private:

    FVecPtr getBuffer(unsigned int size);
    
    /**
     * @brief execute GET commands from the command channel
     * @param cmd the incoming command
     */
    void execGetCommand(CommandPtr  cmd); 
    /**
     * @brief handle SET commands from the command channel
     * @param cmd the incoming command
     */
    void execSetCommand(CommandPtr  cmd); 
    /**
     * @brief handle Report commands from the command channel
     * @param cmd the incoming command
     */
    void execRepCommand(CommandPtr  cmd); 

    /**
     * @brief create an AM/SSB modulation envelope
     *
     * @param audio_buf the buffer of modulating audio info
     * @param len the length of the audio buffer
     * @param is_usb if true, generate upper sideband
     * @param is_lsb if true, generate lower sideband
     * if both is_usb and is_lsb are false, the modulator
     * creates an IQ stream that is amplitude modulated
     */
    BufPtr modulateAM(FVecPtr  audio_buf, unsigned int len, bool is_usb, bool is_lsb); 

    /**
     * @brief create a narrowband/wideband FM modulation envelope
     *
     * @param audio_buf the buffer of modulating audio info
     * @param len the length of the audio buffer
     * @param deviation the phase shift per audio sample for a maximum amplitude (1.0) input.
     *
     * Note that this modulator varies the mic gain to prevent over-deviation. 
     */
    BufPtr modulateFM(FVecPtr  audio_buf, unsigned int len, double deviation);
    double fm_phase;
    double nbfm_deviation; ///< phase advance for 2.5kHz deviation.
    double wbfm_deviation; ///< phase advance for 75kHz deviation
    double fm_mic_gain; ///< separate gain control for FM deviation....

    
    DatMBox * tx_stream; ///< outbound RF stream to USRPTX transmit chain
    CmdMBox * cmd_stream; ///< command stream from UI and other units
    
    // The interpolator
    ReSample48to625 * interpolator;  ///< Upsample from 48KHz to 625KHz

    // parameters
    unsigned int audio_buffer_size; ///< length (in samples) of an input audio buffer
    double audio_sample_rate; ///< wired in lots of places as 48KHz
    unsigned int tx_buffer_size;  ///< how long is the outbound RF buffer

    Command::ModulationType tx_mode; ///< what modulation scheme? USB? LSB? CW_U?...
    bool cw_tx_mode; ///< if true, tx_mode is CW_L or CW_U
    bool tx_on; ///< set by Command::TX_STATE to on or off
    

    float af_gain; ///< local microphone gain. 

    // audio server state
    AudioIfc * audio_ifc; ///< pointer to an AudioIfc object for the microphone input

    bool tx_stream_on; ///< if true, we are transmitting. 

    // we need some intermediate storage for things like
    // the IQ buffer
    std::complex<float> * audio_IQ_buf; ///< temporary storage for outbound modulation envelope

    /**
     * SSB modulation requires that we upsample before
     * doing the quadrature generation.
     */
    FVecPtr  ssb_af_upsample; 

    /**
     * This is a buffer that holds a set of "noise" samples (uniform random)
     * for testing the TX audio chain.
     */
    FVecPtr  noise_buffer; 

    /**
     * When this is TRUE, audio modes (USB,LSB,AM,NBFM,WBFM) use a noise source for
     * input. 
     */
    bool tx_noise_source_ena; 

    /**
     * There is an audio filter in the chain, by default.  This is the enable.
     */
    bool tx_audio_filter_ena; 
    
    /** 
     * TX audio filter
     */
    OSFilter * tx_audio_filter;

    /**
     *The hilbert transformer to create an analytic (I/Q) signal.
     */
    HilbertTransformer * hilbert;

    /**
     * mic gain is adjustable, to make sure we aren't noxious.
     */
    float mic_gain;

    bool debug_mode; ///< if true, print extra debug info
    int debug_ctr; 
  }; 
}

