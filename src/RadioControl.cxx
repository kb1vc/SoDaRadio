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

  RadioControl::RadioControl(ParamsPtr _params,
			     const std::string & thread_name) : Thread(thread_name) {
    // initialize variables
    tx_on = false;
    first_gettime = 0.0;
    rx_rf_gain = 0.0;
    tx_rf_gain = 0.0;
    tx_samp_rate = 625000;
    cur_rx_lo_hw = 0.0;
    cur_rx_lo_sw = 0.0;    
    cur_tx_lo_hw = 0.0;
    cur_tx_freq = 0.0;

    // set it to something.  we'll get real info soon. 
    tx_modulation = Command::CW_U;
    
    params = _params;

    // we need a cmd stream, but that's going to be setup by subscribeToMailBox
    cmd_stream = NULL; 
  }
  
  /// Subscribe to the command stream. 
  void RadioControl::subscribeToMailBoxes(const std::vector<MailBoxBasePtr> & mailboxes) {
    for(auto mbox_p : mailboxes) {
      MailBoxBase::connect<MailBox<CommandPtr>>(mbox_p,
					       "CMDstream",
					       cmd_stream);
    }

    if(cmd_stream == nullptr) {
      throw MissingMailBox("CMD", getSelfPtr());    
    }
    else {
      cmd_subs = cmd_stream->subscribe();
    }
  }


  void RadioControl::run()
  {
    if(cmd_stream == NULL) {
      throw SDR::Exception(Format("Never got command stream subscription\n"),
			   getSelfPtr());
    }
    // INIT_SETUP_COMPLETE used to fire here, but the GUI uses it to trigger
    // restoreSettings(), which needs the Mode / AF-filter comboboxes to be
    // populated first.  Those entries only arrive after the GUI requests
    // HWMB_REP -> LIST_MODES / LIST_AF_FILTERS, which happens later.  So the
    // emission is now at the end of BaseBandRX::reportAFFilters().

    // do the initial commands
    cmd_stream->put(Command::make(Command::SET, Command::RX_SAMP_RATE,
				params->getRXRate())); 
    cmd_stream->put(Command::make(Command::SET, Command::TX_SAMP_RATE,
				params->getTXRate()));

    setSampleRate(params->getRXRate(), SoDa::RX);
    setSampleRate(params->getTXRate(), SoDa::TX);    
    
    cmd_stream->put(Command::make(Command::SET, Command::RX_ANT, 
				params->getRXAnt())); 
    debugMsg(Format("Sending TX_ANT as [%0]\n").addS(params->getTXAnt()));
    cmd_stream->put(Command::make(Command::SET, Command::TX_ANT,
				params->getTXAnt()));
    cmd_stream->put(Command::make(Command::SET, Command::CLOCK_SOURCE,
				params->getClockSource())); 

    cmd_stream->put(Command::make(Command::SET, Command::TX_RF_GAIN, 0.0)); 
    cmd_stream->put(Command::make(Command::SET, Command::RX_RF_GAIN, 0.0));
    cmd_stream->put(Command::make(Command::SET, Command::RX_AF_GAIN, 50.0));

    // transmitter is off
    tx_on = false; 
    cmd_stream->put(Command::make(Command::SET, Command::TX_STATE, 0)); 
  
    bool exitflag = false;
    unsigned int cmds_processed = 0;
    unsigned int loopcount = 0; 
    while(!exitflag) {
      loopcount++; 
      CommandPtr cmd;
      if(!cmd_stream->get(cmd_subs, cmd)) {
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

  void RadioControl::execCommand(CommandPtr cmd)
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
  void RadioControl::execSetCommand(CommandPtr cmd)
  {
    double freq, fdiff; 
    if(cmd->cmd != Command::SET) {
      std::cerr << "execSetCommand got a non-set command!  " << cmd->toString() << std::endl;
      return; 
    }
    double tmp;
    switch (cmd->target) {
    case Command::TX_MODE:
      // we only care about CW mode or NOT CW mode.
      tx_modulation = Command::ModulationType(cmd->iparms[0]);
      break; 
      
    case Command::RX_TUNE_FREQ:
      setFreq(cmd->dparms[0], SoDa::RX);
      break; 

    case Command::LO_CHECK:
      if(cmd->dparms[0] == 0.0) {
	setLOFreq(cur_rx_lo_hw, SoDa::RX);
      }
      else {
	debugMsg(Format("setting lo check freq to %0\n") .addF(cmd->dparms[0], 'e', 10, 6));
	setLOFreq(cmd->dparms[0], SoDa::RX);
	cmd_stream->put(Command::make(Command::GET, Command::LO_OFFSET, 0));
      }
      break;

    case Command::TX_TUNE_FREQ:
      setFreq(cmd->dparms[0], SoDa::TX);
      cmd_stream->put(Command::make(Command::REP, Command::TX_TUNE_FREQ, 
				  cmd->dparms[0]));
      break; 

    case Command::RX_SAMP_RATE:
      setSampleRate(cmd->dparms[0], SoDa::RX);
      cmd_stream->put(Command::make(Command::REP, Command::RX_SAMP_RATE, 
				  getSampleRate(SoDa::RX)));
      break; 
    case Command::TX_SAMP_RATE:
      tx_samp_rate = cmd->dparms[0]; 
      cmd_stream->put(Command::make(Command::REP, Command::TX_SAMP_RATE, 
				  getSampleRate(SoDa::TX)));
      break;
    
    case Command::RX_RF_GAIN:
      {
	rx_rf_gain = cmd->dparms[0];
	auto gain = setRFGain(rx_rf_gain, SoDa::RX);
	cmd_stream->put(Command::make(Command::REP, Command::RX_RF_GAIN, gain));
      }
      break;
    case Command::TX_RF_GAIN:
      {
	tx_rf_gain = cmd->dparms[0];
	auto gain = setRFGain(tx_rf_gain, SoDa::TX);
	cmd_stream->put(Command::make(Command::REP, Command::TX_RF_GAIN, gain));
      }
      break;
    case Command::TX_STATE: // SET Command::TX_ON
      debugMsg(Format("TX_STATE arg = %0\n").addI(cmd->iparms[0]));
      if(cmd->iparms[0] == Command::TX_ON_0) {
	// we're going to TX mode.
	// This is the first stage.
	setTXRXMode(Command::TX_ON_0, cmd->iparms[1]);
      }
      if(cmd->iparms[0] == Command::TX_OFF_0) {
	// going to receive mode
	setTXRXMode(Command::TX_OFF_0, cmd->iparms[1]);
      }
      break; 

    case Command::RX_ANT:
      setAntenna(cmd->sparm, SoDa::RX);
      cmd_stream->put(Command::make(Command::REP, Command::RX_ANT, 
				    getAntenna(SoDa::RX)));
      break; 

    case Command::TX_ANT:
      tx_ant = cmd->sparm; 
      setAntenna(cmd->sparm, SoDa::TX);
      cmd_stream->put(Command::make(Command::REP, Command::TX_ANT, 
				    getAntenna(SoDa::TX)));
      break;

    case Command::CLOCK_SOURCE:
      setClockSource(static_cast<Command::ClockSource>(cmd->iparms[0]));
      break;
      
    default:
      break; 
    }

    subExecSetCommand(cmd);
  }

  void RadioControl::execGetCommand(CommandPtr cmd)
  {
    int res;

    switch (cmd->target) {
    case Command::RX_TUNE_FREQ:
      cmd_stream->put(Command::make(Command::REP, Command::RX_TUNE_FREQ, 
				  cur_rx_lo_hw + cur_rx_lo_sw));
      break; 
    case Command::TX_TUNE_FREQ:
      cmd_stream->put(Command::make(Command::REP, Command::TX_TUNE_FREQ, 
				    cur_tx_freq));
      break; 

    case Command::RX_LO_FREQ: 
      cmd_stream->put(Command::make(Command::REP, Command::RX_LO_FREQ, 
				  cur_rx_lo_hw));
      break; 
    case Command::TX_LO_FREQ:
      cmd_stream->put(Command::make(Command::REP, Command::TX_LO_FREQ, 
				  cur_tx_lo_hw));
      break; 
      
    case Command::RX_SAMP_RATE:
      cmd_stream->put(Command::make(Command::REP, Command::RX_SAMP_RATE, 
				  getSampleRate(SoDa::RX)));
      break; 
    case Command::TX_SAMP_RATE:
      cmd_stream->put(Command::make(Command::REP, Command::TX_SAMP_RATE, 
				  getSampleRate(SoDa::TX)));
      break;

    case Command::TX_GAIN_RANGE:
      cmd_stream->put(Command::make(Command::REP, Command::TX_GAIN_RANGE,
				    -30.0, 0.0));
      break; 

    case Command::RX_GAIN_RANGE:
      cmd_stream->put(Command::make(Command::REP, Command::TX_GAIN_RANGE,
				    -30.0, 0.0));
      break; 
      
    case Command::CLOCK_SOURCE:
	  cmd_stream->put(Command::make(Command::REP, Command::CLOCK_SOURCE,
				      getClockSource()));
	  break; 
    case Command::HWMB_REP:
      cmd_stream->put(Command::make(Command::REP, Command::HWMB_REP,
				    getHardwareDescription()));
      reportAntennas();
      cmd_stream->put(Command::make(Command::GET, Command::LIST_MODES));
      cmd_stream->put(Command::make(Command::GET, Command::LIST_AF_FILTERS));    
      break;
    case Command::TX_ANT:
      cmd_stream->put(Command::make(Command::REP, Command::TX_ANT,
				    getAntenna(SoDa::TX)));
      
      break; 
    case Command::RX_ANT:
      cmd_stream->put(Command::make(Command::REP, Command::RX_ANT,
				    getAntenna(SoDa::RX)));
      
      break; 
    default:
      break; 
    }

    subExecGetCommand(cmd);
  }

  void RadioControl::execRepCommand(CommandPtr cmd)
  {
    subExecRepCommand(cmd);
  }


  void RadioControl::reportAntennas() 
  {
    std::vector<std::string> rx_ants = listAntennas(SoDa::RX);
    for(auto ant: rx_ants) {
      debugMsg(Format("Sending RX antenna list element [%0]\n")
	       .addS(ant));
      cmd_stream->put(Command::make(Command::REP, Command::RX_ANT_NAME, 
				  ant)); 
    }
    std::vector<std::string> tx_ants = listAntennas(SoDa::TX);
    for(auto ant: tx_ants) {
      debugMsg(Format("Sending TX antenna list element [%0]\n")
	       .addS(ant));
      cmd_stream->put(Command::make(Command::REP, Command::TX_ANT_NAME, 
				  ant)); 

    }
  }

  // return -1 if we don't need to reset the LO
  double RadioControl::findGoodRXLO(double freq, double cur_lo_freq) {
    auto diff = freq - cur_lo_freq;
    double retval;
    if((diff > 100e3) && (diff < 200e3)) {
      retval = -1.0; 
    }
    else {
      // pick a spot half way between so that the IF ends up around
      // 150 kHz.
      retval = freq - 150e3; 
    }
    
    debugMsg(SoDa::Format("RadioControl::findGoodRXLO(%0, %1) returns %2\n")
	     .addF(freq, 'e', 6)
	     .addF(cur_lo_freq, 'e', 6)
	     .addF(retval, 'e', 6));

    return retval;
  }

  void RadioControl::setFreq(double freq, SoDa::RXTX rxtx) {
    // is this TX or RX?
    if(rxtx == SoDa::TX) {
      // We offset the transmit frequency in CW mode down for CW_U and
      // up for CW_L
      if(tx_modulation == Command::CW_L) {
	freq = freq - 0; // 500;
      }
      if(tx_modulation == Command::CW_U) {
	freq = freq + 0; //500;
      }

      // ask the device controller to set the TX lo.  Normally this should
      // be equal to freq and actual_lo_freq would be freq
      cur_tx_lo_hw = setLOFreq(freq, SoDa::TX);
      
      cur_tx_freq =  (freq - cur_tx_lo_hw);
      // tell the TX IF to set its new IF frequency. 
      cmd_stream->put(Command::make(Command::SET, Command::TX_IF_FREQ, cur_tx_freq));
      cmd_stream->put(Command::make(Command::REP, Command::TX_LO_FREQ,
				  cur_tx_lo_hw));
      cmd_stream->put(Command::make(Command::REP, Command::TX_TUNE_FREQ,
				  cur_tx_lo_hw + cur_tx_freq));
    }
    else { // rxtx is RX
      // OK... We have several frequencies in play.
      // 1. The requested (carrier) frequency  "freq"
      // 2. The hardware IF frequency (f_if_hw)
      // 3. The hardware LO frequency (cur_rx_lo_hw)
      // 4. The software LO frequency (cur_rx_lo_sw)
      //
      // We want our desired cur_rx_lo_hw to fall in a range such that
      // freq - cur_rx_lo_hw < 240 kHz and freq - cur_rx_lo_hw > 100 kHz.
      // This keeps the DC spike at least 100 kHz away from the
      // frequency of interest and below the 80% rolloff in the
      // sampling range -312.5 kHz to 312.5 kHz.
      //
      // If the requested frequency falls in the range, then we
      // don't need to change cur_rx_lo_hw. Otherwise, we pick
      // cur_rx_lo_hw so that it is 170 kHz below freq.
      //
      // We trust the hardware interface to give us the LO we
      // ask for, or at least something close. USRPs, for
      // instance, will tune their first RF lo to an integer multiple
      // of a reference clock, then use a CORDIC NCO on the fpga
      // to make up the difference.
      // first find the current cur_rx_lo_hw
      auto cur_rx_lo_hw = getLOFreq(SoDa::RX);
      // now test
      auto f_diff = freq - cur_rx_lo_hw; 
      if((f_diff < 240e3) && (f_diff > 100e3)) {
	// we only need to change the software lo that beats us down to baseband
	cur_rx_lo_sw = f_diff;
      }
      else {
	// we need to pick a new LO... It should be about 170 kHz below the
	// requested frequency
	cur_rx_lo_hw = freq - 170e3;
	// change the software lo
	cur_rx_lo_sw = freq - cur_rx_lo_hw; 
	// now set the hardware lo
	setLOFreq(cur_rx_lo_hw, SoDa::RX); 
	// we also need to tell everyone about the new center frequency for the IF stream
	cmd_stream->put(Command::make(Command::REP, Command::RX_CENTER_FREQ, 
				    cur_rx_lo_hw));
      }
      // tell the RX IF to set its new IF frequency. Then wait for it
      // to respond with a report. 
      cmd_stream->put(Command::make(Command::SET, Command::RX_IF_FREQ, cur_rx_lo_sw));
      cmd_stream->put(Command::make(Command::REP, Command::RX_TUNE_FREQ, 
				  cur_rx_lo_hw + cur_rx_lo_sw));
      cmd_stream->put(Command::make(Command::REP, Command::RX_LO_FREQ, 
				  cur_rx_lo_hw));
      
    }
  }
  
  bool RadioControl::isLocked(SoDa::RXTX rxtx) {
    if(rxtx == SoDa::RX) {
      return (cur_rx_lo_hw == getLOFreq(SoDa::RX)) && isLOLocked(SoDa::RX);
    }
    else {
      return (cur_rx_lo_hw == getLOFreq(SoDa::TX)) && isLOLocked(SoDa::TX);
    }
  }

  void RadioControl::setTXRXMode(Command::RxTxState rxtxst, bool full_duplex) {
    if(rxtxst == Command::TX_ON_0) {
      //   1. X turn the RX gain to 0 (temporarily)
      if(!full_duplex) {
	setRFGain(0.0, SoDa::RX);
      }

      //   2. X set the TX gain to its current requested level
      //   3. X Report the tx gain to the world
      auto gain = setRFGain(tx_rf_gain, SoDa::TX);
      cmd_stream->put(Command::make(Command::REP, Command::TX_RF_GAIN, gain));

      //   4. X Set the tx local oscillator (in case it was in "tune remote mode"
      setLOFreq(cur_tx_lo_hw, SoDa::TX);

      //   5.   Enable the rest of the tx hardware, including the antenna relay.
      setTXEna(true, full_duplex);

      //   6. X Send a SET message to put Command::TX_ON_1, full_duplex_flag
      // This will tell the RX unit to shut down (unless it is in full-dux) and then
      // the RX unit will send a Command::TX_ON_2 message to the TX unit. 
      // This avoids the race between CTRL and TX/RX units for setup and teardown....
      cmd_stream->put(Command::make(Command::SET, Command::TX_STATE,
				  Command::TX_ON_1, 0));

    }
    else if(rxtxst == Command::TX_OFF_0) {

      // 1.   Mute TX to minimum gain and disable the transmitter / antenna relay.
      //      Do NOT call setRFGain(0.0, TX) first — on Pluto, 0 dB is maximum power.
      //      setTXEna(false) drives the gain to TX_GAIN_MIN (~-89.75 dB).
      setTXEna(false, full_duplex);

      // 2.   Shift the TX LO at least 1 MHz away from the RX passband immediately.
      setLOFreq(cur_tx_lo_hw + tx_freq_rxmode_offset, SoDa::TX);

      // 3. X Sets rx gain to current level
      setRFGain(rx_rf_gain, SoDa::RX);

      // 4. X Send SET with Command::TX_OFF_1
      // and tell the RX unit to turn on the RX
      // This avoids the race between CTRL and TX/RX units for setup and teardown....
      cmd_stream->put(Command::make(Command::SET, Command::TX_STATE,
				  Command::TX_OFF_1, 0));
    }
    else if(rxtxst == Command::TX_OFF_2) {
      // LO offset already applied in TX_OFF_0; nothing else needed here.
    }
    
  }
  
  bool RadioControl::setClockSource(Command::ClockSource src) {
    return true; 
  }

  Command::ClockSource RadioControl::getClockSource() {
    return Command::ClockSource::INTERNAL; 
  }
}
