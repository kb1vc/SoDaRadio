/*
  Copyright (c) 2017, Matthew H. Reilly (kb1vc)
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

#include "SoapyTX.hxx"
#include <SoapySDR/Errors.hpp>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

SoDa::SoapyTX::SoapyTX(Params * params, SoDa::SoapyCtrl * _ctrl, 
		       DatMBox * _tx_stream, DatMBox * _cw_env_stream,
		       CmdMBox * _cmd_stream) : SoDa::SoDaThread("SoapyTX")
{
  cmd_stream = _cmd_stream;
  tx_stream = _tx_stream;
  cw_env_stream = _cw_env_stream;

  ctrl = _ctrl; 
  radio = ctrl->getSoapySDR(); 
  
  // subscribe to the command stream.
  cmd_subs = cmd_stream->subscribe();
  // subscribe to the tx data stream
  tx_subs = tx_stream->subscribe();
  // and to the CW envelope stream
  cw_subs = cw_env_stream->subscribe(); 

  first_burst = true; 
  LO_enabled = false;
  LO_configured = false;
  LO_capable = false;

  // create the tx buffer streamers.
  std::vector<size_t> channel_nums;
  channel_nums.push_back(0);  
  tx_bits = radio->setupStream(SOAPY_SDR_TX, "CF32", channel_nums); 
#if 0
  int stat = radio->deactivateStream(tx_bits);
  if(stat < 0) debugMsg(boost::format("constructor: deactivateStream returns [%d] [%s]\n") % stat % SoapySDR::errToStr(stat));          
#endif  
  tx_activated = false; 

  radio->setDCOffsetMode(SOAPY_SDR_TX, 0, true); // set to automatic compensation

  // find out how to configure the transmitter
  tx_sample_rate = params->getTXRate();
  tx_buffer_size = params->getRFBufferSize();

  // we need to put some artificial backpressure on sending, since the Lime support
  // for SoapySDR has very very deep buffers (18 seconds at 625ksamp/sec!)  
  seconds_per_sample = 1.0 / ((double) tx_sample_rate); 

  // we want to keep about 1 second in flight
  in_flight_limit = tx_sample_rate; 
  // reset the samples_in_flight counter
  samples_in_flight = 0; 

  // 400 Hz is a nice tone
  // but 400 doesn't really work that well.
  // 650 is better.  but all the RX filters
  // are centered near 500... let's do that.
  CW_tone_freq = 500.0;
  setCWFreq(true, CW_tone_freq); 

  // we aren't waiting for anything. 
  waiting_to_run_dry = false; 

  // build the beacon buffer, and the zero buffer.
  beacon_env = new float[tx_buffer_size];
  zero_env = new float[tx_buffer_size];
  for(unsigned int i = 0; i < tx_buffer_size; i++) {
    beacon_env[i] = 1.0;
    zero_env[i] = 0.0; 
  }

  // build the cwbuffer
  cw_buf = new std::complex<float>[tx_buffer_size];

  // set the initial envelope amplitude
  cw_env_amplitude = 0.7; 
  
  // build the zero buffer and the transverter lo buffer
  zero_buf = new std::complex<float>[tx_buffer_size];
  const_buf = new std::complex<float>[tx_buffer_size];
  for(unsigned int i = 0; i < tx_buffer_size; i++) {
    zero_buf[i] = std::complex<float>(0.0, 0.0);
    const_buf[i] = std::complex<float>(1.0, 0.0);
  }

  send_histo = new SoDa::Histogram(1000, 100e-6, 100e-3);
  write_stream_histo = new SoDa::Histogram(1000, 100e-6, 100e-3); 

  tx_enabled = false;
}

void SoDa::SoapyTX::run()
{

  bool exitflag = false;
  SoDaBuf * txbuf, * cwenv;
  Command * cmd; 
  std::complex<float> * tbuf; 

  int stat;
  int Acount, Bcount, Ccount, Dcount, Ecount; 

  Acount = Bcount = Ccount = Dcount = Ecount = 0; 

  while(!ctrl->isReady()) {
    usleep(1000);
  }
  


  while(!exitflag) {
    bool didwork = false; 

    if((cmd = cmd_stream->get(cmd_subs)) != NULL) {
      // process the command.
      execCommand(cmd);
      didwork = true; 
      exitflag |= (cmd->target == Command::STOP); 
      cmd_stream->free(cmd); 
    }
    else if(tx_enabled &&
	    tx_bits &&
	    (tx_modulation != SoDa::Command::CW_L) &&
	    (tx_modulation != SoDa::Command::CW_U) &&
	    (txbuf = tx_stream->get(tx_subs)) != NULL) {
      // get a buffer and send it. 
      Acount++; 
      std::cerr << ".";
      tbuf = txbuf->getComplexBuf(); 
      double ts = getTime();
      stat = sendBuffer(tbuf, txbuf->getComplexLen()); 
      double te = getTime();
      send_histo->updateTable(te - ts); 
      std::cerr << ".";   
      if(stat < 0) debugMsg(boost::format("A: writeStream returns [%d] [%s] tx_modulation = %d\n") 
			    % stat % SoapySDR::errToStr(stat) % tx_modulation);
      didwork = true; 
      // now free the buffer up.
      tx_stream->free(txbuf);
    }
    else if(tx_enabled &&
	    tx_bits &&
	    ((tx_modulation == SoDa::Command::CW_L) ||    
	     (tx_modulation == SoDa::Command::CW_U))) {
      cwenv = cw_env_stream->get(cw_subs); 
      if(cwenv != NULL) {
	Bcount++; 
	// modulate a carrier with a cw message
	doCW(cw_buf, cwenv->getFloatBuf(), cwenv->getComplexLen());
	// now send it to the radio
	double ts = getTime();	
	stat = sendBuffer(cw_buf, cwenv->getComplexLen()); 
	double te = getTime();
	send_histo->updateTable(te - ts); 
	if(stat < 0) debugMsg(boost::format("B: writeStream returns [%d] [%s] Bc = %d Cc = %d\n") 
			      % stat % SoapySDR::errToStr(stat) % Bcount % Ccount);      
	cw_env_stream->free(cwenv);
	didwork = true; 
      }
      else {
	// we have an empty CW buffer -- we've run out of text.
	Ccount++; 
	doCW(cw_buf, zero_env, tx_buffer_size);

	// are we supposed to tell anybody about this? 
	if(waiting_to_run_dry) {
	  drainTXStream();
	  cmd_stream->put(new Command(Command::REP, Command::TX_CW_EMPTY, 0));
	  debugMsg("clear waiting_to_run_dry\n");	  
	  waiting_to_run_dry = false; 
	}
      }
    }
    else if(tx_enabled &&
	    tx_bits &&
	    beacon_mode) {
      // modulate a carrier with a constant envelope
      doCW(cw_buf, beacon_env, tx_buffer_size);
      // now send it to the radio
      double ts = getTime();      
      stat = sendBuffer(cw_buf, tx_buffer_size);
      double te = getTime();
      send_histo->updateTable(te - ts); 
      
      if(stat < 0) debugMsg(boost::format("D: writeStream returns [%d] [%s]\n") % stat % SoapySDR::errToStr(stat));      
      didwork = true; 
    }
    else if(tx_enabled && 
	    tx_bits) {
      // nothing to do here. send nothing? 
    }

    if(!didwork) {
      usleep(10);
    }
  }

  if(tx_activated) {
    debugMsg("Deactivate TX Stream\n");
    radio->deactivateStream(tx_bits);
    debugMsg("Close TX Stream\n");    
    radio->closeStream(tx_bits);
    debugMsg("Closed TX Stream\n");    
  }

  send_histo->writeTable("SendHisto.dat");
  write_stream_histo->writeTable("WriteStreamHisto.dat");  

  debugMsg("Leaving\n");
}


void SoDa::SoapyTX::doCW(std::complex<float> * out, float * envelope, unsigned int env_len)
{
  unsigned int i;
  std::complex<float> c;
  
  for(i = 0; i < env_len; i++) {
    c = CW_osc.stepOscCF(); 
    out[i] = c * envelope[i] * cw_env_amplitude;
  }
}

void SoDa::SoapyTX::setCWFreq(bool usb, double freq)
{
  // set to - for USB and + for LSB.
  CW_osc.setPhaseIncr((usb ? -1.0 : 1.0) * freq * 2.0 * M_PI / tx_sample_rate);
  // likely to be extremely small... 
}

bool SoDa::SoapyTX::drainTXStream()
{
  debugMsg("Writing zero_buf, draining tx stream\n");      
  std::cerr << "&";      
  // send a zero filled buffer. 
  sendBuffer(zero_buf, tx_buffer_size, true);
  std::cerr << "&";      	    
  // now wait for the end of buffer marker
  int itercount = 0; 
  while(!lookForEOB(10000)) {
    itercount++; 
    if((itercount & 0xff) == 0) {
      std::cerr << boost::format("Still looking for EOB after %d iterations.\n") % itercount;
    }
  }

  return true; 
}

void SoDa::SoapyTX::transmitSwitch(bool tx_on)
{
  if(tx_on) {
    debugMsg(boost::format("transmitSwitch(ON) tx_enabled = %c\n") % ((char) (tx_enabled ? 'T' : 'F')));
    if(tx_enabled) return;
    tx_enabled = true; 
    samples_in_flight = 0; 
  }
  else {
    debugMsg("transmitSwitch off\n");
    if(!tx_enabled && !LO_enabled) return;
    tx_enabled = false;
  }
}

bool SoDa::SoapyTX::lookForEOB(unsigned long timeout_us) {
  int flags = 0; 
  size_t chan_mask = 1; 
  long long tns; 
  int stat = radio->readStreamStatus(tx_bits, chan_mask, flags, tns, timeout_us); 
  debugMsg(boost::format("lookForEOB read status returns [%d] [%s]  flags [%d], time %ld\n")
	   % stat % SoapySDR::errToStr(stat) % flags % tns); 
  // if(stat != 0) return false; 
  if(flags & SOAPY_SDR_END_BURST) { 
    return true; 
  }
  else return false; 
}

int SoDa::SoapyTX::sendBuffer(std::complex<float> * buf, size_t len, bool end_burst) {

  if(!tx_activated) {
    debugMsg("Activating TX stream A\n");
    int stat = radio->activateStream(tx_bits);
    if(stat < 0) debugMsg(boost::format("A: activateStream returns [%d] [%s]\n") 
			  % stat % SoapySDR::errToStr(stat));      		
    tx_activated = true; 
  }

  std::complex<float> * buflist[1]; 
  buflist[0] = buf;

  size_t left = len;   
  int flags = end_burst ? SOAPY_SDR_END_BURST : 0; 

  double tst, ten; 
  while(left > 0) {
    tst = getTime(); 
    int stat = radio->writeStream(tx_bits, (void**) buflist, left, flags); 
    ten = getTime();
    write_stream_histo->updateTable(ten - tst); 
    if(stat < 0) {
      return stat; 
    }
    else {
      left -= stat; 
      flags = 0; 
      buflist[0] += stat; 
    }
  }

  // how many samples outstanding so far? 
  samples_in_flight += (long) len; 
  // sleep long enough to let the radio digest a little bit. 
  // but don't do this just for the heck of it... 
  if((samples_in_flight - in_flight_limit) > (in_flight_limit / 8)) {
    double wait_time = seconds_per_sample * ((double) len);  // (samples_in_flight - in_flight_limit));
    double start_time = getTime();
    double target_time = start_time + wait_time;
    while(getTime() < target_time) {
      // usleep(10); 
    }
    double time_span = getTime() - start_time; 
    samples_in_flight -= ((long) (tx_sample_rate * time_span));
    if(samples_in_flight < 0) {
      std::cerr << boost::format("Holy cow!  s_i_f = %ld  wait_time = %g\n") 
	% samples_in_flight % wait_time; 
      samples_in_flight = 0; 
    }
  }

  return len; 
}

void SoDa::SoapyTX::execSetCommand(Command * cmd)
{
  switch(cmd->target) {
  case SoDa::Command::TX_MODE:
    tx_modulation = SoDa::Command::ModulationType(cmd->iparms[0]);
    if(tx_modulation == SoDa::Command::CW_L) {
      setCWFreq(false, CW_tone_freq); 
    }
    else if(tx_modulation == SoDa::Command::CW_U) {
      setCWFreq(true, CW_tone_freq); 
    }
    break; 
  case Command::TX_STATE:
    // TX_STATE must be 3 to turn the transmitter on.
    // bit 1 of the command indicates that CTRL has already done the
    // setup for TX <-> RX mode transitions.
    debugMsg(boost::format("TX_STATE %d\n") % cmd->iparms[0]);
    if((cmd->iparms[0] & 0x2) != 0) {  
      transmitSwitch(cmd->iparms[0] == 3);
      cmd_stream->put(new Command(Command::REP, Command::TX_STATE, tx_enabled ? 1 : 0));
    }
    break;
  case Command::TX_BEACON:
    beacon_mode = (cmd->iparms[0] != 0);
    break;
  case Command::TX_CW_EMPTY:
    debugMsg("set waiting_to_run_dry\n");
    waiting_to_run_dry = true; 
    break;
  default:
    break; 
  }
}

void SoDa::SoapyTX::execGetCommand(Command * cmd)
{
  switch(cmd->target) {
  case Command::TX_STATE:
    cmd_stream->put(new Command(Command::REP, Command::TX_STATE, tx_enabled ? 1 : 0)); 
    break;
  default:
    break; 
  }
}

void SoDa::SoapyTX::execRepCommand(Command * cmd)
{
  switch(cmd->target) {
  case SoDa::Command::TVRT_LO_CONFIG:
    debugMsg("LO configured");
    LO_configured = true; 
    break;
  default:
    break;
  }
}
