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

#include "USRPCtrl.hxx"
#include "SoDaBase.hxx"
#include <uhd/utils/thread_priority.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/types/tune_request.hpp>
#include <uhd/types/tune_result.hpp>
#include <boost/format.hpp>


// Mac OSX doesn't have a clock_gettime, it has
// the microsecond resolution gettimeofday. 
#include <sys/time.h>

const unsigned int SoDa::USRPCtrl::TX_RELAY_CTL = 0x1000;
const unsigned int SoDa::USRPCtrl::TX_RELAY_MON = 0x0800;


SoDa::USRPCtrl::USRPCtrl(Params * _params, CmdMBox * _cmd_stream) : SoDa::SoDaThread("USRPCtrl")
{
  // turn off all the babbling, by default. 
  debug_mode = false;
  
  cmd_stream = _cmd_stream;
  params = _params;

  // subscribe to the command stream.
  subid = cmd_stream->subscribe();
  
  // make the device.
  usrp = uhd::usrp::multi_usrp::make(params->getUHDArgs());

  if(usrp == NULL) {
    throw SoDaException((boost::format("Unable to allocate USRP unit with arguments = [%]\n") % params->getUHDArgs()).str(), this);
  }
  
  first_gettime = 0.0;
  double tmp = getTime();
  // remove the whole number of seconds -- paranoia
  first_gettime = floor(tmp); 

  // get the tx front end subtree
  uhd::property_tree::sptr tree = usrp->get_device()->get_tree();
  const std::string mbname = tree->list("/mboards").at(0);
  uhd::fs_path tx_fe_root = "/mboards/" + mbname + "/dboards/A/tx_frontends/0";
  tx_fe_subtree = tree->subtree(tx_fe_root); 
  
  // setup the control IO pins (for TX/RX external relay)
  initControlGPIO();

  // turn off the transmitter
  setTXEna(false); 
}


void SoDa::USRPCtrl::run()
{
  uhd::set_thread_priority(); 
  // now do the event loop.  we watch
  // for commands and responses on the command stream.
  
  // do the initial commands
  cmd_stream->put(new Command(Command::SET, Command::RX_SAMP_RATE,
			     params->getRXRate())); 
  cmd_stream->put(new Command(Command::SET, Command::TX_SAMP_RATE,
			     params->getTXRate()));

  cmd_stream->put(new Command(Command::SET, Command::RX_ANT, 
			     params->getRXAnt())); 
  cmd_stream->put(new Command(Command::SET, Command::TX_ANT,
			     params->getTXAnt()));
  cmd_stream->put(new Command(Command::SET, Command::CLOCK_SOURCE,
			     params->getClockSource())); 

  cmd_stream->put(new Command(Command::SET, Command::TX_RF_GAIN, 100.0)); 
  cmd_stream->put(new Command(Command::SET, Command::RX_RF_GAIN, 10.0));

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
      boost::this_thread::sleep(boost::posix_time::milliseconds(50));
    }
    else {
      // process the command.
      if(debug_mode && ((cmds_processed & 0xff) == 0)) {
	std::cerr << "USRPCtrl processed "
		  << cmds_processed << " commands." << std::endl;
      }
      cmds_processed++; 
      execCommand(cmd);
      exitflag |= (cmd->target == Command::STOP); 
      cmd_stream->free(cmd); 
    }
    // if(loopcount == 100) {
    //   loopcount = 0;
    //   cmd_stream->put(new Command(Command::GET, Command::DBG_REP, (int) Command::AudioRX));
    // }
  }
}

double SoDa::USRPCtrl::getTime()
{
  double ret; 
  struct timeval tv;
  gettimeofday(&tv, NULL);
  ret = (((double) tv.tv_sec) - first_gettime) + 1.0e-6*((double) tv.tv_usec);
  return ret; 
}

void SoDa::USRPCtrl::execCommand(Command * cmd)
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

uhd::tune_result_t SoDa::USRPCtrl::checkLock(uhd::tune_request_t & req, char sel, uhd::tune_result_t & cur)
{
  int lock_itercount = 1;
  uhd::tune_result_t ret = cur; 
  while(1) {
    uhd::sensor_value_t lo_locked = (sel == 'r') ? usrp->get_rx_sensor("lo_locked",0) :
      usrp->get_tx_sensor("lo_locked",0);
    if(lo_locked.to_bool()) break;
    else usleep(1000);
    if((lock_itercount & 0xfff) == 0) {
      std::cerr << "waiting for LO lock, count = " << lock_itercount << " on " << sel << std::endl;
      if(sel == 'r') ret = usrp->set_rx_freq(req);
      else ret = usrp->set_tx_freq(req);
    }
    lock_itercount++; 
  }

  return ret; 
}

