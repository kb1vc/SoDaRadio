/*
  Copyright (c) 2022, Matthew H. Reilly (kb1vc)
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

#include "BaseBandTX.hxx"
#include "Buffer.hxx"
#include "MailBoxTypes.hxx"
#include "MailBoxRegistry.hxx"
#include "Radio.hxx"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <SoDa/OSFilter.hxx>
#include <SoDa/ReSampler.hxx>
#include <cstdlib>
#include <math.h>
#include "MacFixes.hxx"

namespace SoDa {
  BaseBandTX::BaseBandTX(Params & params, 
			 ReSampler * resampler, 
			 AudioIfc * _audio_ifc
			 ) : tx_resampler(resampler), Thread("BaseBandTX")
  {
    debug_mode = false;
    debug_ctr = 0;
    tx_stream = NULL;

    cmd_stream = NULL;

    audio_buffer_size = params.getTXAFBufferSize();
    tx_buffer_size = params.getTXRFBufferSize();

    audio_IQ_buf.resize(audio_buffer_size);
    
    // create the audio stream.
    // borrow the stream from the BaseBandRX side. ? 
    //   pa_stream = _pa_stream;
    double srate = params.getAudioSampleRate();

    // now setup the FM deviation
    nbfm_deviation = 2.0 * M_PI * 2.5e3 / srate; // 2.5 kHz max deviation
    wbfm_deviation = 2.0 * M_PI * 75.0e3 / srate; // 75 kHz max deviation
    fm_phase = 0.0; 

    audio_ifc = _audio_ifc; 

    tx_stream_on = false;

    // setup the audio filter for the tx audio input...
    tx_usb_audio_filter = new OSFilter(150, 2300, 200, srate, audio_buffer_size); 

    tx_lsb_audio_filter = new OSFilter(-150, -2300, 200, srate, audio_buffer_size); 
    
    mic_gain = 0.8;  //  was ... 0.4;

    fm_mic_gain = 0.8;
  }

  void BaseBandTX::run()
  {
    bool exitflag = false;
    std::shared_ptr<Command> cmd; 
    FBuf audio_buf = makeFBuf(audio_buffer_size); 


    if((cmd_stream == NULL) || (tx_stream == NULL)) {
      throw Radio::Exception(std::string("Missing a stream connection.\n"),
			     this);	
    }
  
    /**
     * This is the BaseBandTX run loop.
     * \code
     *     while true
     *       if(commands_available) 
     *          handle commands
     *       else if(in CW transmit mode) 
     *          sleep 1mS
     *       else 
     *          if ((tx ON) and audio is ready)
     *            get audio buffer
     *            modulate with audio buffer
     *            put modulation envelope in outbound queue
     *       sleep 1mS
     * \endcode
     */

    // first wake up the audio channel
    audio_ifc->wakeIn();  

    while(!exitflag) {
      if((cmd = cmd_stream->get(cmd_subs)) != NULL) {
	// process the command.
	execCommand(cmd);
	exitflag |= (cmd->target == Command::STOP); 
      }
      else if (!tx_stream_on || cw_tx_mode) {
	// read audio information and throw it away. 	  	
	audio_ifc->recv(audio_buf->data(), audio_buffer_size, true);
	usleep(1000); 
      }
      else {
	// If we're in TX mode that isn't CW....
	// get an input audio buffer.
	if (tx_stream_on && audio_ifc->recv(audio_buf->data(), audio_buffer_size, true)) { 
	  CFBuf txbuf = makeCFBuf(tx_buffer_size);

	  switch (tx_mode) {
	  case Command::USB:
	  case Command::LSB:
	  case Command::AM:
	    txbuf = modulateAM(audio_buf, tx_mode);
	    break; 
	  case Command::NBFM:
	    txbuf = modulateFM(audio_buf, nbfm_deviation);
	    break; 
	  case Command::WBFM:
	    txbuf = modulateFM(audio_buf, wbfm_deviation);
	    break;
	  }

	  if(txbuf != NULL) {
	    tx_stream->put(txbuf); 
	  }
	}      
	usleep(1000); 
      }
    }
  }

  CFBuf  BaseBandTX::modulateAM(FBuf audio_buf,
				Command::ModulationType tx_mode)
  {
    // Once upon a time  we used a hilbert transformer. That was a bad
    // idea because the transformer wasn't all that good around DC, making
    // the carrier and sideband supression pretty sketchy. We'll use a
    // bandpass filter here, applied to the synthetic complex buffer.
    for(int i = 0; i < audio_IQ_buf.size(); i++) {
      audio_IQ_buf[i] = std::complex<float>((*audio_buf)[i] * mic_gain, 0.0);
    }
  
    // now we apply the appropriate filter
    switch (tx_mode) {
    case Command::USB:
      tx_usb_audio_filter->apply(audio_IQ_buf, audio_IQ_buf);
      break; 
    case Command::LSB:
      tx_lsb_audio_filter->apply(audio_IQ_buf, audio_IQ_buf);
      break; 
    default:
      // happy with what we have. 
	break; 
    }

    /// Now that the I/Q channels have been populated, get a transmit buffer. 
    /// and upsample the I/Q audio up to the RF rate.
    CFBuf txbuf = makeCFBuf(tx_buffer_size);
  
    /// Upsample the IQ audio (at 48KS/s) to the RF sample rate of 625 KS/s
    tx_resampler->apply(audio_IQ_buf, *txbuf); 

    /// pass the newly created and filled buffer back to the caller
    return txbuf; 
  }


  CFBuf  BaseBandTX::modulateFM(FBuf audio_buf, double deviation)
  {
    unsigned int i;
  
    for(i=0; i < audio_buffer_size; i++) {
      double audio_amp = (*audio_buf)[i] * fm_mic_gain;

      // apply a little clipping here.. better to sound
      // bad than to bleed into the neighboring channel.
      if(fabs(audio_amp) > 1.0) {
	audio_amp = (audio_amp > 0.0) ? 1.0 : -1.0; 
      }
      
      fm_phase += deviation * audio_amp; 

      while (fm_phase > (float)(M_PI))
	fm_phase -= (float)(2.0 * M_PI);
      while (fm_phase < (float)(-M_PI))
	fm_phase += (float)(2.0 * M_PI);

      double oq, oi; 
      sincos(fm_phase, &oi, &oq);
      audio_IQ_buf[i] = std::complex<float>(oi,oq);
    }
 
    CFBuf txbuf = makeCFBuf(tx_buffer_size);

    // Upsample the IQ audio (at 48KS/s) to the RF sample rate of 625 KS/s
    tx_resampler->apply(audio_IQ_buf, *txbuf);

    // Pass the newly created and filled buffer back to the caller
    return txbuf;
  }


  void BaseBandTX::execSetCommand(std::shared_ptr<Command> cmd)
  {

    switch (cmd->target) {
    case Command::TX_MODE:
      // we only care about CW mode or NOT CW mode.
      tx_mode = Command::ModulationType(cmd->iparms[0]);
      if((tx_mode == Command::CW_L) || (tx_mode == Command::CW_U)) {
	cw_tx_mode = true; 
      }
      else {
	cw_tx_mode = false; 
      }
      break; 
    case Command::TX_STATE: // SET TX_ON
      // transition from RX to TX
      if(cmd->iparms[0] == 3) {
	tx_on = true;
	if(!cw_tx_mode) {
	  // Wake up the audio interface. 
	    // actually, never need to do this, as we never let it sleep
	    // audio_ifc->wakeIn();
	    tx_stream_on = true; 
	}
      }

      // transition from TX to RX
      if(cmd->iparms[0] == 0) {
	tx_on = false;
	if(tx_stream_on) {
	  // Put the audio interface to sleep
	  // and flush the input buffer	
	    // Actually, never put the audio interface to sleep. 
	    // always read from the input buffer. 
	    // audio_ifc->sleepIn(); 
	  tx_stream_on = false; 
	}
      }
      break;
    case Command::TX_AF_GAIN: // set audio gain.
      // audio gain is passed around as linear (in dB), but
      // gets converted before we set the envelope power.
      af_gain = powf(10.0, 0.1 * (cmd->dparms[0] - 50.0));
      cmd_stream->put(Command::make(Command::REP, Command::TX_AF_GAIN, 
				    50 + 10.0 * log10(af_gain)));
      break; 
    case Command::TX_AUDIO_IN:
      debugMsg("TX Audio IN is MIC.\n");
      break;
    default:
      break; 
    }
  }

  void BaseBandTX::execGetCommand(std::shared_ptr<Command> cmd)
  {
    (void) cmd; 
  }

  void BaseBandTX::execRepCommand(std::shared_ptr<Command> cmd)
  {
    (void) cmd; 
  }

  /// implement the subscription method
  void BaseBandTX::subscribe()
  {
    auto reg = MailBoxRegistry::getRegistrar();

    cmd_stream = MailBoxBase::convert<MsgMBox>(reg->get("CMD"));
    cmd_subs = cmd_stream->subscribe();

    tx_stream = MailBoxBase::convert<CFMBox>(reg->get("TX"));
  }
}
