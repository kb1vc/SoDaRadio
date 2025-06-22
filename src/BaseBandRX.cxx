/*
  Copyright (c) 2012, 2025 Matthew H. Reilly (kb1vc)
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

#include "BaseBandRX.hxx"
#include <fstream>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <SoDa/Format.hxx>

#include <memory>

namespace SoDa {
  
  BaseBandRX::BaseBandRX(ParamsPtr params,
			 AudioIfcPtr _audio_ifc) : Thread("BaseBandRX")
  {
    audio_ifc = _audio_ifc; 
    rx_stream = NULL;

    cmd_stream = NULL;

    // set up some convenient defaults
    rx_modulation = Command::USB;
    // what is the default sample rate and buffer size?
    // the audio rate is set and welded, as we have statically calculated
    // filter parameters. 
    audio_sample_rate = params->getAudioSampleRate(); 
    rf_sample_rate = params->getRXRate();


    // build the resamplers
    rf_resampler = SoDa::ReSampler::make(rf_sample_rate, audio_sample_rate, 0.05);
    wbfm_resampler = SoDa::ReSampler::make(rf_sample_rate, audio_sample_rate, 0.05);
  

    audio_buffer_size = rf_resampler->getOutputBufferSize();
    rf_buffer_size = rf_resampler->getInputBufferSize();

    std::cerr << "\n\n*********\naudio_buffer_size " << audio_buffer_size << "\n******\n\n";
    std::cerr << "\n\n*********\naudio_sample_rate " << audio_sample_rate << "\n******\n\n";

    // setup the audio level meter
    log_audio_buffer_size = log10((float) audio_buffer_size); 

    // setup the resampler now...
    buildFilterMap();


    af_filter_selection = Command::BW_6000;
    cur_audio_filter = filter_map[af_filter_selection];

  
    // initial af gain
    af_gain = 1.0;
    af_sidetone_gain = 1.0;
    cur_af_gain = &af_gain; 
    unsigned int i, j;

    // prime the audio stream so that we don't fall behind
    // right away.
    for(j = 0; j < 6; j++) {
      sidetone_silence = FBuf::make(audio_buffer_size); 
      auto & ssv = sidetone_silence->getBuf();
      for(auto & ss : ssv) {
	ss = 0.0;
      }
      if(j < 5) { // don't pend the last buffer, as we use it for background silence
	pendAudioBuffer(sidetone_silence); 
      }
    }
    // create hilbert transformer
    hilbert = SoDa::HilbertTransformer::make(audio_buffer_size);

    // initialize the sample for the NBFM and WBFM demodulator
    last_phase_samp = 0.0;

    // setup the catchup mechanism that adjusts to differences
    // between the radio's clock frequency and the sound system's clock
    in_catchup = false;
    in_fallback = false;  
    // setup the random number generator.  Note that randomness
    // isn't nearly as important as an apparently long period.
    // The RNG is used to "steal" an audio sample out of the
    // stream in a pattern that won't be perceptible to human
    // hearing. 
    srandom(0x13245); 
    // we could do something mod(audio_buffer_size) but 2304 is
    // a tough point to make.  Since superlative randomness isn't
    // all that important, doing mod 2^k where 2^k is the largest
    // power of two less than the audio buffer size will ensure
    // reasonable distribution of the dropouts. (The trick is to
    // make the period undetectable.)
    for(catchup_rand_mask = 0x1;
	catchup_rand_mask < audio_buffer_size;
	catchup_rand_mask = ((catchup_rand_mask << 1) | 1));
    catchup_rand_mask = catchup_rand_mask >> 1;

    // debug help
    dbg_ctr = 0;


    audio_rx_stream_enabled = true;
    debugMsg("audio_rx_stream_enabled = true\n");  
    audio_rx_stream_needs_start = true;

    // log all audio to an output file (debug only....?)
    audio_save_enable = false;
    //audio_file.open("soda_audio.bin", std::ios::out | std::ios::binary);
    //audio_file2.open("soda_audio_iq_fm.bin", std::ios::out | std::ios::binary);

    // pend a null buffer or two just to keep the out stream from 
    // under-flowing
    pendNullBuffer(2);

    // default NBFM squelch is midlin
    nbfm_squelch_level = 1000.0 * ((float) audio_buffer_size); // this is really modest
    // hang time is 5 audio frames (about 1/4 sec)
    nbfm_squelch_hang_time = 5;
    // start with initial hang count of 0 (haven't broken squelch yet)
  }

  void BaseBandRX::demodulateWBFM(CBufPtr rxbuf, Command::ModulationType mod, float af_gain)
  {
    (void) mod;

    auto abuf = FBuf::make(audio_buffer_size);
    auto & audio_buffer = abuf->getBuf();
    std::vector<float> demod_out(rf_buffer_size);
    unsigned int i;

    auto & dbuf = rxbuf->getBuf();

    // Interestingly, arctan based demodulation (see Lyons p 486 for instance)
    // performs much better than the approximation that avoids the atan call.
    // Texts that talk about atan generally don't talk about the problem of
    // rollover, where the sign changes from atan(samp[n]) and atan(samp[n+1]).
    // In this case, dphase will be much bigger than M_PI, and it should be
    // "corrected".  We're really trying to find the angular diference between
    // samples, so the wraparound is important. 

    // broadcast FM has a deviation of +/- 75 kHz or so.  At a sampling
    // rate of 625kHz, we'd see a maximum angle advance, assuming zero-beat, of
    // pi * 75 / 625 =  about pi/8
    // but that puts the audio output way too low... We'd like the audio gain
    // range to be similar to the audio gain setting for NBFM and even CW
    // for a moderately strong signal.  So, we make the angle even larger.
    // much much larger... 
    float recip_max_phase_diff = 32.0 / (M_PI * 75.0e3 / rf_sample_rate); 
    for(i = 0; i < dbuf.size(); i++) {
      // do the atan demod
      // measure the phase of the incoming signal.
      float phase = arg(dbuf[i]);
      float dphase = phase - last_phase_samp;
      if(dphase < -M_PI) dphase += 2.0 * M_PI;
      if(dphase > M_PI) dphase -= 2.0 * M_PI;
      demod_out[i] = recip_max_phase_diff * dphase; 
      last_phase_samp = phase; 
    }
    // now downsample it
    wbfm_resampler->apply(demod_out, audio_buffer);
    // do a median filter to eliminate the pops.
    // better not. fmMedianFilter.apply(audio_buffer, audio_buffer, audio_buffer_size); 
    // gain was arrived at by trial and error.  
    fm_audio_filter->apply(audio_buffer, audio_buffer, af_gain);
    // then send it to the audio port.
    pendAudioBuffer(abuf);
  }

  void BaseBandRX::demodulateNBFM(CBufPtr dbuf, Command::ModulationType mod, float af_gain)
  {
    (void) mod;

    auto abuf = FBuf::make(dbuf->size());
    auto & audio_buffer = abuf->getBuf();
    // we need a vector because we're sending it through a post-demod filter. 
    std::vector<std::complex<float>> demod_out(dbuf->size());
    auto & demod_buffer = dbuf->getBuf();

    // First we need to band-limit the input RF -- modulation width is about 12.5kHz,
    // so the filter should be a 12.5kHz LPF. 
  
    // Interestingly, arctan based demodulation (see Lyons p 486 for instance)
    // performs much better than the approximation that avoids the atan call.
    // Texts that talk about atan generally don't talk about the problem of
    // rollover, where the sign changes from atan(samp[n]) and atan(samp[n+1]).
    // In this case, dphase will be much bigger than M_PI, and it should be
    // "corrected".  We're really trying to find the angular diference between
    // samples, so the wraparound is important. 
    unsigned int i; 
    float amp_sum = 0.0;
    // NB FM has a deviation of +/- 6.25 kHz or so.  At a sampling
    // rate of 625kHz, we'd see a maximum angle advance, assuming zero-beat, of
    // pi * 6.25 / 312.5
    // As with WBFM, we goose the gain a bit 
    float recip_max_phase_diff = 4.0 / (M_PI * 6.25e3 / rf_sample_rate); 
  
    for(i = 0; i < dbuf->size(); i++) {
      // do the atan demod
      // measure the phase of the incoming signal.
      float phase = arg(demod_buffer[i]);
      float dphase = phase - last_phase_samp;
      if(dphase < -M_PI) dphase += 2.0 * M_PI;
      if(dphase > M_PI) dphase -= 2.0 * M_PI;
      demod_out[i] = recip_max_phase_diff * dphase;     
      last_phase_samp = phase; 
      // measure the amplitude of the incoming signal.
      // measure it over a period of one buffer's worth. 
      amp_sum += abs(demod_buffer[i]);
    }

    // now look at the magnitude and compare it to the threshold

    if(amp_sum > nbfm_squelch_level) {
      nbfm_squelch_hang_count = nbfm_squelch_hang_time;
    }
    else if(nbfm_squelch_hang_count > 0) {
      nbfm_squelch_hang_count--;
    }
  
    cur_audio_filter->apply(demod_out, demod_out, af_gain);
  
    if(audio_save_enable) {
      audio_file2.write((char*) demod_out.data(), demod_out.size() * sizeof(std::complex<float>));
    }
    for(i = 0; i < audio_buffer_size; i++) {
      audio_buffer[i] = nbfm_squelch_hang_count ? demod_out[i].real() : 0.0; 
    }
    // do a median filter to eliminate the pops.
    // maybe not... fmMedianFilter.apply(audio_buffer, audio_buffer, audio_buffer_size); 

    // then send it to the audio port.
    pendAudioBuffer(abuf);
  }

  void BaseBandRX::demodulateSSB(CBufPtr dbuf, Command::ModulationType mod)
  {
    auto abuf = FBuf::make(dbuf->size());
    auto & audio_buffer = abuf->getBuf();
    auto & demod_buffer = dbuf->getBuf();

    // at some point I should just redo this with the filter method.
    // the phasing approach isn't all that great...
    // The right thing to do is to take the conj of the dmod buffer if we're LSB
    // then take the real part. 
  
    // shift the Q channel by pi/2
    // note that this hilbert filter transforms the Q channel and delays the I channel
    hilbert->applyIQ(demod_buffer, demod_buffer); 

    // then add/subtract I/Q to a single real channel
    float sbmul = ((mod == Command::LSB) || (mod == Command::CW_L)) ? 1.0 : -1.0;
    unsigned int i; 

    for(i = 0; i < demod_buffer.size(); i++) {
      audio_buffer[i] = (float) (demod_buffer[i].real() + sbmul * demod_buffer[i].imag()); 
    }
    // then send it to the audio port.
    pendAudioBuffer(abuf);
  }

  void BaseBandRX::demodulateAM(CBufPtr dbuf)
  {
    auto abuf = FBuf::make(dbuf->size());
    auto & audio_buffer = abuf->getBuf();
    auto & demod_buffer = dbuf->getBuf();

    unsigned int i;
    float maxval = 0.0;
    float sumsq = 0.0; 
    for(i = 0; i < abuf->size(); i++) {
      float v = 0.5 * abs(demod_buffer[i]);
      if(v > maxval) maxval = v; 
      sumsq += v * v; 
      audio_buffer[i] = v; 
    }
    sumsq = sqrt(sumsq / ((float) dbuf->size()));
    // audio is biased above DC... it really really needs to get its DC component removed. 
    am_audio_filter->apply(audio_buffer, audio_buffer); 

    // then send it to the audio port.
    pendAudioBuffer(abuf);
  }

  void BaseBandRX::demodulate(CBufPtr rxbuf)
  {
    // First we downsample and apply the audio filter unless this is a WBFM signal.
    auto dbufi = CBuf::make(audio_buffer_size); 
    auto dbufo = CBuf::make(audio_buffer_size); 
    // Note that audio_buffer_size must be (sample_length / decimation rate)
  
    if((rx_modulation != Command::WBFM) && (rx_modulation != Command::NBFM)) {
      rf_resampler->apply(rxbuf->getBuf(), dbufi->getBuf());
    
      // now do the low pass filter
      if(rx_modulation == Command::AM) {
	am_pre_filter->apply(dbufi->getBuf(), dbufo->getBuf(), *cur_af_gain);
      }
      else {
	cur_audio_filter->apply(dbufi->getBuf(), dbufo->getBuf(), *cur_af_gain);
      }
    }
    else if(rx_modulation == Command::NBFM) {
      // first, bandpass the RF down to about 25 kHz wide...
      auto & rfbuf = rxbuf->getBuf();
      nbfm_pre_filter->apply(rfbuf, rfbuf, 1.0);
      rf_resampler->apply(rfbuf, dbufo->getBuf());
    }
 
    switch(rx_modulation) {
    case Command::LSB:
    case Command::CW_L:
      demodulateSSB(dbufo, Command::LSB); 
      break; 
    case Command::USB:
    case Command::CW_U:
      demodulateSSB(dbufo, Command::USB); 
      break;
    case Command::NBFM:
      demodulateNBFM(dbufo, Command::NBFM, *cur_af_gain);
      break; 
    case Command::WBFM:
      demodulateWBFM(rxbuf, Command::NBFM, *cur_af_gain);
      break; 
    case Command::AM:
      demodulateAM(dbufo); 
      break; 
    default:
      // all other modes are unsupported just for now.
      throw Radio::Exception("Unsupported Modulation Mode in RX", getSelfPtr());
      break; 
    }
  }

  void BaseBandRX::repAFFilterShape() {
    std::pair<double, double> fshape = cur_audio_filter->getFilterEdges();  
    switch (rx_modulation) {
    case Command::USB:
    case Command::CW_U:
      cmd_stream->put(Command::make(Command::REP, Command::RX_AF_FILTER_SHAPE, 
				    fshape.first, fshape.second));
      break; 
    case Command::LSB:
    case Command::CW_L:
      cmd_stream->put(Command::make(Command::REP, Command::RX_AF_FILTER_SHAPE, 
				    -fshape.first, -fshape.second));
      break; 
    case Command::AM:
      cmd_stream->put(Command::make(Command::REP, Command::RX_AF_FILTER_SHAPE, 
				    -fshape.second, fshape.second));
      break; 
    default:
      cmd_stream->put(Command::make(Command::REP, Command::RX_AF_FILTER_SHAPE, 
				    -100, 100));
    
    }
  }

  void BaseBandRX::execSetCommand(CommandPtr cmd)
  {
    Command::AudioFilterBW fbw;
    Command::ModulationType txmod; 
    switch (cmd->target) {
    case Command::RX_MODE:
      rx_modulation = Command::ModulationType(cmd->iparms[0]);
      repAFFilterShape();    
      break;
    case Command::TX_MODE:
      txmod = Command::ModulationType(cmd->iparms[0]);
      if((txmod == Command::CW_L) || (txmod == Command::CW_U)) {
	sidetone_stream_enabled = true;
      }
      else {
	sidetone_stream_enabled = false; 
      }
      break; 
    case Command::TX_STATE: // SET TX_ON
      if(cmd->iparms[0] == 1) {
	// flush the audio buffers that have RX info that we
	// aren't going to need anymore.
	debugMsg("In TX ON");      
	flushAudioBuffers(); 
	if (cmd->iparms[1] != 0) {
	  // we're in full-duplex mode, don't change the RX at all.
	  debugMsg("full duplex mode\n");
	}
	else if (sidetone_stream_enabled) {
	  debugMsg("sidetone mode\n"); 
	  cur_af_gain = &af_sidetone_gain;
	}
	else {
	  audio_rx_stream_enabled = false;
	  debugMsg("audio_rx_stream_enabled = false\n");
	  // 	audio_ifc->sleepOut();
	}
      }
      if(cmd->iparms[0] == 2) { // the CTRL unit has done the setup.... 
	debugMsg("In RX ON");
	cur_af_gain = &af_gain; 
	audio_rx_stream_enabled = true;
	debugMsg("audio_rx_stream_enabled = true\n");      
      }
      break;
    case Command::RX_AF_FILTER: // set af filter bw.
      fbw = (Command::AudioFilterBW) cmd->iparms[0];
      if(filter_map.find(fbw) != filter_map.end()) {
	cur_audio_filter = filter_map[fbw];
	af_filter_selection = fbw; 
      }
      else {
	// if unsupported -- use widest. 
	cur_audio_filter = filter_map[Command::BW_6000]; 
	af_filter_selection = Command::BW_6000;
      }
      {
	cmd_stream->put(Command::make(Command::REP, Command::RX_AF_FILTER, 
				      af_filter_selection));
	repAFFilterShape();
      }
      break; 
    case Command::RX_AF_GAIN: // set audio gain. 
      af_gain = powf(10.0, 0.25 * (cmd->dparms[0] - 50.0));
      cmd_stream->put(Command::make(Command::REP, Command::RX_AF_GAIN, 
				    50. + 4.0 * log10(af_gain)));
      break; 
    case Command::RX_AF_SIDETONE_GAIN: // set audio gain. 
      af_sidetone_gain = powf(10.0, 0.25 * (cmd->dparms[0] - 50.0));
      // we send out reports for hamlib and other listeners...
      cmd_stream->put(Command::make(Command::REP, Command::RX_AF_SIDETONE_GAIN, 
				    50. + 4.0 * log10(af_sidetone_gain)));
      break;
    case Command::NBFM_SQUELCH:
      nbfm_squelch_level = powf(10, 0.5 * cmd->dparms[0]) * ((float) audio_buffer_size);
      break; 
    default:
      break; 
    }
  }

  void BaseBandRX::execGetCommand(CommandPtr cmd)
  {
    switch (cmd->target) {
    case Command::RX_AF_FILTER: // set af filter bw.
      cmd_stream->put(Command::make(Command::REP, Command::RX_AF_FILTER, 
				    af_filter_selection));
      break;
    case Command::RX_AF_GAIN: // set af filter bw.
      cmd_stream->put(Command::make(Command::REP, Command::RX_AF_GAIN, 
				    50.0 + 4.0 * log10(af_gain)));
      break;
    case Command::DBG_REP: // report status
      Command::UnitSelector us;
      us = Command::UnitSelector(cmd->iparms[0]);
      // do nothing...
      break; 
    default:
      break; 
    }

  }

  void BaseBandRX::execRepCommand(CommandPtr cmd)
  {
    (void) cmd; 
  }

  void BaseBandRX::run()
  {
    bool exitflag = false;
    CBufPtr rxbuf;
    CommandPtr cmd; 

    int trim_count = 0; 
    int add_count = 0;     

    int null_audio_buf_count = 0;
    int sleep_count = 0; 
    int catchup_count = 0;

    int restart_count = 0;

    int debug_count = 0;
    
    if((cmd_stream == NULL) || (rx_stream == NULL)) {
      throw Radio::Exception(std::string("Missing a stream connection.\n"),
			     getSelfPtr());	
    }
  
  
    while(!exitflag) {
      bool did_work = false;
      bool did_audio_work = false; 

      if(cmd_stream->get(cmd_subs, cmd)) {
	// process the command.
	execCommand(cmd);
	did_work = true; 
	exitflag |= (cmd->target == Command::STOP);
	cmd = nullptr; 
      }

      // now look for incoming buffers from the rx_stream. 
      int bcount = 0; 
      for(bcount = 0; (bcount < 5) && rx_stream->get(rx_subs, rxbuf); bcount++) {
	if((debug_count % 20) == 0) {
	  std::cerr << SoDa::Format("BaseBandRX got %0 buffers (%1 samples each) on rx_stream\n")
	    .addI(debug_count)
	    .addI(rxbuf->size())
	    ;
	}
	debug_count++; 
	if(rxbuf == NULL) break; 
	did_work = true; 
	// if we're in TX mode, we should just pend silence and ignore the incoming buffer
	// otherwise, demodulate it.

	if(audio_rx_stream_enabled) {
	  // demodulate the buffer.
	  demodulate(rxbuf); 
	}
	else {
	  pendNullBuffer();
	}
	rxbuf = nullptr;
      }


      if(!did_audio_work && !did_work) {
	usleep(1000); 
	sleep_count++; 
      }
    }
    // close(outdump); 

    if(audio_save_enable) {
      audio_file.close();
      audio_file2.close();    
    }
  }





  void BaseBandRX::pendNullBuffer(int count) {
    for(int b = 0; b < count; b++) {

      auto nullbufptr = FBuf::make(audio_buffer_size);
      auto & nullvec = nullbufptr->getBuf();
      for(auto & v : nullvec) {
	v = 0.0;
      }
      pendAudioBuffer(nullbufptr);
    }
  }

  void BaseBandRX::pendAudioBuffer(FBufPtr bp)
  {
    // no big deal here.  We're going to send it right to 
    // the audio device.
    auto & fv = bp->getBuf();
    auto b = fv.data();
    auto buf_size = fv.size();
    audio_ifc->send(b, buf_size * sizeof(float));

    if(audio_save_enable) {
      audio_file.write((char*) b, buf_size * sizeof(float));
    }

    // nobody should be using the data inside "bp" after this.
  
    float al = 1.0e-19; // really small...
    for(auto m : fv) {
      al += m * m;
    }
    audio_level = 10.0 * (log10(al / af_gain) - log_audio_buffer_size);
  }

  void BaseBandRX::flushAudioBuffers()
  {
    // do we need to do anything here? 
    return;
  }

  void BaseBandRX::buildFilterMap()
  {
    // Each filter is 512 samples long... (a really big filter)
    // The Overlap and Save buffer needs to be long enough to make this all
    // work
  
    filter_map[Command::BW_2000] = OSFilter::make(300.0, 2300.0, 100, audio_sample_rate, audio_buffer_size);
    filter_map[Command::BW_WSPR] = OSFilter::make(1300.0, 1800.0, 100, audio_sample_rate, audio_buffer_size);  
    filter_map[Command::BW_500] = OSFilter::make(400.0, 900.0, 100, audio_sample_rate, audio_buffer_size);

    filter_map[Command::BW_100] = OSFilter::make(400.0, 500.0, 100, audio_sample_rate, audio_buffer_size);

    filter_map[Command::BW_6000] = OSFilter::make(300.0, 6300.0, 100, audio_sample_rate, audio_buffer_size);
    filter_map[Command::BW_PASS] = OSFilter::make(-10.0, 15000.0, 3000, audio_sample_rate, audio_buffer_size);

    fm_audio_filter = OSFilter::make(100.0, 8000.0, 100, audio_sample_rate, audio_buffer_size);
    am_audio_filter = filter_map[Command::BW_6000]; 

    am_pre_filter = OSFilter::make(10.0, 8000.0, 100, audio_sample_rate, audio_buffer_size);

    nbfm_pre_filter = OSFilter::make(10.0,  12500.0, 1000, rf_sample_rate, rf_buffer_size);

  }

  /// implement the subscription method
  void BaseBandRX::subscribeToMailBoxes(const std::vector<MailBoxBasePtr> & mailboxes)
  {
    for(auto mbox_p : mailboxes) {
      MailBoxBase::connect<MailBox<CommandPtr>>(mbox_p,
						"CMDstream",
						cmd_stream); 
      MailBoxBase::connect<MailBox<CBufPtr>>(mbox_p,
					     "RXstream",
					     rx_stream); 
    }

    if(cmd_stream == nullptr) {
      throw MissingMailBox("CMD", getSelfPtr());    
    }
    else {
      cmd_subs = cmd_stream->subscribe();
    }

    if(rx_stream == nullptr) {
      throw MissingMailBox("RX", getSelfPtr());
    }
    else {
      rx_subs = rx_stream->subscribe();
    }
  }
}
