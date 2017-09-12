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

  beacon_mode = false; 

  // create the tx buffer streamers.
  std::vector<size_t> channel_nums;
  channel_nums.push_back(0);  
  tx_bits = radio->setupStream(SOAPY_SDR_TX, "CF32", channel_nums); 
#if 0
  int stat = radio->deactivateStream(tx_bits);
  if(stat < 0) debugMsg(boost::format("constructor: deactivateStream returns [%d] [%s]\n") % stat % SoapySDR::errToStr(stat));          
#endif  
  tx_activated = false; 

  radio->setDCOffsetMode(SOAPY_SDR_TX, 0, false); // set to automatic compensation
  std::cerr << "Temporary fix for DC OFFSET\n";
  radio->setDCOffset(SOAPY_SDR_TX, 0, std::complex<double>(-0.6, 0.1));
  // radio->setIQBalance(SOAPY_SDR_TX, 0, std::complex<double>(0.999563,0.000293756));          

  // find out how to configure the transmitter
  tx_sample_rate = params->getTXRate();
  tx_buffer_size = params->getRFBufferSize();

  // we need to put some artificial backpressure on sending, since the Lime support
  // for SoapySDR has very very deep buffers (18 seconds at 625ksamp/sec!)  
  seconds_per_sample = 1.0 / ((double) tx_sample_rate); 
  // setup targets for flow control
  target_backlog = 1 * lround(tx_sample_rate); // one second of backlog? 
  undershoot_backlog = (target_backlog * 3) / 4; 


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
  insert_delay_actual_histo = new SoDa::Histogram(1000, 1e-3, 1.0);
  insert_delay_wait_histo = new SoDa::Histogram(1000, 1e-3, 1.0);

  tx_enabled = false;

  // send a zero buff for just a few samples to clear the TX
  sendBuffer(zero_buf, 10); 
}

void SoDa::SoapyTX::run()
{

  bool exitflag = false;
  SoDaBuf * txbuf, * cwenv;
  Command * cmd; 
  std::complex<float> * tbuf; 

  debugMsg(boost::format("Start time = %g\n") % getTime()); 

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
      // We're in a modulation mode like USB, LSB, AM, NBFM, ...
      Acount++; 
      tbuf = txbuf->getComplexBuf(); 
      double ts = getTime();
      stat = sendBuffer(tbuf, txbuf->getComplexLen()); 
      double te = getTime();
      send_histo->updateTable(te - ts); 
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

      // std::cerr << "Temporary fix for DC OFFSET\n";
      // radio->setDCOffset(SOAPY_SDR_TX, 0, std::complex<double>(-0.6, 0.1));
      // radio->setIQBalance(SOAPY_SDR_TX, 0, std::complex<double>(0.999563,0.000293756));          
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
	stat = sendBuffer(cw_buf, tx_buffer_size); 
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
  insert_delay_actual_histo->writeTable("DelayActualHisto.dat");
  insert_delay_wait_histo->writeTable("DelayWaitHisto.dat");      

  std::ofstream dh("DelayHistory.lis");
  BOOST_FOREACH(auto & tp, delay_history) {
    unsigned long st, en; 
    double wt, sst;
    std::tie(st, en, wt, sst) = tp; 
    dh << boost::format("%ld %ld %g %g\n") % st % en % wt % sst; 
  }
  dh.close(); 

  debugMsg("Leaving\n");
}