void SoDa::USRPCtrl::set1stLOFreq(double freq, char sel, bool set_if_freq)
{
  // select "r" for rx and "t" for tx.
  // We only want to tune for one band :: 2m 144 to 148.
  // well... not really... I'd like to tune to 432 as well.
  // well.... this really should be made to work for all freqs.
  // 
  
  double target_rx_freq = freq;
  
  uhd::tune_request_t tx_trequest(freq);
  if(sel == 'r') {
    // we round the target frequency to a point that puts the
    // baseband between 80 and 220 KHz below the requested
    // frequency. and an even 100kHz multiple. 

    // The nearest step of the integer-N synthesizer is at 143.75
    // so that is the constant front-end RX frequency that we'll use.
    double stepsize;
    double freq1stLO;
    /// This code depends on the integer-N tuning features in libuhd 3.7
    /// earlier libraries will revert to fractional-N tuning and might
    /// see a rise in the noisefloor and perhaps some troublesome spurs
    /// at multiples of the reference frequency divided by the fractional divisor.
    if(freq > 256e6) stepsize = 12.5e6;
    else stepsize = 6.25e6; 
    freq1stLO = floor(freq / stepsize) * stepsize;

    //    if((freq - freq1stLO) < 1.0e6) freq1stLO += stepsize;

    target_rx_freq = 100e3 * floor(freq / 100e3);
    if((freq - target_rx_freq) < 80e3) target_rx_freq -= 100.0e3; 

    uhd::tune_request_t rx_trequest(target_rx_freq, 100.0e3);
    rx_trequest.args = uhd::device_addr_t("mode_n=integer"); 
    last_rx_tune_result = usrp->set_rx_freq(rx_trequest);
    last_rx_tune_result = checkLock(rx_trequest, 'r', last_rx_tune_result);
    if(debug_mode) {
      std::cerr << boost::format("RX Tune RF_actual %lf DDC = %lf tuned = %lf target = %lf request  rf = %lf request ddc = %lf\n")
	% last_rx_tune_result.actual_rf_freq
	% last_rx_tune_result.actual_dsp_freq
	% freq
	% target_rx_freq
	% rx_trequest.rf_freq
	% rx_trequest.dsp_freq;
    }
  }
  else {
    // On the transmit side, we're using a minimal IF rate and
    // using the full range of the tuning hardware.

    // if the tx is off, we pretend that we're locked and we ignore the freq.
    if(!tx_on) return;

    if(debug_mode) {
      std::cerr << "Tuning TX unit to new frequency: ["
		<< std::setprecision(10) << freq << std::endl;
    }
    
    last_tx_tune_result = usrp->set_tx_freq(freq);  
    last_tx_tune_result = checkLock(tx_trequest, 't', last_tx_tune_result);
  }

  // If we are setting the RX mode, then we need to send
  // a message to the USRPRX to tell it what its IF freq should be.
  if((sel == 'r') && set_if_freq) {
    cmd_stream->put(new Command(Command::SET, Command::RX_LO3_FREQ,
				freq - target_rx_freq)); 
  }
  else {
  }
}

/**
 * exeSetCommand handles SET messages of the following type:
 * @li RX_RETUNE_FREQ causes the front-end LO + DDS (FE chain) to be retuned to a frequency
 * at least 80 kHz above and no more than 250 kHz above the requested
 * frequency. This places the signal of interest at an IF frequency between
 * 80 kHz and 250 kHz. The IF frequency is set (in the USRPRX thread) to the
 * difference between the requested frequency and the FE chain frequency.
 * @li RX_TUNE_FREQ and RX_FE_FREQ set the FE chain frequency to the specified value.
 * @li LO_CHECK set the FE chain to the specified frequency. This is used in
 * calibrating a transverter chain by "tuning" to a frequency near the transverter
 * LO and listening to the leakage signal.  (It's a long story.)
 * @li TX_RETUNE_FREQ, RX_TUNE_FREQ and RX_FE_FREQ all set the transmit FE chain frequency
 * to the requested value PLUS the tx_freq_rxmode_offset (the transmit IF frequency).
 * @li RX_SAMP_RATE set the receive A/D sample rate in the USRP
 * @li TX_SAMP_RATE set the transmit D/A sample rate in the USRP
 * @li RX_RF_GAIN set the RF gain of the receive front-end
 * @li TX_RF_GAIN set the gain of the transmit front-end
 * @li TX_STATE turn the transmitter ON or OFF
 * @li CLOCK_SOURCE select the external frequency reference for the USRP or the internal clock.
 * @li RX_ANT set the receive antenna port
 * @li TX_ANT set the transmit antenna port
 */
