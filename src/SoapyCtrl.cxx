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

#include "SoapyCtrl.hxx"
#include "SoDaBase.hxx"
#include <uhd/utils/thread_priority.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/utils/msg.hpp>
#include <uhd/types/tune_request.hpp>
#include <uhd/types/tune_result.hpp>
#include <boost/format.hpp>
#include <boost/property_tree/exceptions.hpp>


// Mac OSX doesn't have a clock_gettime, it has
// the microsecond resolution gettimeofday. 
#include <sys/time.h>

SoDa::SoapyCtrl::SoapyCtrl(const std::string & driver_name, Params * _params, CmdMBox * _cmd_stream) : SoDa::SoDaThread("SoapyCTRL")
{
  // initialize variables
  last_rx_req_freq = 0.0; // at least this is a number...
  tx_on = false;
  rx_rf_gain = 0.0;
  tx_rf_gain = 0.0;
  tx_freq = 0.0;
  tx_freq_rxmode_offset = 0.0;
  tx_samp_rate = 625000;
  tx_ant = std::string("TX");
  model_name = std::string("UNKNOWN");
  
  cmd_stream = _cmd_stream;
  params = _params;

  // subscribe to the command stream.
  subid = cmd_stream->subscribe();
  
  // make the device.
  makeRadio(driver_name);

  if(radio == NULL) {
    throw SoDaException((boost::format("Unable to allocate SoapySDR [%s] device with arguments = [%]\n")
			 % driver_name % params->getRadioArgs()).str(), this);
  }
#if 0

  // find the gain ranges
  rx_rf_gain_range = usrp->get_rx_gain_range();
  tx_rf_gain_range = usrp->get_tx_gain_range();

  // find the frequency ranges
  rx_rf_freq_range = usrp->get_rx_freq_range();
  tx_rf_freq_range = usrp->get_tx_freq_range();

  // set the sample rates
  usrp->set_rx_rate(params->getRXRate());
  usrp->set_tx_rate(params->getTXRate());
  
  // setup the control IO pins (for TX/RX external relay)
  // Note that there are no GPIOs available for the B2xx right now.
  initControlGPIO();

  // setup a widget to control external devices 
  tr_control = SoDa::TRControl::makeTRControl(usrp);     

  // turn off the transmitter
  setTXEna(false);

  // turn of the LO
  tvrt_lo_mode = false;

  // if we are in integer-N mode, setup the step table.
  testIntNMode(params->forceIntN(), params->forceFracN());
#endif  
}

void SoDa::SoapyCtrl::makeRadio(const std::string & driver_name)
{
  // enumerate all the devices...
  std::string enstr = (boost::format("driver=%s") % driver_name).str(); 
  SoapySDR::KwargsList kwl = SoapySDR::Device::enumerate(enstr); 

  if(kwl.size() == 0) {
    std::cerr << "No LimeSDR devices were found.\n"; 
    exit(-1);
  }
  else if(kwl.size() > 1) {
    std::cerr << boost::format("Found %d LimeSDR devices.  Choosing the first one.\n") % kwl.size(); 
  }

  // make the radio
  radio = SoapySDR::Device::make(kwl[0]);
}

void SoDa::SoapyCtrl::run()
{
 
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
      boost::this_thread::sleep(boost::posix_time::milliseconds(50));
    }
    else {
      // process the command.
      if((cmds_processed & 0xff) == 0) {
	debugMsg(boost::format("SoapyCTRL processed %d commands") % cmds_processed);
      }
      cmds_processed++; 
      execCommand(cmd);
      exitflag |= (cmd->target == Command::STOP); 
      cmd_stream->free(cmd); 
    }
  }
}


