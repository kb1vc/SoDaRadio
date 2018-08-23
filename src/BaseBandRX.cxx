/*
  Copyright (c) 2012, Matthew H. Reilly (kb1vc)
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
#include "OSFilter.hxx"
#include <fstream>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

SoDa::BaseBandRX::BaseBandRX(Params * params,
		       DatMBox * _rx_stream, CmdMBox * _cmd_stream,
		       AudioIfc * _audio_ifc) : SoDa::SoDaThread("BaseBandRX")
{
  audio_ifc = _audio_ifc; 
  rx_stream = _rx_stream;
  rx_subs = rx_stream->subscribe();

  cmd_stream = _cmd_stream; 
  cmd_subs = cmd_stream->subscribe();

  // set up some convenient defaults
  rx_modulation = SoDa::Command::USB;
  // what is the default sample rate and buffer size?
  // the audio rate is set and welded, as we have statically calculated
  // filter parameters. 
  audio_sample_rate = params->getAudioSampleRate(); 
  rf_sample_rate = params->getRXRate(); 
  audio_buffer_size = params->getAFBufferSize();
  rf_buffer_size = params->getRFBufferSize();

  // setup the audio level meter
  log_audio_buffer_size = log10((float) audio_buffer_size); 

  // setup the resampler now...
  buildFilterMap();

  // build the resamplers
  rf_resampler = new SoDa::TDResampler625x48<std::complex<float> >(150000.0);
  wbfm_resampler = new SoDa::TDResampler625x48<float>(1.0);  

  af_filter_selection = SoDa::Command::BW_6000;
  cur_audio_filter = filter_map[af_filter_selection];
  
  // initial af gain
  af_gain = 1.0;
  af_sidetone_gain = 1.0;
  cur_af_gain = &af_gain; 
  unsigned int i, j;
  
  // create hilbert transformer
  hilbert = new SoDa::HilbertTransformer(audio_buffer_size);

  // initialize the sample for the NBFM and WBFM demodulator
  last_phase_samp = 0.0;

  // debug help
  dbg_ctr = 0;

  audio_rx_stream_enabled = true;
  debugMsg("audio_rx_stream_enabled = true\n");  

  // log all audio to an output file (debug only....?)
  audio_save_enable = false;
  //audio_file.open("soda_audio.bin", std::ios::out | std::ios::binary);
  //audio_file2.open("soda_audio_iq_fm.bin", std::ios::out | std::ios::binary);
}

void SoDa::BaseBandRX::demodulateWBFM(SoDaBuf * rxbuf, SoDa::Command::ModulationType mod, float af_gain)
{
  (void) mod;
  // now allocate a new audio buffer from the buffer ring
  float * audio_buffer = audio_ifc->getBuffer(audio_buffer_size);
  float demod_out[rf_buffer_size];
  unsigned int i;

  std::complex<float> * dbuf = rxbuf->getComplexBuf();

  // Interestingly, arctan based demodulation (see Lyons p 486 for instance)
  // performs much better than the approximation that avoids the atan call.
  // Texts that talk about atan generally don't talk about the problem of
  // rollover, where the sign changes from atan(samp[n]) and atan(samp[n+1]).
  // In this case, dphase will be much bigger than M_PI, and it should be
  // "corrected".  We're really trying to find the angular diference between
  // samples, so the wraparound is important. 
  for(i = 0; i < rf_buffer_size; i++) {
    // do the atan demod
    // measure the phase of the incoming signal.
    float phase = arg(dbuf[i]);
    float dphase = phase - last_phase_samp;
    if(dphase < -M_PI) dphase += 2.0 * M_PI;
    if(dphase > M_PI) dphase -= 2.0 * M_PI;
    demod_out[i] = af_gain * dphase; 
    last_phase_samp = phase; 
  }
  // now downsample it
  wbfm_resampler->apply(demod_out, audio_buffer, rf_buffer_size, audio_buffer_size);
  // do a median filter to eliminate the pops.
  // better not. fmMedianFilter.apply(audio_buffer, audio_buffer, audio_buffer_size); 
  // gain was arrived at by trial and error.  
  fm_audio_filter->apply(audio_buffer, audio_buffer, 0.0012);
  // then send it to the audio port.
  audio_ifc->send(audio_buffer, audio_buffer_size);
}

void SoDa::BaseBandRX::demodulateNBFM(std::complex<float> * dbuf, SoDa::Command::ModulationType mod, float af_gain)
{
  (void) mod;
  // now allocate a new audio buffer from the buffer ring
  float * audio_buffer = audio_ifc->getBuffer(audio_buffer_size);
  std::complex<float> demod_out[audio_buffer_size];

  // Interestingly, arctan based demodulation (see Lyons p 486 for instance)
  // performs much better than the approximation that avoids the atan call.
  // Texts that talk about atan generally don't talk about the problem of
  // rollover, where the sign changes from atan(samp[n]) and atan(samp[n+1]).
  // In this case, dphase will be much bigger than M_PI, and it should be
  // "corrected".  We're really trying to find the angular diference between
  // samples, so the wraparound is important. 
  unsigned int i; 
  for(i = 0; i < audio_buffer_size; i++) {
    // do the atan demod
    // measure the phase of the incoming signal.
    float phase = arg(dbuf[i]);
    float dphase = phase - last_phase_samp;
    if(dphase < -M_PI) dphase += 2.0 * M_PI;
    if(dphase > M_PI) dphase -= 2.0 * M_PI;
    demod_out[i] = af_gain * dphase; 
    last_phase_samp = phase; 
  }

  cur_audio_filter->apply(demod_out, demod_out, 25.0);
  
  if(audio_save_enable) {
    audio_file2.write((char*) demod_out, audio_buffer_size * sizeof(std::complex<float>));
  }
  for(i = 0; i < audio_buffer_size; i++) {
    audio_buffer[i] = demod_out[i].real(); 
  }
  // do a median filter to eliminate the pops.
  // maybe not... fmMedianFilter.apply(audio_buffer, audio_buffer, audio_buffer_size); 

  // then send it to the audio port.
  audio_ifc->send(audio_buffer, audio_buffer_size);
}

void SoDa::BaseBandRX::demodulateSSB(std::complex<float> * dbuf, SoDa::Command::ModulationType mod)
{
  // now allocate a new audio buffer from the buffer ring
  float * audio_buffer = audio_ifc->getBuffer(audio_buffer_size);

  // shift the Q channel by pi/2
  // note that this hilbert filter transforms the Q channel and delays the I channel
  hilbert->applyIQ(dbuf, dbuf); 

  // then add/subtract I/Q to a single real channel
  float sbmul = ((mod == SoDa::Command::LSB) || (mod == SoDa::Command::CW_L)) ? 1.0 : -1.0;
  unsigned int i; 
  for(i = 0; i < audio_buffer_size; i++) {
    audio_buffer[i] = (float) (dbuf[i].real() + sbmul * dbuf[i].imag()); 
  }
  // then send it to the audio port.
  audio_ifc->send(audio_buffer, audio_buffer_size);
}

void SoDa::BaseBandRX::demodulateAM(std::complex<float> * dbuf)
{
  // now allocate a new audio buffer from the buffer ring
  float * audio_buffer = audio_ifc->getBuffer(audio_buffer_size);

  unsigned int i;
  float maxval = 0.0;
  float sumsq = 0.0; 
  for(i = 0; i < audio_buffer_size; i++) {
    float v = 0.5 * abs(dbuf[i]);
    if(v > maxval) maxval = v; 
    sumsq += v * v; 
    audio_buffer[i] = v; 
  }
  sumsq = sqrt(sumsq / ((float) audio_buffer_size));
  // if((dbg_ctr & 0xff) == 0) {
  //   std::cerr << boost::format("maxval = %f rms = %f\n") % maxval % sumsq;
  // }

  // audio is biased above DC... it really really needs to get its DC component removed. 
  am_audio_filter->apply(audio_buffer, audio_buffer); 

  // then send it to the audio port.
  audio_ifc->send(audio_buffer, audio_buffer_size);  
}

void SoDa::BaseBandRX::demodulate(SoDaBuf * rxbuf)
{
  // First we downsample and apply the audio filter unless this is a WBFM signal.
  std::complex<float> dbufi[audio_buffer_size]; 
  std::complex<float> dbufo[audio_buffer_size]; 
  // Note that audio_buffer_size must be (sample_length / decimation rate)
  
  if((rx_modulation != SoDa::Command::WBFM) && (rx_modulation != SoDa::Command::NBFM)) {
    rf_resampler->apply(rxbuf->getComplexBuf(), dbufi, rf_buffer_size, audio_buffer_size);
    
    // now do the low pass filter
    if(rx_modulation == SoDa::Command::AM) {
      am_pre_filter->apply(dbufi, dbufo, *cur_af_gain); 
    }
    else {
      cur_audio_filter->apply(dbufi, dbufo, *cur_af_gain);
    }
  }
  else if(rx_modulation == SoDa::Command::NBFM) {
    // first, bandpass the RF down to about 25 kHz wide...
    std::complex<float> * rfbuf = rxbuf->getComplexBuf();
    nbfm_pre_filter->apply(rfbuf, rfbuf, 1.0);
    rf_resampler->apply(rfbuf, dbufo, rf_buffer_size, audio_buffer_size);
  }

 
  switch(rx_modulation) {
  case SoDa::Command::LSB:
  case SoDa::Command::CW_L:
    demodulateSSB(dbufo, SoDa::Command::LSB); 
    break; 
  case SoDa::Command::USB:
  case SoDa::Command::CW_U:
    demodulateSSB(dbufo, SoDa::Command::USB); 
    break;
  case SoDa::Command::NBFM:
    demodulateNBFM(dbufo, SoDa::Command::NBFM, *cur_af_gain);
    break; 
  case SoDa::Command::WBFM:
    demodulateWBFM(rxbuf, SoDa::Command::NBFM, *cur_af_gain);
    break; 
  case SoDa::Command::AM:
    demodulateAM(dbufo); 
    break; 
  default:
    // all other modes are unsupported just for now.
    throw(new SoDa::SoDaException("Unsupported Modulation Mode in RX", this)); 
    break; 
  }
}

void SoDa::BaseBandRX::repAFFilterShape() {
  std::pair<double, double> fshape = cur_audio_filter->getFilterEdges();  
  switch (rx_modulation) {
  case SoDa::Command::USB:
  case SoDa::Command::CW_U:
      cmd_stream->put(new Command(Command::REP, Command::RX_AF_FILTER_SHAPE, 
				  fshape.first, fshape.second));
      break; 
  case SoDa::Command::LSB:
  case SoDa::Command::CW_L:
      cmd_stream->put(new Command(Command::REP, Command::RX_AF_FILTER_SHAPE, 
				  -fshape.first, -fshape.second));
      break; 
  case SoDa::Command::AM:
      cmd_stream->put(new Command(Command::REP, Command::RX_AF_FILTER_SHAPE, 
				  -fshape.second, fshape.second));
      break; 
  default:
      cmd_stream->put(new Command(Command::REP, Command::RX_AF_FILTER_SHAPE, 
				  -100, 100));
    
  }
}

void SoDa::BaseBandRX::execSetCommand(SoDa::Command * cmd)
{
  SoDa::Command::AudioFilterBW fbw;
  SoDa::Command::ModulationType txmod; 
  switch (cmd->target) {
  case SoDa::Command::RX_MODE:
    rx_modulation = SoDa::Command::ModulationType(cmd->iparms[0]);
    repAFFilterShape();    
    break;
  case SoDa::Command::TX_MODE:
    txmod = SoDa::Command::ModulationType(cmd->iparms[0]);
    if((txmod == SoDa::Command::CW_L) || (txmod == SoDa::Command::CW_U)) {
      sidetone_stream_enabled = true;
    }
    else {
      sidetone_stream_enabled = false; 
    }
    break; 
  case SoDa::Command::TX_STATE: // SET TX_ON
    if(cmd->iparms[0] == 1) {
      // flush the audio buffers that have RX info that we
      // aren't going to need anymore.
      debugMsg("In TX ON");      
      audio_ifc->flushRXBuffers();
      if (sidetone_stream_enabled) {
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
  case SoDa::Command::RX_AF_FILTER: // set af filter bw.
    fbw = (SoDa::Command::AudioFilterBW) cmd->iparms[0];
    if(filter_map.find(fbw) != filter_map.end()) {
      cur_audio_filter = filter_map[fbw];
      af_filter_selection = fbw; 
    }
    else {
      // if unsupported -- use widest. 
      cur_audio_filter = filter_map[SoDa::Command::BW_6000]; 
      af_filter_selection = SoDa::Command::BW_6000;
    }
    {
      cmd_stream->put(new Command(Command::REP, Command::RX_AF_FILTER, 
				  af_filter_selection));
      repAFFilterShape();
    }
    break; 
  case SoDa::Command::RX_AF_GAIN: // set audio gain. 
    af_gain = powf(10.0, 0.25 * (cmd->dparms[0] - 50.0));
    cmd_stream->put(new Command(Command::REP, Command::RX_AF_GAIN, 
				50. + 4.0 * log10(af_gain)));
    break; 
  case SoDa::Command::RX_AF_SIDETONE_GAIN: // set audio gain. 
    af_sidetone_gain = powf(10.0, 0.25 * (cmd->dparms[0] - 50.0));
    cmd_stream->put(new Command(Command::REP, Command::RX_AF_SIDETONE_GAIN, 
				50. + 4.0 * log10(af_sidetone_gain)));
    break; 
  default:
    break; 
  }
}

void SoDa::BaseBandRX::execGetCommand(SoDa::Command * cmd)
{
  switch (cmd->target) {
  case SoDa::Command::RX_AF_FILTER: // set af filter bw.
    cmd_stream->put(new Command(Command::REP, Command::RX_AF_FILTER, 
				af_filter_selection));
    break;
  case SoDa::Command::RX_AF_GAIN: // set af filter bw.
    cmd_stream->put(new Command(Command::REP, Command::RX_AF_GAIN, 
				50.0 + 4.0 * log10(af_gain)));
    break;
  case SoDa::Command::DBG_REP: // report status
    break; 
  default:
    break; 
  }

}

void SoDa::BaseBandRX::execRepCommand(SoDa::Command * cmd)
{
  (void) cmd; 
}

void SoDa::BaseBandRX::run()
{
  bool exitflag = false;
  SoDaBuf * rxbuf;
  Command * cmd; 

  int trim_count = 0; 
  int add_count = 0;     

  int null_audio_buf_count = 0;
  int sleep_count = 0; 
  int catchup_count = 0;

  int restart_count = 0;

  while(!exitflag) {
    bool did_work = false;
    bool did_audio_work = false; 

    if((cmd = cmd_stream->get(cmd_subs)) != NULL) {
      // process the command.
      execCommand(cmd);
      did_work = true; 
      exitflag |= (cmd->target == Command::STOP); 
      cmd_stream->free(cmd); 
    }

    // now look for incoming buffers from the rx_stream. 
    int bcount = 0; 
    for(bcount = 0; (bcount < 2) && ((rxbuf = rx_stream->get(rx_subs)) != NULL); bcount++) {
      if(rxbuf == NULL) break; 
      did_work = true; 
      // if we're in TX mode, we should just pend silence and ignore the incoming buffer
      // otherwise, demodulate it.

      if(audio_rx_stream_enabled) {
	// demodulate the buffer.
	demodulate(rxbuf); 
      }
      // now free the buffer up.
      rx_stream->free(rxbuf); 
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

void SoDa::BaseBandRX::buildFilterMap()
{
  // Each filter is 512 samples long... (a really big filter)
  // The Overlap and Save buffer needs to be long enough to make this all
  // work
  
  filter_map[SoDa::Command::BW_2000] = new SoDa::OSFilter(200.0, 300.0, 2300.0, 2400.0, 512, 1.0, audio_sample_rate, audio_buffer_size);
  filter_map[SoDa::Command::BW_WSPR] = new SoDa::OSFilter(1100.0, 1400.0, 1600.0, 1900.0, 512, 1.0, audio_sample_rate, audio_buffer_size);  
  filter_map[SoDa::Command::BW_500] = new SoDa::OSFilter(300.0, 400.0, 900.0, 1000.0, 512, 1.0, audio_sample_rate, audio_buffer_size);

  filter_map[SoDa::Command::BW_100] = new SoDa::OSFilter(300.0, 400.0, 500.0, 600.0, 512, 1.0, audio_sample_rate, audio_buffer_size);

  filter_map[SoDa::Command::BW_6000] = new SoDa::OSFilter(200.0, 300.0, 6300.0, 6400.0, 512, 1.0, audio_sample_rate, audio_buffer_size);
  filter_map[SoDa::Command::BW_PASS] = new SoDa::OSFilter(0.0, 10.0, 15000.0, 18000.0, 512, 1.0, audio_sample_rate, audio_buffer_size);

  fm_audio_filter = new SoDa::OSFilter(50.0, 100.0, 8000.0, 9000.0, 512, 1.0, audio_sample_rate, audio_buffer_size);
  am_audio_filter = filter_map[SoDa::Command::BW_6000]; 

  am_pre_filter = new SoDa::OSFilter(0.0, 0.0, 8000.0, 9000.0, 512, 1.0, audio_sample_rate, audio_buffer_size);

  nbfm_pre_filter = new SoDa::OSFilter(0.0, 0.0, 25000.0, 32000.0, 512, 1.0, rf_sample_rate, rf_buffer_size);


}
