/*
  Copyright (c) 2012, Matthew H. Reilly (kb1vc)
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

#include "AudioTX.hxx"
#include "AudioRX.hxx"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "OSFilter.hxx"
#include "ReSamplers625x48.hxx"

#include "SoDa_tx_filter_tables.hxx" 

SoDa::AudioTX::AudioTX(Params * params, DatMBox * _tx_stream,
		       CmdMBox * _cmd_stream,
		       AudioIfc * _audio_ifc
		       ) : SoDa::SoDaThread("AudioTX")
{
  debug_mode = false;
  debug_ctr = 0;
  tx_stream = _tx_stream;

  cmd_stream = _cmd_stream; 
  cmd_subs = cmd_stream->subscribe();

  audio_buffer_size = params->getAFBufferSize();
  tx_buffer_size = params->getRFBufferSize();
  
  // create the interpolator.
  interpolator = new SoDa::ReSample48to625(audio_buffer_size);

  // create the audio stream.
  // borrow the stream from the AudioRX side. ? 
  //   pa_stream = _pa_stream;
  double srate = params->getAudioSampleRate();

  // now setup the FM deviation
  nbfm_deviation = 2.0 * M_PI * 2.5e3 / srate; // 2.5 kHz max deviation
  wbfm_deviation = 2.0 * M_PI * 75.0e3 / srate; // 75 kHz max deviation
  fm_phase = 0.0; 

  audio_ifc = _audio_ifc; 

  tx_stream_on = false;

  // create the IQ buffer.
  audio_IQ_buf = new std::complex<float>[8*audio_buffer_size];

  // create the Hilbert transformer
  hilbert = new SoDa::HilbertTransformer(audio_buffer_size);

  mic_gain = 0.4;

  fm_mic_gain = 0.8;
}

void SoDa::AudioTX::run()
{
  bool exitflag = false;
  SoDaBuf * rxbuf;
  Command * cmd; 
  float audio_buf[audio_buffer_size];

  /**
   * This is the AudioTX run loop.
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
  while(!exitflag) {
    if((cmd = cmd_stream->get(cmd_subs)) != NULL) {
      // process the command.
      execCommand(cmd);
      exitflag |= (cmd->target == Command::STOP); 
      cmd_stream->free(cmd); 
    }
    else if (cw_tx_mode) {
      usleep(1000); 
    }
    else {
      // If we're in TX mode that isn't CW....
      // get an input audio buffer.
      if (tx_stream_on && (audio_ifc->recvBufferReady(audio_buffer_size))) {
	audio_ifc->recv(audio_buf, audio_buffer_size); 
	SoDaBuf * txbuf = NULL; 
	//
	if(tx_mode == SoDa::Command::USB) {
	  txbuf = modulateAM(audio_buf, audio_buffer_size, true, false); 
	}
	else if(tx_mode == SoDa::Command::LSB) {
	  txbuf = modulateAM(audio_buf, audio_buffer_size, false, true); 
	}
	else if(tx_mode == SoDa::Command::AM) {
	  txbuf = modulateAM(audio_buf, audio_buffer_size, false, false); 
	}
	else if(tx_mode == SoDa::Command::NBFM) {
	  txbuf = modulateFM(audio_buf, audio_buffer_size, nbfm_deviation);
	}
	else if(tx_mode == SoDa::Command::WBFM) {
	  txbuf = modulateFM(audio_buf, audio_buffer_size, wbfm_deviation);
	}
	if(txbuf != NULL) {
	  tx_stream->put(txbuf); 
	}
      }      
      usleep(1000); 
    }
  }
}

SoDa::SoDaBuf * SoDa::AudioTX::modulateAM(float * audio_buf,
					  unsigned int len,
					  bool is_usb,
					  bool is_lsb)
{
  /// If the modulation scheme is USB or SSB,
  /// we need to make an analytic signal from the scalar real audio_buf.
  
  if(is_usb || is_lsb) {
    /// If we're looking at USB, then for I = sin(w), Q => cos(w)
    /// for LSB I = sin(w), Q => -cos(w)
    hilbert->apply(audio_buf, audio_IQ_buf, is_lsb, mic_gain);
  }
  else {
    /// if neither is_usb is_lsb is true, then we want to produce
    /// an AM envelope.  Put the same signal in both I and Q.
    int i;
    for(i = 0; i < audio_buffer_size; i++) {
      audio_IQ_buf[i] = std::complex<float>(audio_buf[i], 0.0) * mic_gain;
    }
  }

  

  /// Now that the I/Q channels have been populated, get a transmit buffer. 
  /// and upsample the I/Q audio up to the RF rate.
  SoDa::SoDaBuf * txbuf = tx_stream->alloc();
  if(txbuf == NULL) {
    txbuf = new SoDaBuf(tx_buffer_size); 
  }
  if(txbuf->getComplexLen() < tx_buffer_size) {
    throw(new SoDa::SoDaException("Transmit signal buffer was a bad size.", this));
  }
  
  /// Upsample the IQ audio (at 48KS/s) to the RF sample rate of 625 KS/s
  interpolator->apply(audio_IQ_buf, txbuf->getComplexBuf()); 

  /// pass the newly created and filled buffer back to the caller
  return txbuf; 
}


SoDa::SoDaBuf * SoDa::AudioTX::modulateFM(float *audio_buf, unsigned int len, double deviation)
{
  int i;
  
  for(i=0; i < audio_buffer_size; i++) {
    double audio_amp = audio_buf[i] * fm_mic_gain;

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
 
  SoDa::SoDaBuf * txbuf = tx_stream->alloc();
  if(txbuf == NULL){
    txbuf = new SoDaBuf(tx_buffer_size);
  }

  if(txbuf->getComplexLen() < tx_buffer_size){
    throw(new SoDa::SoDaException("FM: Transmit signal buffer was a bad size.",this));
  }
  // Upsample the IQ audio (at 48KS/s) to the RF sample rate of 625 KS/s
  interpolator->apply(audio_IQ_buf, txbuf->getComplexBuf());

  // Pass the newly created and filled buffer back to the caller
  return txbuf;
}


void SoDa::AudioTX::execSetCommand(SoDa::Command * cmd)
{
  SoDa::Command::AudioFilterBW fbw;

  switch (cmd->target) {
  case SoDa::Command::TX_MODE:
    // we only care about CW mode or NOT CW mode.
    tx_mode = SoDa::Command::ModulationType(cmd->iparms[0]);
    if((tx_mode == SoDa::Command::CW_L) || (tx_mode == SoDa::Command::CW_U)) {
      cw_tx_mode = true; 
    }
    else {
      cw_tx_mode = false; 
    }
    break; 
  case SoDa::Command::TX_STATE: // SET TX_ON
    // transition from RX to TX
    if(cmd->iparms[0] == 3) {
      tx_on = true;
      if(!cw_tx_mode) {
	// Wake up the audio interface. 
	audio_ifc->wakeIn();
	tx_stream_on = true; 
      }
    }

    // transition from TX to RX
    if(cmd->iparms[0] == 0) {
      tx_on = false;
      if(tx_stream_on) {
	// Put the audio interface to sleep
	audio_ifc->sleepIn(); 
	tx_stream_on = false; 
      }
    }
    break;
  case SoDa::Command::TX_AF_GAIN: // set audio gain.
    // audio gain is passed around as linear (in dB), but
    // gets converted before we set the envelope power.
    af_gain = powf(10.0, 0.1 * (cmd->dparms[0] - 50.0));
    cmd_stream->put(new Command(Command::REP, Command::TX_AF_GAIN, 
				50 + 10.0 * log10(af_gain)));
    break; 
  }
}

void SoDa::AudioTX::execGetCommand(SoDa::Command * cmd)
{
  switch (cmd->target) {
  }
}

void SoDa::AudioTX::execRepCommand(SoDa::Command * cmd)
{
}

