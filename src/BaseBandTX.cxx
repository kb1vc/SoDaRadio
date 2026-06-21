/*
  Copyright (c) 2012, 2025, 2026 Matthew H. Reilly (kb1vc)
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

#include "BaseBandTX.hxx"
#include "BaseBandRX.hxx"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <cstdlib>
#include <math.h>

#include <memory>

// MacOS doesn't provide "sincos" but it does provide something by another name.
#if defined(__APPLE__)
#define sincos(a, b, c) __sincos(a, b, c)
#endif

namespace SoDa {
  BaseBandTX::BaseBandTX(ParamsPtr params, 
			 AudioIfcPtr _audio_ifc
			 ) : Thread("BaseBandTX")
  {
    debug_mode = false;
    debug_ctr = 0;
    tx_stream =nullptr;

    cmd_stream =nullptr;

    audio_sample_rate = params->getAudioSampleRate();
    auto rf_sample_rate = params->getRXRate();
  
    // create the interpolator.
    // ts=0.063 gives in=2304, out=30000 (2304 * 625/48 = 30000, the original design)
    interpolator = ReSampler::make(audio_sample_rate, rf_sample_rate, 0.063f);

    audio_buffer_size = interpolator->getInputBufferSize();
    tx_buffer_size    = interpolator->getOutputBufferSize();

    // now setup the FM deviation
    nbfm_deviation = 2.0 * M_PI * 2.5e3 / audio_sample_rate; // 2.5 kHz max deviation
    wbfm_deviation = 2.0 * M_PI * 75.0e3 / audio_sample_rate; // 75 kHz max deviation
    fm_phase = 0.0; 

    audio_ifc = _audio_ifc; 

    tx_stream_on = false;

    // create the IQ buffer -- must match interpolator->getInputBufferSize() exactly.
    audio_IQ_buf.resize(audio_buffer_size);

    // create the Hilbert transformer
    hilbert = HilbertTransformer::make(audio_buffer_size);

    // create the noise buffer
    noise_buffer.resize(audio_buffer_size);
    // and initialize it.
    // use the same random init every time -- counterintuitive, but 
    // I want this to be "repeatable" noise.
    srandom(0x92314159);
    for(auto & nb : noise_buffer) {
      float rfl = ((float) random()) / ((float) RAND_MAX);
      rfl = rfl - 0.5; // make the mean 0
      nb = rfl * 0.5; 
    }
  
    // clear the noise enable 
    tx_noise_source_ena = false; 

    // setup the audio filter for the tx audio input...
    tx_audio_filter = OSFilter::make(150.0, 2300.0,
				     100,
				     audio_sample_rate, audio_buffer_size);

    // enable the audio filter by default.
    tx_audio_filter_ena = true; 

    mic_gain = 0.8;

    fm_mic_gain = 0.8;

    // af_gain starts at unity; the GUI adjusts it via TX_AF_GAIN.
    af_gain = 1.0;
  }

  void BaseBandTX::run()
  {
    bool exitflag = false;
    CommandPtr cmd; 
    std::vector<float> audio_buf(audio_buffer_size);

    if((cmd_stream ==nullptr) || (tx_stream ==nullptr)) {
      throw SDR::Exception(std::string("Missing a stream connection.\n"),
			     getSelfPtr());	
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

    unsigned int bbtx_put_count = 0;
    unsigned int bbtx_recv_count = 0;

    while(!exitflag) {
      if(cmd_stream->get(cmd_subs, cmd)) {
	// process the command.
	execCommand(cmd);
	exitflag |= (cmd->target == Command::STOP);
      }
      else if (!tx_stream_on || cw_tx_mode) {
	// read audio information and throw it away.
	audio_ifc->recv(audio_buf, true);
	usleep(1000);
      }
      else {
	// If we're in TX mode that isn't CW....
	// get an input audio buffer.
	int recv_ret = audio_ifc->recv(audio_buf, true);
	if(bbtx_recv_count < 5) {
	  std::cerr << SoDa::Format("BaseBandTX: recv ret=%0 tx_mode=%1 tx_stream_on=%2\n")
	    .addI(recv_ret).addI((int)tx_mode).addI((int)tx_stream_on);
	}
	if (tx_stream_on && recv_ret) {
	  CBufPtr txbuf =nullptr;
	  std::vector<float> * audio_tx_buffer = & audio_buf;

	  if(tx_noise_source_ena) {
	    audio_tx_buffer = &noise_buffer;
	  }

	  if(tx_audio_filter_ena) {
	    tx_audio_filter->apply(*audio_tx_buffer, audio_buf);
	    audio_tx_buffer = & audio_buf;
	  }

	  if(tx_mode == Command::USB) {
	    txbuf = modulateAM(*audio_tx_buffer, true, false);
	  }
	  else if(tx_mode == Command::LSB) {
	    txbuf = modulateAM(*audio_tx_buffer, false, true);
	  }
	  else if(tx_mode == Command::AM) {
	    txbuf = modulateAM(*audio_tx_buffer, false, false);
	  }
	  else if(tx_mode == Command::NBFM) {
	    txbuf = modulateFM(*audio_tx_buffer, nbfm_deviation);
	  }
	  else if(tx_mode == Command::WBFM) {
	    txbuf = modulateFM(*audio_tx_buffer, wbfm_deviation);
	  }
	  if(txbuf !=nullptr) {
	    if(bbtx_put_count == 0) {
	      std::cerr << SoDa::Format("BaseBandTX: first tx_stream put  tx_mode=%0\n")
		.addI((int)tx_mode);
	    }
	    bbtx_put_count++;
	    bbtx_recv_count++;
	    tx_stream->put(txbuf);
	  }
	}
	usleep(1000);
      }
    }
  }

  CBufPtr BaseBandTX::modulateAM(std::vector<float> & audio_buf,
				 bool is_usb,
				 bool is_lsb)
  {
    /// If the modulation scheme is USB or SSB,
    /// we need to make an analytic signal from the scalar real audio_buf.
  
    float total_gain = mic_gain * af_gain;

    if(is_usb || is_lsb) {
      hilbert->apply(audio_buf, audio_IQ_buf, is_lsb, total_gain);
    }
    else {
      unsigned int i;
      for(i = 0; i < audio_buffer_size; i++) {
	audio_IQ_buf[i] = std::complex<float>(audio_buf[i], 0.0) * total_gain;
      }
    }

    // Periodic peak diagnostic — every ~5s of TX (100 buffers @ ~50ms each)
    static unsigned int am_diag_count = 0;
    if((am_diag_count % 100) == 0) {
      float peak = 0.0f;
      for(unsigned int i = 0; i < audio_buffer_size; i++) {
	float a = fabsf(audio_buf[i]);
	if(a > peak) peak = a;
      }
      std::cerr << SoDa::Format("BaseBandTX: modulateAM peak_audio=%0 af_gain=%1 mic_gain=%2\n")
	.addF(peak, 'e', 4).addF(af_gain, 'e', 4).addF(mic_gain, 'f', 4);
    }
    am_diag_count++;

    CBufPtr txbuf = CBuf::make(tx_buffer_size);
    interpolator->apply(audio_IQ_buf, txbuf->getBuf());
    return txbuf;
  }


  CBufPtr BaseBandTX::modulateFM(std::vector<float> & audio_buf, 
				 double deviation)
  {
    unsigned int i;
  
    for(i=0; i < audio_buffer_size; i++) {
      double audio_amp = audio_buf[i] * fm_mic_gain * af_gain;

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
 
    CBufPtr txbuf = CBuf::make(tx_buffer_size);

    // Upsample the IQ audio (at 48KS/s) to the RF sample rate of 625 KS/s
    interpolator->apply(audio_IQ_buf, txbuf->getBuf());

    // Pass the newly created and filled buffer back to the caller
    return txbuf;
  }

  void BaseBandTX::execSetCommand(CommandPtr cmd)
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
      std::cerr << SoDa::Format("BaseBandTX: TX_STATE iparms[0]=%0 tx_stream_on=%1 cw_tx_mode=%2\n")
	.addI(cmd->iparms[0]).addI((int)tx_stream_on).addI((int)cw_tx_mode);
      // transition from RX to TX
      if(cmd->iparms[0] == (int)Command::TX_ON_0) {
	tx_on = true;
	if(!cw_tx_mode) {
	  audio_ifc->wakeIn();
	  tx_stream_on = true;
	}
      }

      // transition from TX to RX
      if(cmd->iparms[0] == (int)Command::TX_OFF_0) {
	tx_on = false;
	if(tx_stream_on) {
	  audio_ifc->sleepIn();
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
      if(cmd->iparms[0] == Command::NOISE) {
	tx_noise_source_ena = true;
	debugMsg("TX Audio IN is NOISE.\n");      
      }
      else {
	tx_noise_source_ena = false;
	debugMsg("TX Audio IN is MIC.\n");
      }
      break;
    case Command::TX_AUDIO_FILT_ENA: 
      if(cmd->iparms[0] == 1) {
	tx_audio_filter_ena = true;
	debugMsg("TX Audio filter is enabled.\n");      
      }
      else {
	tx_audio_filter_ena = false;      
	debugMsg("TX Audio filter is disabled.\n");            
      }
      break;
    
    default:
      break; 
    }
  }

  void BaseBandTX::execGetCommand(CommandPtr cmd)
  {
    (void) cmd; 
  }

  void BaseBandTX::execRepCommand(CommandPtr cmd)
  {
    (void) cmd; 
  }

  /// implement the subscription method
  void BaseBandTX::subscribeToMailBoxes(const std::vector<MailBoxBasePtr> & mailboxes)
  {
    for(auto mbox_p : mailboxes) {
      MailBoxBase::connect<MailBox<CommandPtr>>(mbox_p,
						"CMDstream",
						cmd_stream); 
      MailBoxBase::connect<MailBox<CBufPtr>>(mbox_p,
					     "TXstream",
					     tx_stream); 
    }

    if(cmd_stream == nullptr) {
      throw MissingMailBox("CMD", getSelfPtr());    
    }
    else {
      cmd_subs = cmd_stream->subscribe();
    }

    if(tx_stream == nullptr) {
      throw MissingMailBox("TX", getSelfPtr());
    }
  }
}
