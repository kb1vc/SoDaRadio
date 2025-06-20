/*
Copyright (c) 2012,2013,2014,2025 Matthew H. Reilly (kb1vc)
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
#include "Params.hxx"

#include "Command.hxx"
#include "HilbertTransformer.hxx"
#include "AudioIfc.hxx"
#include "MedianFilter.hxx"

#include <SoDa/ReSampler.hxx>
#include <SoDa/OSFilter.hxx>
#include <SoDa/MailBox.hxx>

#include <queue>
#include <mutex>
#include <fstream>
#include <string>
#include <memory>

namespace SoDa {
  /**
   * BaseBandRX -- this is the audio processing chain for the recieve path
   *
   * @image html SoDa_Radio_RX_Signal_Path.svg
   *
   * The BaseBandRX unit accepts the 3rd IF (nominally 80kHz to 200kHz)
   * from the USRPRX thread on the rx_stream.
   *
   * In most cases (all but Wide Band FM) the rx stream is downselected
   * by a 625 to 48 resampler before being passed through an audio filter
   * and finally demodulated.  (Some radios require a different RF sample
   * rate -- Ettus N3xx for instance can't go that low, and some non USRP
   * radios use sample rates that have nothing to do with 10 MHz. In all
   * cases, we ask the parameter unit for the RF sample rate, and
   * setup about 50 ms of buffer space. 
   * 
   *
   * As each buffer/timeslice is demodulated, it
   * is placed on a queue of outbound audio blocks for the host processor's
   * audio system. Since there is some slop in the timing (specifically, the
   * clock governing the radio is not necessarily in sync with the audio
   * system clock) the BaseBandRX unit monitors the backlog of outbound audio
   * buffers.  When this backlog gets longer than about 400mS, the unit will
   * "drop out" a sample here and there until the downstream sound system
   * catches up.
   *
   * BaseBandRX supports CW_U (upper sideband CW), CW_L (lower sideband CW),
   * USB, and LSB modulation via the phasing method, since both I and Q
   * channels are available. AM is performed with a simple magnitude detector.
   */

  class BaseBandRX;
  typedef std::shared_ptr<BaseBandRX> BaseBandRXPtr;
  typedef std::weak_ptr<BaseBandRX> BaseBandRXWeakPtr;
  
  class BaseBandRX : public SoDa::Thread {
  private:
    
    /**
     * @brief the constructor
     *
     * @param params command line parameter object
     * @param audio_ifc pointer to the audio output handler
     **/
    BaseBandRX(ParamsPtr params,
	       AudioIfcPtr audio_ifc);

  public:
    /**
     * @brief the maker -- produces a shared pointer
     *p
     * @param params command line parameter object
     * @param audio_ifc pointer to the audio output handler
     **/
    static BaseBandRXPtr make(ParamsPtr params, AudioIfcPtr audio_ifc) {
      auto ret = std::shared_ptr<BaseBandRX>(new BaseBandRX(params, audio_ifc));
      ret->self = ret; 
      ret->registerThread(ret);
      return ret; 
    }

    
    /// implement the subscription method
    void subscribeToMailBoxes(const std::vector<MailBoxBasePtr> & mailboxes);
    
    /**
     * @brief the run method -- does the work of the audio receiver process
     */
    void run();

  private:
    /**
     * @brief execute GET commands from the command channel
     * @param cmd the incoming command
     */
    void execGetCommand(CommandPtr cmd); 
    /**
     * @brief handle SET commands from the command channel
     * @param cmd the incoming command
     */
    void execSetCommand(CommandPtr cmd); 
    /**
     * @brief handle Report commands from the command channel
     * @param cmd the incoming command
     */
    void execRepCommand(CommandPtr cmd); 

    /**
     * @brief demodulate the input stream as an SSB signal
     * place the resulting audio buffer on the audio output queue.
     *
     * @param drxbuf downsampled  RF input buffer
     * @param mod modulation type -- LSB, USB, CW_U, or CW_R
     */
    void demodulateSSB(SoDa::BufPtr drxbuf,
		       SoDa::Command::ModulationType mod); 

    /**
     * @brief demodulate the input stream as an amplitude modulated signal
     * place the resulting audio buffer on the audio output queue.
     *
     * @param drxbuf downsampled  RF input buffer
     */
    void demodulateAM(SoDa::BufPtr drxbuf);

    /**
     * @brief demodulate the input stream as a narrowband frequency modulated signal
     * place the resulting audio buffer on the audio output queue.
     *
     * @param drxbuf downsampled  RF input buffer
     * @param mod modulation type -- NBFM
     * @param af_gain factor to goose the audio output
     */
    void demodulateNBFM(SoDa::BufPtr drxbuf,
			SoDa::Command::ModulationType mod,
			float af_gain); 

    /**
     * @brief demodulate the input stream as a wideband frequency modulated signal
     * place the resulting audio buffer on the audio output queue.
     *
     * Note the wideband FM unit takes the raw RX buffer rather than the downsampled
     * rx buffer.   
     *
     * @param rxbuf RF input buffer
     * @param mod modulation type -- WBFM
     * @param af_gain factor to goose the audio output
     */
    void demodulateWBFM(SoDa::BufPtr rxbuf,
			SoDa::Command::ModulationType mod,
			float af_gain);

    
    /**
     * @brief apply the currently selected demodulation scheme to the input RX buffer
     * place the resulting audio buffer on the audio output queue.
     *
     * @param rxbuf RF input buffer
     */
    void demodulate(SoDa::BufPtr rxbuf);

    /**
     * @brief send a report of the lower and upper edges of the IF passband
     * based on the current filter and modulation type.
     */
    void repAFFilterShape();

    // parameters
    unsigned int audio_buffer_size; ///< size of output audio buffer chunk
    unsigned int rf_buffer_size; ///< size of input RF buffer chunk
    double audio_sample_rate; ///< sample rate of audio output -- assumed 48KHz
    double rf_sample_rate; ///< sample rate of RF input from USRP -- assumed 625KHz
    bool audio_rx_stream_enabled; ///< if true, send pending audio buffers to output
    bool audio_rx_stream_needs_start; ///< if true, the audio output device needs a wakeup
    bool sidetone_stream_enabled;  ///< if true, send CW sidetone to audio output

    SoDa::Command::ModulationType rx_modulation; ///< current receive modulation mode (USB,LSB,CW_U,CW_L,NBFM,WBFM,AM,...)
    
    DatMBoxPtr rx_stream; ///< mailbox producing rx sample stream from USRP
    CmdMBoxPtr cmd_stream; ///< mailbox producing command stream from user
    DatMBox::Subscription rx_subs; ///< mailbox subscription ID for rx data stream
    CmdMBox::Subscription cmd_subs; ///< mailbox subscription ID for command stream

    AudioIfcPtr audio_ifc; ///< pointer to the audio interface (output) object
    
    // buffer pool management

    /**
     * @brief put an audio buffer on the "pending for output" list
     *
     * @param b pointer to an audio buffer
     *
     */
    void pendAudioBuffer(SoDa::BufPtr b); 
    
    /**
     * @brief put an empty (zero signal) audio buffer on the pending for output list
     *
     * @param count number of buffers to pend onto the list.
     */ 
    void pendNullBuffer(int count = 1);
    
    /**
     * @brief return the next queued audio buffer to pass to the audio output device
     *
     * @return a pointer to the next buffer in sequence
     */
    float * getNextAudioBuffer();
    /**
     * @brief empty the queue of pending audio buffers, we're going into TX mode.
     */
    void flushAudioBuffers();
    
    // flow timing management
    bool in_catchup;  ///< when true, the audio server has fallen behind...
    bool in_fallback;  ///< when true, the audio server has gotten ahead...
    unsigned int catchup_rand_mask; ///< a mask to use for fast selection of a random index into an audio buffer. 

    SoDa::BufPtr sidetone_silence;  ///< a sequence of zero samples to stuff silence into the audio

    // resampler -- downsample from 625K samples / sec to 48K samples/sec
    SoDa::ReSamplerPtr rf_resampler; ///< downsample the RF input to 48KS/s
    // a second resampler for wideband fm    
    SoDa::ReSamplerPtr wbfm_resampler; ///< downsample the RF input to 48KS/s for WBFM unit 

    /**
     * @brief build the audio filter map for selected bandwidths
     */
    void buildFilterMap();
    
    SoDa::Command::AudioFilterBW af_filter_selection; ///< currently audio filter selector
    SoDa::OSFilterPtr cur_audio_filter; ///< currently selected audio filter
    SoDa::OSFilterPtr fm_audio_filter; ///< audio filter for FM (wider passband)
    SoDa::OSFilterPtr am_pre_filter; ///< Before AM demod, we do some (6KHz) prefilter
    SoDa::OSFilterPtr nbfm_pre_filter; ///< Before NBFM demod, we do some (15KHz) prefilter -- rf rate
    SoDa::OSFilterPtr am_audio_filter; ///< After AM demod, we do a second filter

    
    
    std::map<SoDa::Command::AudioFilterBW, SoDa::OSFilterPtr> filter_map; ///< map filter selectors to the filter objects

    // hilbert transformer
    SoDa::HilbertTransformerPtr hilbert; ///< hilbert transform object for SSB/CW widgets
    
    // audio gain
    float af_gain;   ///< audio gain setting for RX mode
    float af_sidetone_gain; ///< audio gain setting for TX/CW mode
    float *cur_af_gain; ///< pointer to the gain setting for this mode

    // support for NBFM/WBFM demodulator
    float last_phase_samp; ///< history value used to calculate dPhase/dt in FM atan based discriminator.

    // median filter for FM demods
    MedianFilter3<float> fmMedianFilter; ///< simple 3 point median filter for FM units
    
    // debug helper
    unsigned int dbg_ctr; ///< debug counter, used to support one-time or infrequent bulletins
    std::ofstream dbg_out;

    // audio output file
    bool audio_save_enable; 
    std::ofstream audio_file;
    std::ofstream audio_file2;     

    // recent audio level
    float audio_level; 
    float log_audio_buffer_size; 

    float nbfm_squelch_level;  ///< average amplitude must be greater to trigger demod.
    int nbfm_squelch_hang_count; ///< if > 0 then send audio.
    int nbfm_squelch_hang_time; ///< continue FM audio demod for this many frames after threshold drops
  };
}