void SoDa::USRPCtrl::execSetCommand(Command * cmd)
{
  double freq, fdiff; 
  if(cmd->cmd != Command::SET) {
    std::cerr << "execSetCommand got a non-set command!  " << cmd->toString() << std::endl;
    return; 
  }
  switch (cmd->target) {
  case Command::RX_RETUNE_FREQ:
    last_rx_req_freq = cmd->dparms[0]; 
    freq = cmd->dparms[0];
    if(debug_mode) {
      std::cerr << "Got RX RETUNE request -- frequency = " << freq << " diff is " << fdiff << std::endl;
    }
    fdiff = freq - (last_rx_tune_result.actual_rf_freq - last_rx_tune_result.actual_dsp_freq); 
    if((fdiff < 300e3) && (fdiff > 50e3)) {
      cmd_stream->put(new Command(Command::SET, Command::RX_LO3_FREQ, fdiff)); 
      cmd_stream->put(new Command(Command::REP, Command::RX_FE_FREQ, 
				  last_rx_tune_result.actual_rf_freq - last_rx_tune_result.actual_dsp_freq));
      break; 
    }
    // else -- treat this as a RX_TUNE_FREQ request.
  case Command::RX_TUNE_FREQ:    
  case Command::RX_FE_FREQ:
    last_rx_req_freq = cmd->dparms[0]; 
    set1stLOFreq(cmd->dparms[0], 'r', cmd->target != Command::RX_TUNE_FREQ);
    cmd_stream->put(new Command(Command::REP, Command::RX_FE_FREQ, 
			       last_rx_tune_result.actual_rf_freq - last_rx_tune_result.actual_dsp_freq)); 
    break;

  case Command::LO_CHECK:
    if(cmd->dparms[0] == 0.0) {
      set1stLOFreq(last_rx_req_freq, 'r', false);
    }
    else {
      if(debug_mode) {
	std::cerr << "setting lo check freq to " << cmd->dparms[0] << std::endl;
      }
      usrp->set_rx_freq(cmd->dparms[0]);
      // now send a GET lo offset command
      cmd_stream->put(new Command(Command::GET, Command::LO_OFFSET, 0));
    }
    break;

  case Command::TX_RETUNE_FREQ:
  case Command::TX_TUNE_FREQ:
  case Command::TX_FE_FREQ:
    if (debug_mode) {
      std::cerr << "\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n!!! GOT TX TUNE REQ !!!!!!!!\n!!!!!!!!!!!!!!!!!!" << std::endl;
      std::cerr << " freq = " << std::setprecision(10) << freq << std::endl; 
      std::cerr << "\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n!!! GOT TX TUNE REQ !!!!!!!!\n!!!!!!!!!!!!!!!!!!" << std::endl;
    }
    set1stLOFreq(cmd->dparms[0] + tx_freq_rxmode_offset, 't', false);
    tx_freq = cmd->dparms[0]; 
    cmd_stream->put(new Command(Command::REP, Command::TX_FE_FREQ, 
			       last_tx_tune_result.actual_rf_freq + last_tx_tune_result.actual_dsp_freq)); 
    break; 

  case Command::RX_SAMP_RATE:
    usrp->set_rx_rate(cmd->dparms[0]); 
    cmd_stream->put(new Command(Command::REP, Command::RX_SAMP_RATE, 
			       usrp->get_rx_rate())); 
    break; 
  case Command::TX_SAMP_RATE:
    tx_samp_rate = cmd->dparms[0]; 
    usrp->set_tx_rate(cmd->dparms[0]); 
    cmd_stream->put(new Command(Command::REP, Command::TX_SAMP_RATE, 
			       usrp->get_tx_rate())); 
    break;
    
  case Command::RX_RF_GAIN:
    rx_rf_gain = cmd->dparms[0];
    if(!tx_on) {
      usrp->set_rx_gain(cmd->dparms[0]); 
      cmd_stream->put(new Command(Command::REP, Command::RX_RF_GAIN, 
				  usrp->get_rx_gain()));
    }
    break; 
  case Command::TX_RF_GAIN:
    tx_rf_gain = cmd->dparms[0];
    if(tx_on) {
      usrp->set_tx_gain(cmd->dparms[0]);
      cmd_stream->put(new Command(Command::REP, Command::TX_RF_GAIN, 
				  usrp->get_tx_gain())); 
    }
    break; 
  case SoDa::Command::TX_STATE: // SET TX_ON
    if(cmd->iparms[0] == 1) {
      // set the txgain to where it is supposed to be.
      tx_on = true; 
      usrp->set_rx_gain(0.0); 
      usrp->set_tx_gain(tx_rf_gain); 
      cmd_stream->put(new Command(Command::REP, Command::TX_RF_GAIN, 
				  usrp->get_tx_gain()));
      // to move a birdie away, we bumped the TX LO,, move it back. 
      tx_freq_rxmode_offset = 0.0; // so tuning works.

      // enable the transmit relay
      if(debug_mode) {
	std::cerr << "Enabling TX" << std::endl;
	std::cerr << "Current TXENA " << tx_fe_subtree->access<bool>("enabled").get() << std::endl;
	std::cerr << boost::format("Current GPIO = %x ") %
	  dboard->get_gpio_out(uhd::usrp::dboard_iface::UNIT_TX) << std::endl;
      }
      setTXEna(true);
      if(debug_mode) {
	std::cerr << "New TXENA "
		  << tx_fe_subtree->access<bool>("enabled").get() << std::endl;
	std::cerr << boost::format("New GPIO = %x ")
	  % dboard->get_gpio_out(uhd::usrp::dboard_iface::UNIT_TX) << std::endl;
      }
      // and tell the TX unit to turn on the TX
      cmd_stream->put(new Command(Command::SET, Command::TX_STATE, 
				  3));
    }
    if(cmd->iparms[0] == 0) {
      tx_on = false; 
      // set txgain to zero
      usrp->set_tx_gain(0.0);
      usrp->set_rx_gain(rx_rf_gain);
      // tune the TX unit 1MHz away from where we want to be. 
      tx_freq_rxmode_offset = rxmode_offset; // so tuning works.
      set1stLOFreq(tx_freq + tx_freq_rxmode_offset, 't', false);
      // turn off the transmit relay and the TX chain.
      // This also turns off the TX LO on a WBX module, so
      // the above tx_freq_rxmode  trick may not be necessary
      // We keep the rxmode_offset here in case other modules
      // leave the TXLO on.
      setTXEna(false); 
      if(debug_mode) {
	std::cerr << "Disabling TX" << std::endl;
	std::cerr << "Got TXENA " << tx_fe_subtree->access<bool>("enabled").get() << std::endl;
	std::cerr << boost::format("Got GPIO = %x ") %
	  dboard->get_gpio_out(uhd::usrp::dboard_iface::UNIT_TX) << std::endl;
      }
    }
    break; 

  case Command::CLOCK_SOURCE:
    if((cmd->iparms[0] & 1) == 1) {
      if(debug_mode) {
	std::cerr << "Setting reference to external" << std::endl;
      }
      usrp->set_clock_source(std::string("external"));
    }
    else {
      if(debug_mode) {
	std::cerr << "Setting reference to internal" << std::endl;
      }
      usrp->set_clock_source(std::string("internal"));
    }
    break; 

  case Command::RX_ANT:
    usrp->set_rx_antenna(cmd->sparm);
    break; 

  case Command::TX_ANT:
    tx_ant = cmd->sparm; 
    usrp->set_tx_antenna(cmd->sparm);
    break;

  default:
    break; 
  }
}

