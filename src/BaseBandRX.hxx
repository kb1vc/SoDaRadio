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
#include "Params.hxx"
#include "Buffer.hxx"
#include "MailBoxRegistry.hxx"
#include "MailBoxTypes.hxx"
#include "Command.hxx"
#include "OSFilter.hxx"
#include "HilbertTransformer.hxx"
#include "TDResamplers625x48.hxx"
#include "AudioIfc.hxx"
#include "MedianFilter.hxx"

#include <queue>
#include <mutex>
#include <fstream>
#include <string>

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
   * and finally demodulated.
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
  class BaseBandRX : public Thread {
  public:
    /**
     * @brief the constructor
     *
     * @param params command line parameter object
     * @param audio_ifc pointer to the audio output handler
     **/
    BaseBandRX(Params * params,
	       AudioIfc * audio_ifc);

    /// implement the subscription method
    void subscribe();
    
    /**
     * @brief the run method -- does the work of the audio receiver process
     */
    void run();

  private:
    /**
     * @brief execute GET commands from the command channel
     * @param cmd the incoming command
     */
    void execGetCommand(CmdMsg  cmd); 
    /**
     * @brief handle SET commands from the command channel
     * @param cmd the incoming command
     */
    void execSetCommand(CmdMsg  cmd); 
    /**
     * @brief handle Report commands from the command channel
     * @param cmd the incoming command
     */
    void execRepCommand(CmdMsg  cmd); 

    /**
     * @brief demodulate the input stream as an SSB signal
     * place the resulting audio buffer on the audio output queue.
     *
     * @param drxbuf downsampled  RF input buffer
     * @param mod modulation type -- LSB, USB, CW_U, or CW_R
     */
    void demodulateSSB(CFBuf & drxbuf,
		       Command::ModulationType mod); 

    /**
     * @brief demodulate the input stream as an amplitude modulated signal
     * place the resulting audio buffer on the audio output queue.
     *
     * @param drxbuf downsampled  RF input buffer
     */
    void demodulateAM(CFBuf & drxbuf);

    /**
     * @brief demodulate the input stream as a narrowband frequency modulated signal
     * place the resulting audio buffer on the audio output queue.
     *
     * @param drxbuf downsampled  RF input buffer
     * @param mod modulation type -- NBFM
     * @param af_gain factor to goose the audio output
     */
    void demodulateNBFM(CFBuf &  drxbuf,
			Command::ModulationType mod,
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
    void demodulateWBFM(CFBuf & rxbuf,
			Command::ModulationType mod,
			float af_gain);

    
    /**
     * @brief apply the currently selected demodulation scheme to the input RX buffer
     * place the resulting audio buffer on the audio output queue.
     *
     * @param rxbuf RF input buffer
     */
    void demodulate(CFBuf & rxbuf);

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

    Command::ModulationType rx_modulation; ///< current receive modulation mode (USB,LSB,CW_U,CW_L,NBFM,WBFM,AM,...)
    
    CFMBoxPtr rx_stream; ///< mailbox producing rx sample stream from USRP
    MsgMBoxPtr cmd_stream; ///< mailbox producing command stream from user
    CFSubs rx_subs; ///< mailbox subscription ID for rx data stream
    MsgSubs cmd_subs; ///< mailbox subscription ID for command stream

    AudioIfc * audio_ifc; ///< pointer to the audio interface (output) object

    /**
     * @brief put an audio buffer on the "pending for output" list
     *
     * @param b pointer to an audio buffer
     *
     */
    void pendAudioBuffer(FBuf b); 
    
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
    FBuf getNextAudioBuffer();
    /**
     * @brief empty the queue of pending audio buffers, we're going into TX mode.
     */
    void flushAudioBuffers();

    /**
     * @brief return number audio buffers available
     */
    int readyAudioBuffers(); 


    // flow timing management
    bool in_catchup;  ///< when true, the audio server has fallen behind...
    bool in_fallback;  ///< when true, the audio server has gotten ahead...
    unsigned int catchup_rand_mask; ///< a mask to use for fast selection of a random index into an audio buffer. 

    std::queue<FBuf> ready_buffers; ///< a list of audio buffers ready to send to the output

    std::mutex ready_mutex; ///< lock for the ready_buffers_pool

    FBuf sidetone_silence;  ///< a sequence of zero samples to stuff silence into the audio

    // resampler -- downsample from 625K samples / sec to 48K samples/sec
    TDResampler625x48<std::complex<float>> * rf_resampler; ///< downsample the RF input to 48KS/s
    // a second resampler for wideband fm
    TDResampler625x48<float>  * wbfm_resampler; ///< downsample the RF input to 48KS/s for WBFM unit

    /**
     * @brief build the audio filter map for selected bandwidths
     */
    void buildFilterMap();
    
    Command::AudioFilterBW af_filter_selection; ///< currently audio filter selector
    OSFilter * cur_audio_filter; ///< currently selected audio filter
    OSFilter * fm_audio_filter; ///< audio filter for FM (wider passband)
    OSFilter * am_pre_filter; ///< Before AM demod, we do some (6KHz) prefilter
    OSFilter * nbfm_pre_filter; ///< Before NBFM demod, we do some (15KHz) prefilter -- rf rate
    OSFilter * am_audio_filter; ///< After AM demod, we do a second filter

    
    
    std::map<Command::AudioFilterBW, OSFilter *> filter_map; ///< map filter selectors to the filter objects

    // hilbert transformer
    HilbertTransformer * hilbert; ///< hilbert transform object for SSB/CW widgets
    
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