void SoDa::SoapyTX::doCW(std::complex<float> * out, float * envelope, unsigned int env_len)
{
  unsigned int i;
  std::complex<float> c(1.0, 0.0);
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
  // send a zero filled buffer. 
  sendBuffer(zero_buf, tx_buffer_size, true);
  // now wait for the end of buffer marker
  int itercount = 0; 
  while(!lookForEOB(10000)) {
    itercount++; 
    if((itercount & 0xff) == 0) {
      std::cerr << boost::format("Still looking for EOB after %d iterations.\n") % itercount;
    }
    if(itercount > 1024) {
      std::cerr << "drainTXStream gives up after 1024 iterations." << std::endl;
      return false; 
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
    // reset the first_burst flag -- for buffer backlog book-keeping. 
    first_burst = true;

    std::complex<double> dcoff = radio->getDCOffset(SOAPY_SDR_TX, 0); 
    std::cerr << boost::format("TX DC Offset [%g %g]\n") % dcoff.real() % dcoff.imag();
    std::cerr << "Temporary fix for DC OFFSET\n";     
    radio->setIQBalance(SOAPY_SDR_TX, 0, std::complex<double>(0.999563,0.000293756));
    radio->setDCOffset(SOAPY_SDR_TX, 0, std::complex<double>(-0.6, 0.1));
    dcoff = radio->getDCOffset(SOAPY_SDR_TX, 0); 
    std::cerr << boost::format("NOW TX DC Offset [%g %g]\n") % dcoff.real() % dcoff.imag();
    
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

/** 
 * @page lime_buffer_management SoapyTX Buffer Management for LimeSDR
 * 
 * Buffer management in SoapySDR for the LimeSDR platform is complicated by 
 * the fact that there is very little backpressure from the writeStream call. 
 * In the implementation circa May 2017, buffers are sent through writeStream 
 * to the Lime hardware until about 18seconds (!) (at 625ksamp/S) have been 
 * queued up somwhere between the library and the hardware.   There is no 
 * control flow information available from the hardware that passes through the
 * SoapySDR interface. 
 * 
 * This is spectacularly inconvenient in CW mode where we process the CW 
 * transmit envelope as it arrives and pass it through the pipe.  If we get
 * 20 seconds of CW, then we send 20 seconds of CW right on through and commit
 * ourselves to 20 seconds of transmit stream.  Ooops... This isn't such a 
 * problem in SSB, as we only originate tx packets as fast as the audio arrives
 * from the input stream.  (Note this is likely to be a problem if the audio
 * stream originates from a modem or some other non-microphone source.)
 *
 * So, the rest of this only applies in the CW modes. 
 *
 * The libuhd version of the SoDa TX unit doesn't have this problem, as it
 * provided backpressure pretty early on.  (The send call is a blocking call and
 * returns once the buffer is committed to the transmitter.  However, the libuhd
 * queues are apparently very shallow.) 
 * 
 * For this reason, we need to implement some book-keeping that maintains a count
 * of samples transmitted, and samples passed. We need to keep ahead of the hardware
 * by a modest amount, but it must not stretch out with time, or else we'll end up
 * with a few seconds in the buffer and no way out.  (In practice, this is unachievable, 
 * since the TX unit in the Lime is running off of a clock that is different from the
 * TX unit in the host.  Yes, we could use the hardware clock, but its resolution and
 * its behavior is not well documented. (For instance, it doesn't run if the RX streamer
 * is stopped.(!)). 
 *
 * We'll manage the flow control in the sendBuffer method.  Only the sendBuffer method
 * will ever call writeStream.  Here's how it works. 
 * 
 * Dramatis personae:
 * Name | Description
 * -----|------------
 * `start_of_tx_time` | Time after the first return from writeStream after tx_on
 * `cur_tx_time` | Time after return from writeStream for the current buffer
 * `samples_sent` | Number of samples passed to writeStream since last tx_on
 * `samples_consumed` | Number of samples consumed by the passage of time since last tx_on
 * `samples_outstanding` | Difference between `samples_sent` and `samples_consumed`
 * `target_backlog` | Number of samples we wish to maintain "in flight" 
 * 
 *
 * Once we initiate the first transmission after the transmitter is enabled, we will
 * set a timestamp from the `getTime()` call that marks the start of transmission -- this
 * is called `start_of_tx_time`. We set `samples_sent` to the length of the first 
 * transmitted buffer. 
 *
 * After each writeStream  we call `getTime()` and subtract the `start_of_tx_time`
 * from it.  This delta is used to calculate the number of samples that have been 
 * "consumed" by the passage of time: `samples_consumed`.  We add the number of samples we 
 * just wrote to get `samples_sent` and then subtract `samples_consumed` to get 
 * `samples_outstanding`.
 * 
 * If the `samples_outstanding` is greater than `target_backlog` we calculate a wait
 * time to bring us back to about 3/4 of the target_backlog samples in the buffer. 
 * We sleep for about that amount of time, and move on. 
 */
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

  // in SSB, AM, or FM modes we use the incoming audio stream for buffer flow management
  if((tx_modulation != SoDa::Command::CW_U) && (tx_modulation != SoDa::Command::CW_L)) {
    return len; 
  }

  double cur_tx_time = getTime(); ///< Time after return from writeStream for the current buffer  
  if(first_burst) {
    start_of_tx_time = cur_tx_time; 
    samples_sent = len; 
    first_burst = false; 
  }
  else {
    samples_sent += len; 
  }

  long samples_consumed = lround((cur_tx_time - start_of_tx_time) * tx_sample_rate); 
  long samples_outstanding = samples_sent - samples_consumed; 
  if(samples_outstanding > target_backlog) {
    double target_time = cur_tx_time + seconds_per_sample * ((double) (samples_outstanding - undershoot_backlog)); 
    while(getTime() < target_time) {
      usleep(10); 
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
      // if this is a mode change while we're in TX mode, setup the buffer management.
      first_burst = true; 
      setCWFreq(false, CW_tone_freq); 
    }
    else if(tx_modulation == SoDa::Command::CW_U) {
      // if this is a mode change while we're in TX mode, setup the buffer management.
      first_burst = true; 
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