void SoDa::USRPCtrl::execGetCommand(Command * cmd)
{
  int res;

  
  switch (cmd->target) {
  case Command::RX_FE_FREQ:
    cmd_stream->put(new Command(Command::REP, Command::RX_FE_FREQ, 
				last_rx_tune_result.actual_rf_freq,
				last_rx_tune_result.actual_dsp_freq)); 
    break; 
  case Command::TX_FE_FREQ:
    cmd_stream->put(new Command(Command::REP, Command::TX_FE_FREQ, 
				last_tx_tune_result.actual_rf_freq,
				last_tx_tune_result.actual_dsp_freq)); 
    break; 

  case Command::RX_SAMP_RATE:
    cmd_stream->put(new Command(Command::REP, Command::RX_SAMP_RATE, 
			       usrp->get_rx_rate())); 
    break; 
  case Command::TX_SAMP_RATE:
    cmd_stream->put(new Command(Command::REP, Command::TX_SAMP_RATE, 
			       usrp->get_tx_rate())); 
    break;

  case Command::CLOCK_SOURCE:
    res = 0;
    if(usrp->get_clock_source(0) == std::string("external")) {
      res = 2; 
    }

    if (1) {
      // is it locked?
      uhd::sensor_value_t ref_locked = usrp->get_mboard_sensor("ref_locked", 0);
      if(ref_locked.to_bool()) {
	res |= 1; 
      }
    }
       
    cmd_stream->put(new Command(Command::REP, Command::CLOCK_SOURCE,
				res));
    break;
		    
  default:
    break; 
  }
}
void SoDa::USRPCtrl::execRepCommand(Command * cmd)
{
  switch (cmd->target) {
  default:
    break; 
  }
}

