/*
  Copyright (c) 2026, Matthew H. Reilly (kb1vc)
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

#include "RadioControl.hxx"
#include "SoDaBase.hxx"
#include <SoDa/Format.hxx>

#include <sys/time.h>

namespace SoDa {
  const double RadioControl::tx_freq_rxmode_offset = 1.0e6;

  RadioControl::RadioControl(Params * _params
			     const std::string & thread_name) : Thread(thread_name) {
    // initialize variables
    last_rx_req_freq = 0.0; // at least this is a number...
    tx_on = false;
    first_gettime = 0.0;
    rx_rf_gain = 0.0;
    tx_rf_gain = 0.0;
    tx_samp_rate = 625000;
  
    params = _params;

    // we need a cmd stream, but that's going to be setup by subscribeToMailBox
    cmd_stream = NULL; 
  }
  
  /// Subscribe to the command stream. 
  void RadioControl::subscribeToMailBox(const std::string & mbox_name, 
					BaseMBox * mbox_p) {
    if(mbox_name == "CMD") {
      CmdMBox * _cmd_stream = dynamic_cast<CmdMBox *>(mbox_p);
      if(_cmd_stream != NULL) {
	cmd_stream = _cmd_stream;

	// subscribe to the command stream.
	subid = cmd_stream->subscribe();
      }
      else {
	throw SDR::Exception(Format("Bad mailbox pointer for mailbox named = [%0]\n") 
			       .addS(mbox_name) , this);	
      }
    }
  }


  void RadioControl::run()
  {
    
    if(cmd_stream == NULL) {
      throw SDR::Exception(Format("Never got command stream subscription\n"), 
			     this);	
    }
    // I think this is the right place for this....
    cmd_stream->put(new Command(Command::REP, Command::INIT_SETUP_COMPLETE, 0));
    
    // do the initial commands
    cmd_stream->put(new Command(Command::SET, Command::RX_SAMP_RATE,
				params->getRXRate())); 
    cmd_stream->put(new Command(Command::SET, Command::TX_SAMP_RATE,
				params->getTXRate()));

    setSampleRate(params->getRXRate(), SoDa::RX);
    setSampleRate(params->getTXRate(), SoDa::TX);    
    
    cmd_stream->put(new Command(Command::SET, Command::RX_ANT, 
				params->getRXAnt())); 
    debugMsg(Format("Sending TX_ANT as [%0]\n").addS(params->getTXAnt()));
    cmd_stream->put(new Command(Command::SET, Command::TX_ANT,
				params->getTXAnt()));
    cmd_stream->put(new Command(Command::SET, Command::CLOCK_SOURCE,
				params->getClockSource())); 

    cmd_stream->put(new Command(Command::SET, Command::TX_RF_GAIN, 0.0)); 
    cmd_stream->put(new Command(Command::SET, Command::RX_RF_GAIN, 0.0));
    cmd_stream->put(new Command(Command::SET, Command::RX_AF_GAIN, 0.0));

    // transmitter is off
    tx_on = false; 
    cmd_stream->put(new Command(Command::SET, Command::TX_STATE, 0)); 
  
    bool exitflag = false;
    unsigned int cmds_processed = 0;
    unsigned int loopcount = 0; 
    while(!exitflag) {
      loopcount++; 
      Command * cmd = cmd_stream->get(subid);
      if(cmd == NULL) {
	sleep_ms(50);
      }
      else {
	// process the command.
	if((cmds_processed & 0xff) == 0) {
	  debugMsg(Format("RadioControl processed %0 commands").addI(cmds_processed));
	}
	cmds_processed++; 
	execCommand(cmd);
	exitflag |= (cmd->target == Command::STOP); 
	cmd_stream->free(cmd); 
      }
    }
  }

  double RadioControl::getTime()
  {
    double ret; 
    struct timeval tv;
    gettimeofday(&tv, NULL);
    ret = (((double) tv.tv_sec) - first_gettime) + 1.0e-6*((double) tv.tv_usec);
    return ret; 
  }

  void RadioControl::execCommand(Command * cmd)
  {
    switch (cmd->cmd) {
    case Command::GET:
      execGetCommand(cmd); 
      break;
    case Command::SET:
      execSetCommand(cmd); 
      break; 
    case Command::REP:
      execRepCommand(cmd); 
      break;
    default:
      break; 
    }
  }

  /**
   * exeSetCommand handles SET messages of the following type:
   * @li RX_TUNE_FREQ sets the FE chain frequency to the specified value.
   * @li LO_CHECK set the FE chain to the specified frequency. This is used in
   * calibrating a transverter chain by "tuning" to a frequency near the transverter
   * LO and listening to the leakage signal.  (It's a long story.)
   * @li TX_TUNE_FREQ all set the transmit FE chain frequency
   * to the requested value PLUS the tx_freq_rxmode_offset (the transmit IF frequency).
   * @li RX_SAMP_RATE set the receive A/D sample rate in the radio
   * @li TX_SAMP_RATE set the transmit D/A sample rate in the radio
   * @li RX_RF_GAIN set the RF gain of the receive front-end
   * @li TX_RF_GAIN set the gain of the transmit front-end
   * @li TX_STATE turn the transmitter ON or OFF
   * @li CLOCK_SOURCE select the external frequency reference for the radio or the internal clock.
   * @li RX_ANT set the receive antenna port
   * @li TX_ANT set the transmit antenna port
   */
  void RadioControl::execSetCommand(Command * cmd)
  {
    double freq, fdiff; 
    if(cmd->cmd != Command::SET) {
      std::cerr << "execSetCommand got a non-set command!  " << cmd->toString() << std::endl;
      return; 
    }
    double tmp;
    switch (cmd->target) {
    case Command::RX_TUNE_FREQ:
      setFreq(cmd->dparms[0], SoDa::RX, char(cmd->dparms[1]));
      break; 

    case Command::LO_CHECK:
      if(cmd->dparms[0] == 0.0) {
	setLOFreq(last_rx_lo_freq, SoDa::RX);
      }
      else {
	debugMsg(Format("setting lo check freq to %0\n") .addF(cmd->dparms[0], 10, 6, 'e'));
	setLOFreq(cmd->dparms[0], SoDa::RX);
	cmd_stream->put(new Command(Command::GET, Command::LO_OFFSET, 0));
      }
      break;

    case Command::TX_TUNE_FREQ:
      setFreq(cmd->dparms[0], SoDa::TX);
      cmd_stream->put(new Command(Command::REP, Command::TX_TUNE_FREQ, 
				  cmd->dparms[0]));
      break; 

    case Command::RX_SAMP_RATE:
      setSampleRate(cmd->dparms[0], SoDa::RX);
      cmd_stream->put(new Command(Command::REP, Command::RX_SAMP_RATE, 
				  getSampleRate(SoDa::RX)));
      break; 
    case Command::TX_SAMP_RATE:
      tx_samp_rate = cmd->dparms[0]; 
      cmd_stream->put(new Command(Command::REP, Command::TX_SAMP_RATE, 
				  getSampleRate(SoDa::TX)));
      break;
    
    case Command::RX_RF_GAIN:
      // dparameters ranges from 0 to 100... normalize this
      // to the actual range;
      {
	auto gain = setRFGain(cmd->dparms[0], SoDa::RX);
	cmd_stream->put(new Command(Command::REP, Command::RX_RF_GAIN, 
				    gain));
      }
      break; 
    case Command::TX_RF_GAIN:
      {
	auto gain = setRFGain(cmd->dparms[0], SoDa::TX);
	cmd_stream->put(new Command(Command::REP, Command::TX_RF_GAIN, 
				    gain));
      }
      break; 
    case Command::TX_STATE: // SET Command::TX_ON
      debugMsg(Format("TX_STATE arg = %0\n").addI(cmd->iparms[0]));
      if(cmd->iparms[0] == Command::TX_ON_0) {
	// we're going to TX mode.
	// This is the first stage.
	setTXRXMode(Command::TX_ON_0, full_duplex);
      }
      if(cmd->iparms[0] == 0) {
	// going to receive mode
	setTXRXMode(Command::TX_OFF_0, full_duplex);
      }
      break; 

    case Command::RX_ANT:
      setAntenna(cmd->sparm, SoDa::RX);
      cmd_stream->put(new Command(Command::REP, Command::RX_ANT, getAntenna(SoDa::RX)));
      break; 

    case Command::TX_ANT:
      tx_ant = cmd->sparm; 
      setAntenna(cmd->sparm, SoDa::TX);
      cmd_stream->put(new Command(Command::REP, Command::TX_ANT, getAntenna(SoDa::TX)));
      break;

    case Command::CLOCK_SOURCE:
      setClockSource(cmd->iparms[0]);
      break;
      
    default:
      break; 
    }

    subExecSetCommand(cmd);
  }

  void RadioControl::execGetCommand(Command * cmd)
  {
    int res;
  
    switch (cmd->target) {
    case Command::RX_TUNE_FREQ:
      cmd_stream->put(new Command(Command::REP, Command::RX_TUNE_FREQ, 
				  cur_rx_freq));
      break; 
    case Command::TX_TUNE_FREQ:
      cmd_stream->put(new Command(Command::REP, Command::TX_TUNE_FREQ, 
				  cur_tx_freq));
      break; 

    case Command::RX_LO_FREQ:
      cmd_stream->put(new Command(Command::REP, Command::RX_LO_FREQ, 
				  cur_rx_lo_freq));
      break; 
    case Command::TX_LO_FREQ:
      cmd_stream->put(new Command(Command::REP, Command::TX_LO_FREQ, 
				  cur_tx_lo_freq));
      break; 
      
    case Command::RX_SAMP_RATE:
      cmd_stream->put(new Command(Command::REP, Command::RX_SAMP_RATE, 
				  getSampleRate(SoDa::RX)));
      break; 
    case Command::TX_SAMP_RATE:
      cmd_stream->put(new Command(Command::REP, Command::TX_SAMP_RATE, 
				  getSampleRate(SoDa::TX)));
      break;

    case Command::TX_GAIN_RANGE:
      cmd_stream->put(new Command(Command::REP, Command::TX_GAIN_RANGE,
				  getGainRange(SoDa::TX)));
      break; 

    case Command::RX_GAIN_RANGE:
      cmd_stream->put(new Command(Command::REP, Command::TX_GAIN_RANGE,
				  getGainRange(SoDa::RX)));
      break; 
      
    case Command::CLOCK_SOURCE:
	  cmd_stream->put(new Command(Command::REP, Command::CLOCK_SOURCE,
				      getClockSource()));
	  break; 
    case Command::HWMB_REP:
      cmd_stream->put(new Command(Command::REP, Command::HWMB_REP,
				  getHardwareDescription));
      reportAntennas();
      cmd_stream->put(new Command(Command::GET, Command::LIST_MODES));
      cmd_stream->put(new Command(Command::GET, Command::LIST_AF_FILTERS));    
      break; 
    default:
      break; 
    }

    subExecGetCommand(cmd);
  }

  void RadioControl::execRepCommand(Command * cmd)
  {
    subExecRepCommand(cmd);
  }


  void RadioControl::reportAntennas() 
  {
    std::vector<std::string> rx_ants = listAntennas(SoDa::RX);
    for(auto ant: rx_ants) {
      debugMsg(Format("Sending RX antenna list element [%0]\n")
	       .addS(ant));
      cmd_stream->put(new Command(Command::REP, Command::RX_ANT_NAME, 
				  ant)); 
    }
    std::vector<std::string> tx_ants = listAntennas(SoDa::TX);
    for(auto ant: tx_ants) {
      debugMsg(Format("Sending TX antenna list element [%0]\n")
	       .addS(ant));
      cmd_stream->put(new Command(Command::REP, Command::TX_ANT_NAME, 
				  ant)); 

    }
  }

  // return -1 if we don't need to reset the LO
  double RadioControl::findGoodRXLO(double freq, double cur_lo_freq) {
    auto diff = freq - cur_freq;
    if((diff > 100e3) && (diff < 200e3)) {
      return -1.0; 
    }
    else {
      // pick a spot half way between so that the IF ends up around
      // 150 kHz.
      return freq - 150e3; 
    }
  }

  void RadioControl::setFreq(double freq, SoDa::RXTX rxtx) {
    // is this TX or RX?
    if(rxtx == SoDa::TX) {
      // ask the device controller to set the TX lo.  Normally this should
      // be equal to freq and actual_lo_freq would be freq
      cur_tx_lo_freq = setLOFreq(freq, SoDa::TX);
      new_tx_if_freq = freq - cur_tx_lo_freq;
      cur_tx_freq = freq;
      // tell the TX IF to set its new IF frequency. Then wait for it
      // to respond with a report. 
      cmd_stream->put(new Command(Command::SET, Command::TX_IF_FREQ, new_tx_if_freq));
      cmd_stream->put(new Command(Command::REP, Command::TX_LO_FREQ,
				  cur_tx_lo_freq));
      cmd_stream->put(new Command(Command::REP, Command::TX_TUNE_FREQ,
				  cur_tx_lo_freq + new_tx_if_freq));
    }
    else { // rxtx is RX
      // if the requested frequency is between 100 kHz and 200 kHz above the LO
      // we don't need to reset the LO, just the IF.
      auto fe_freq = findGoodRXLO(freq, cur_rx_lo_freq); 
      if(fe_freq > 0.0) {
	// retune the front end.
	cur_rx_lo_freq = setLOFreq(fe_freq, SoDa::RX);
	// we also need to tell everyone about the new center frequency for the IF stream
	cmd_stream->put(new Command(Command::REP, Command::RX_CENTER_FREQ, 
				    cur_rx_lo_freq));
      }

      // change the IF frequency    
      new_rx_if_freq = freq - cur_rx_lo_freq;
      // tell the RX IF to set its new IF frequency. Then wait for it
      // to respond with a report. 
      cmd_stream->put(new Command(Command::SET, Command::RX_IF_FREQ, new_rx_if_freq));
      cmd_stream->put(new Command(Command::REP, Command::RX_TUNE_FREQ, 
				  cur_rx_lo_freq + new_rx_if_freq));
      cmd_stream->put(new Command(Command::REP, Command::RX_LO_FREQ, 
				  cur_rx_lo_freq));
      
    }
  }
  
  bool RadioControl::isLocked(SoDa::RXTX rxtx) {
    if(rxtx == SoDa::RX) {
      return (cur_rx_lo_freq == getLOFreq(SoDa::RX)) && isLOLocked(SoDa::TX);
    }
    else {
      return (cur_rx_lo_freq == getLOFreq(SoDa::TX)) && isLOLocked(SoDa::TX);
    }
  }

  void RadioControl::setTXRXMode(RxTxState rxtxst, bool full_duplex) {
    if(rxtxst == Command::TX_ON_0) {
      //   1. X turn the RX gain to 0 (temporarily)
      if(!full_duplex) {
	setRFGain(0.0, SoDa::RX);
      }

      //   2. X set the TX gain to its current requested level
      setRFGain(tx_rf_gain, SoDa::TX);
      
      //   3. X Report the tx gain to the world
      cmd_stream->put(new Command(Command::REP, Command::TX_RF_GAIN, 
				  getGain(SoDa::TX)));

      //   4. X Set the tx local oscillator (in case it was in "tune remote mode"
      setLOFreq(cur_tx_lo_freq, SoDa::TX);

      //   5.   Enable the rest of the tx hardware, including the antenna relay.
      setTXEna(true, full_duplex);

      //   6. X Send a SET message to put Command::TX_ON_1, full_duplex_flag
      // This will tell the RX unit to shut down (unless it is in full-dux) and then
      // the RX unit will send a Command::TX_ON_2 message to the TX unit. 
      // This avoids the race between CTRL and TX/RX units for setup and teardown....
      cmd_stream->put(new Command(Command::SET, Command::TX_STATE, 
				  Command::TX_ON_1, cmd->iparms[1]));

    }
    else if(rxtxst == Command::TX_OFF_0) {

      // 1. X Sets tx gain to 0
      setRFGain(0.0, SoDa::TX);
      
      // 2.   Disable the transmitter and flip the antenna relay
      setTXEna(false, full_duplex);            

      // 3. X Sets rx gain to current level
      setRFGain(rx_rf_gain, SoDa::RX);

      // 4. X Sets tx frequency to tx_freq + an offset that gets the tx LO out of the RX passband
      setLOFreq(cur_tx_lo_freq + tx_freq_rxmode_offset, SoDa::TX);
      
      // 6. X Send SET with Command::TX_OFF_1
      // and tell the RX unit to turn on the RX
      // This avoids the race between CTRL and TX/RX units for setup and teardown.... 
      cmd_stream->put(new Command(Command::SET, Command::TX_STATE, 
				  Command::TX_OFF_1, full_duplex));
    }
  }
  
  double RadioControl::getTime()
  {
    double ret; 
    struct timeval tv;
    gettimeofday(&tv, NULL);
    ret = (((double) tv.tv_sec) - first_gettime) + 1.0e-6*((double) tv.tv_usec);
    return ret; 
  }

  bool RadioControl::setClockSource(Command::ClockSource src) {
    return true; 
  }

  Command::ClockSource RadioControl::getClockSource() {
    return Command::ClockSource::INTERNAL; 
  }
}