void SoDa::SoapyCtrl::execCommand(Command * cmd)
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
 * @li RX_RETUNE_FREQ causes the front-end LO + DDS (FE chain) to be retuned to a frequency
 * at least 80 kHz above and no more than 250 kHz above the requested
 * frequency. This places the signal of interest at an IF frequency between
 * 80 kHz and 250 kHz. The IF frequency is set (in the USRPRX thread) to the
 * difference between the requested frequency and the FE chain frequency.
 * @li RX_TUNE_FREQ and RX_FE_FREQ set the FE chain frequency to the specified value.
 * @li LO_CHECK set the FE chain to the specified frequency. This is used in
 * calibrating a transverter chain by "tuning" to a frequency near the transverter
 * LO and listening to the leakage signal.  (It's a long story.)
 * @li TX_RETUNE_FREQ, TX_TUNE_FREQ and TX_FE_FREQ all set the transmit FE chain frequency
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
void SoDa::SoapyCtrl::execSetCommand(Command * cmd)
{
  (void) cmd; 
#if 0  
  double freq, fdiff; 
  if(cmd->cmd != Command::SET) {
    std::cerr << "execSetCommand got a non-set command!  " << cmd->toString() << std::endl;
    return; 
  }
  double tmp;
  switch (cmd->target) {
  case Command::RX_RETUNE_FREQ:
    last_rx_req_freq = cmd->dparms[0]; 
    freq = cmd->dparms[0];
    fdiff = freq - (last_rx_tune_result.actual_rf_freq - last_rx_tune_result.actual_dsp_freq);
    
    debugMsg(boost::format("Got RX RETUNE request -- frequency %f diff = %f  last actual_rf %f  dsp %f\n")
	     % freq % fdiff % last_rx_tune_result.actual_rf_freq % last_rx_tune_result.actual_dsp_freq);

    if((fdiff < 200e3) && (fdiff > 100e3)) {
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
    // now adjust the 3rd lo (missing in int-N mode redo....)
    fdiff = freq - (last_rx_tune_result.actual_rf_freq - last_rx_tune_result.actual_dsp_freq);    
    cmd_stream->put(new Command(Command::SET, Command::RX_LO3_FREQ, fdiff));     
    cmd_stream->put(new Command(Command::REP, Command::RX_FE_FREQ, 
			       last_rx_tune_result.actual_rf_freq - last_rx_tune_result.actual_dsp_freq)); 
    break;

  case Command::LO_CHECK:
    if(cmd->dparms[0] == 0.0) {
      set1stLOFreq(last_rx_req_freq, 'r', false);
    }
    else {
      debugMsg(boost::format("setting lo check freq to %lf\n") % cmd->dparms[0]);
      usrp->set_rx_freq(cmd->dparms[0]);
      // now send a GET lo offset command
      cmd_stream->put(new Command(Command::GET, Command::LO_OFFSET, 0));
    }
    break;

  case Command::TX_RETUNE_FREQ:
  case Command::TX_TUNE_FREQ:
  case Command::TX_FE_FREQ:
    debugMsg(boost::format("\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n!!! GOT TX TUNE REQ !!!!!!!!\n!!!!!!!!!!!!!!!!!!\n"));
    debugMsg(boost::format(" freq = %10lg\n") % freq);
    debugMsg(boost::format("\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n!!! GOT TX TUNE REQ !!!!!!!!\n!!!!!!!!!!!!!!!!!!"));
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
    // dparameters ranges from 0 to 100... normalize this
    // to the actual range; 
    rx_rf_gain = rx_rf_gain_range.start() + cmd->dparms[0] * 0.01 * (rx_rf_gain_range.stop() - rx_rf_gain_range.start());
    if(!tx_on) {
      usrp->set_rx_gain(rx_rf_gain);
      cmd_stream->put(new Command(Command::REP, Command::RX_RF_GAIN, 
				  usrp->get_rx_gain()));
    }
    break; 
  case Command::TX_RF_GAIN:
    tx_rf_gain = tx_rf_gain_range.start() + cmd->dparms[0] * 0.01 * (tx_rf_gain_range.stop() - tx_rf_gain_range.start());
    tmp = cmd->dparms[0];
    debugMsg(boost::format("Setting TX gain to %lg from power %lg") % tx_rf_gain % tmp);
    if(tx_on) {
      usrp->set_tx_gain(tx_rf_gain);
      cmd_stream->put(new Command(Command::REP, Command::TX_RF_GAIN, 
				  usrp->get_tx_gain())); 
    }
    break; 
  case SoDa::Command::TX_STATE: // SET TX_ON
    debugMsg(boost::format("TX_STATE arg = %d\n") % cmd->iparms[0]);
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
      debugMsg(boost::format("Enabling TX\nCurrent TXENA %d\n") % tx_fe_subtree->getBoolProp("enabled"));
      if(supports_tx_gpio) {
	debugMsg(boost::format("Current GPIO = %x ") %
		 dboard->get_gpio_out(uhd::usrp::dboard_iface::UNIT_TX));
      }
      setTXEna(true);

      debugMsg(boost::format("New TXENA %d\n") % tx_fe_subtree->getBoolProp("enabled"));
      if(supports_tx_gpio) {
	debugMsg(boost::format("New GPIO = %x ") % dboard->get_gpio_out(uhd::usrp::dboard_iface::UNIT_TX));
      }
      // and tell the TX unit to turn on the TX
      // This avoids the race between CTRL and TX/RX units for setup and teardown.... 
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
      debugMsg(boost::format("Disabling TX -- Got TXENA %d") % tx_fe_subtree->getBoolProp("enabled"));
      if(supports_tx_gpio) {
	debugMsg(boost::format("Got GPIO = %x ") % 
		 dboard->get_gpio_out(uhd::usrp::dboard_iface::UNIT_TX));
      }
      // and tell the RX unit to turn on the RX
      // This avoids the race between CTRL and TX/RX units for setup and teardown.... 
      cmd_stream->put(new Command(Command::SET, Command::TX_STATE, 
				  2));
    }
    break; 

  case Command::CLOCK_SOURCE:
    if((cmd->iparms[0] & 1) == 1) {
      debugMsg("Setting reference to external");
      usrp->set_clock_source(std::string("external"));
    }
    else {
      debugMsg("Setting reference to internal");
      usrp->set_clock_source(std::string("internal"));
    }
    break; 

  case Command::RX_ANT:
    usrp->set_rx_antenna(cmd->sparm);
    debugMsg(boost::format("Got RX antenna as [%s]\n") % usrp->get_rx_antenna());
    cmd_stream->put(new Command(Command::REP, Command::RX_ANT, usrp->get_rx_antenna()));
    break; 

  case Command::TX_ANT:
    tx_ant = cmd->sparm; 
    usrp->set_tx_antenna(cmd->sparm);
    cmd_stream->put(new Command(Command::REP, Command::TX_ANT, usrp->get_tx_antenna()));
    break;

  case Command::TVRT_LO_CONFIG:
    setTransverterLOFreqPower(cmd->dparms[0], cmd->dparms[1]);
    break;

  case SoDa::Command::TVRT_LO_ENABLE:
    debugMsg("Enable Transverter LO");
    enableTransverterLO();
    break; 

  case SoDa::Command::TVRT_LO_DISABLE:
    debugMsg("Disable Transverter LO");
    disableTransverterLO();
    break;

  default:
    break; 
  }
#endif  
}

void SoDa::SoapyCtrl::execGetCommand(Command * cmd)
{
  (void) cmd; 
#if 0  
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

  case Command::HWMB_REP:
    cmd_stream->put(new Command(Command::REP, Command::HWMB_REP,
				(boost::format("%s\t%6.1f to %6.1f MHz")
				 % motherboard_name
				 % (rx_rf_freq_range.start() * 1e-6)
				 % (rx_rf_freq_range.stop() * 1e-6)).str()));
    break; 
  default:
    break; 
  }
#endif  
}