void SoDa::USRPCtrl::initControlGPIO()
{
  // first, find the daughtercard
  dboard = usrp->get_tx_dboard_iface(); 

  // now get the old version of the GPIO enable mask.
  unsigned short dir = dboard->get_gpio_ddr(uhd::usrp::dboard_iface::UNIT_TX); 
  unsigned short ctl = dboard->get_pin_ctrl(uhd::usrp::dboard_iface::UNIT_TX);
  unsigned short out = dboard->get_gpio_out(uhd::usrp::dboard_iface::UNIT_TX);
  
  // and print it.
  if(debug_mode) {
    std::cerr << boost::format("TX GPIO direction = %04x  control = %04x  output = %04x  ctlmask = %04x  monmask = %04x") % dir % ctl % out % TX_RELAY_CTL % TX_RELAY_MON << std::endl;
  }

  // now set the direction to OUT for the CTL bit
  dboard->set_gpio_ddr(uhd::usrp::dboard_iface::UNIT_TX,
		    TX_RELAY_CTL, TX_RELAY_CTL);
  
  // and control it from the GPIO write
  dboard->set_pin_ctrl(uhd::usrp::dboard_iface::UNIT_TX,
		    0, TX_RELAY_CTL);
  
  // and make sure the txena is OFF
  dboard->set_gpio_out(uhd::usrp::dboard_iface::UNIT_TX,
		    0, TX_RELAY_CTL); 

}

void SoDa::USRPCtrl::setTXEna(bool val)
{
  unsigned short enabits = val ? TX_RELAY_CTL : 0; 
  dboard->set_gpio_out(uhd::usrp::dboard_iface::UNIT_TX,
		       enabits, TX_RELAY_CTL);

  // enable the transmitter (or disable it)
  tx_fe_subtree->access<bool>("enabled").set(val);
  if(debug_mode) {
    std::cerr << "Got " << tx_fe_subtree->access<bool>("enabled").get() << " from call to en/dis TX with val = " << val << std::endl;
  }

  // if we're enabling, set the power, freq, and other stuff
  if(val) {
    usleep(400);    
    // set the tx antenna
    usrp->set_tx_antenna(tx_ant); 
    // set the tx rate
    usrp->set_tx_rate(tx_samp_rate); 
    // set the tx gain. 
    usrp->set_tx_gain(tx_rf_gain);
    // tx freq
    set1stLOFreq(tx_freq, 't', false);  
  }
}


bool SoDa::USRPCtrl::getTXEna()
{
  unsigned int enabits; 
  enabits = dboard->get_gpio_out(uhd::usrp::dboard_iface::UNIT_TX);

  return ((enabits & TX_RELAY_CTL) != 0); 
}


bool SoDa::USRPCtrl::getTXRelayOn()
{
  unsigned int enabits; 
  enabits = dboard->read_gpio(uhd::usrp::dboard_iface::UNIT_TX);

  return ((enabits & TX_RELAY_MON) != 0); 
}