void SoDa::SoapyCtrl::execRepCommand(Command * cmd)
{
  switch (cmd->target) {
  default:
    break; 
  }
}

#if 0
void SoDa::SoapyCtrl::initControlGPIO()
{
  supports_tx_gpio = true;

  // now, find the daughtercard, if it exists
  try {
    dboard = usrp->get_tx_dboard_iface();     
  }
  catch (uhd::lookup_error & v) {
    std::cerr << "No daughterboard interface found..." << std::endl;
    dboard == NULL; 
    supports_tx_gpio = false;
  }

  if(supports_tx_gpio) {

    // now get the old version of the GPIO enable mask.
    unsigned short dir = dboard->get_gpio_ddr(uhd::usrp::dboard_iface::UNIT_TX); 
    unsigned short ctl = dboard->get_pin_ctrl(uhd::usrp::dboard_iface::UNIT_TX);
    unsigned short out = dboard->get_gpio_out(uhd::usrp::dboard_iface::UNIT_TX);
  
    // and print it.
    debugMsg(boost::format("TX GPIO direction = %04x  control = %04x  output = %04x  ctlmask = %04x  monmask = %04x") % dir % ctl % out % TX_RELAY_CTL % TX_RELAY_MON);


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
  else {
    std::cerr << "GPIO control of TX relay is disabled.\n"
	      << "This is normal if this radio is a B2xx.\n";
  }
}
#endif
