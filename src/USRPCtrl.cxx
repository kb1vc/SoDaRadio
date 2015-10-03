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
#include <uhd/utils/msg.hpp>
#include <uhd/types/tune_request.hpp>
#include <uhd/types/tune_result.hpp>
#include <boost/format.hpp>


// Mac OSX doesn't have a clock_gettime, it has
// the microsecond resolution gettimeofday. 
#include <sys/time.h>

const unsigned int SoDa::USRPCtrl::TX_RELAY_CTL = 0x1000;
const unsigned int SoDa::USRPCtrl::TX_RELAY_MON = 0x0800;

SoDa::USRPCtrl * SoDa::USRPCtrl::singleton_ctrl_obj = NULL; 

// borrowed from uhd_usrp_probe print_tree function
void dumpTree(const uhd::fs_path &path, uhd::property_tree::sptr tree){
    std::cout << path << std::endl;
    BOOST_FOREACH(const std::string &name, tree->list(path)){
        dumpTree(path / name, tree);
    }
}

SoDa::USRPCtrl::USRPCtrl(Params * _params, CmdMBox * _cmd_stream) : SoDa::SoDaThread("USRPCtrl")
{
  // point to myself.... 
  SoDa::USRPCtrl::singleton_ctrl_obj = this;

  // setup a normal message handler that doesn't babble
  // so much. 
  uhd::msg::register_handler(normal_message_handler);
  
  // initialize variables
  last_rx_req_freq = 0.0; // at least this is a number...
  tx_on = false;
  first_gettime = 0.0;
  rx_rf_gain = 0.0;
  tx_rf_gain = 0.0;
  tx_freq = 0.0;
  tx_freq_rxmode_offset = 0.0;
  tx_samp_rate = 625000;
  tx_ant = std::string("TX");
  motherboard_name = std::string("UNKNOWN_MB");
  
  cmd_stream = _cmd_stream;
  params = _params;

  // subscribe to the command stream.
  subid = cmd_stream->subscribe();
  
  // make the device.
  usrp = uhd::usrp::multi_usrp::make(params->getUHDArgs());

  if(usrp == NULL) {
    throw SoDaException((boost::format("Unable to allocate USRP unit with arguments = [%]\n") % params->getUHDArgs()).str(), this);
  }

  uhd::property_tree::sptr tree = usrp->get_device()->get_tree();
  const std::string mbname = tree->list("/mboards").at(0);
  // find out what kind of device we have.
  motherboard_name = tree->access<std::string>("/mboards/" + mbname + "/name").get();

  if((motherboard_name == "B200") || (motherboard_name == "B210")) {
    // B2xx needs a master clock rate of 50 MHz to generate a sample rate of 625 kS/s.
    // B2xx needs a master clock rate of 25 MHz to generate a sample rate of 625 kS/s.
    usrp->set_master_clock_rate(25.0e6);
    debugMsg(boost::format("Initial setup %s") % usrp->get_pp_string());
    is_B2xx = true;
    is_B210 = (motherboard_name == "B210");
  }
  else {
    is_B2xx = false; 
    is_B210 = false;
  }

  // we need to setup the subdevices
  if(is_B2xx) {
    usrp->set_rx_subdev_spec(std::string("A:A"), 0);
    std::cerr << "DISABLING TVRT LO" << std::endl;
    if(0 && is_B210) {
      debugMsg("Setup two subdevices -- TVRT_LO Capable");
      usrp->set_tx_subdev_spec(std::string("A:A A:B"), 0);
      tvrt_lo_capable = true;
    }
    else {
      debugMsg("Setup one subdevice -- NOT TVRT_LO Capable");
      usrp->set_tx_subdev_spec(std::string("A:A"), 0);
      tvrt_lo_capable = false;
    }
  }
  else {
    debugMsg("Setup one subdevice -- NOT TVRT_LO Capable");
    tvrt_lo_capable = false;
  }

  first_gettime = 0.0;
  double tmp = getTime();
  // remove the whole number of seconds -- paranoia
  first_gettime = floor(tmp); 

  // dumpTree("/mboards", tree);


  uhd::usrp::subdev_spec_t rx_subdev_spec = tree->access<uhd::usrp::subdev_spec_t>("/mboards/" + mbname + "/rx_subdev_spec").get();
  
  debugMsg(boost::format("RX SUBDEV Spec [%s]\n") % rx_subdev_spec.to_string());

  // get the tx front end subtree
  uhd::fs_path tx_fe_root;
  tx_fe_root = is_B2xx ? ("/mboards/" + mbname + "/dboards/A/tx_frontends/A") :
    ("/mboards/" + mbname + "/dboards/A/tx_frontends/0");
  if(tree->exists(tx_fe_root)) {
    tx_fe_subtree = tree->subtree(tx_fe_root);
  }

  // get the rx front end subtree
  uhd::fs_path rx_fe_root;
  rx_fe_root = is_B2xx ? ("/mboards/" + mbname + "/dboards/A/rx_frontends/A") :
    ("/mboards/" + mbname + "/dboards/A/rx_frontends/0");
  if(tree->exists(rx_fe_root)) {
    rx_fe_subtree = tree->subtree(rx_fe_root);
  }
  rx_fe_subtree->access<bool>("enabled").set(true);

  // UBX modules are an issue
  db_is_UBX = false; 
  if(!is_B2xx) {
    uhd::fs_path namepath = rx_fe_root / "name"; 
    std::string dbname = tree->access<std::string>(namepath).get();
    // std::cerr << boost::format("***********\n\nfrontend name = [%s]\n\n*********\n")
    //   % dbname; 

    std::string ubxname("UBX");
    if(dbname.compare(0, 3, ubxname) == 0) {
      // std::cerr << "This DB is a UBX" << std::endl; 
      db_is_UBX = true; 
    }
    else {
      // std::cerr << boost::format("This DB is NOT a UBX [%s] != [%s]\n") % ubxname % dbname; 
      db_is_UBX = false; 
    }
  }

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
  initStepMap(params->forceIntN(), params->forceFracN()); 
}


void SoDa::USRPCtrl::run()
{
  uhd::set_thread_priority_safe(); 
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
	debugMsg(boost::format("USRPCtrl processed %d commands") % cmds_processed);
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
  int lock_itercount = 0;
  uhd::tune_result_t ret = cur;

  // as of version 3.8.2 the B2xx has the lo_locked sensor... if(is_B2xx) return ret;
  
  while(1) {
    uhd::sensor_value_t lo_locked = (sel == 'r') ? usrp->get_rx_sensor("lo_locked",0) : usrp->get_tx_sensor("lo_locked",0);
    if(lo_locked.to_bool()) break;
    else usleep(1000);
    if((lock_itercount & 0xfff) == 0) {
      debugMsg(boost::format("Waiting for %c LO lock to freq = %f (%f:%f)  count = %d\n")
	       % sel % req.target_freq % req.rf_freq % req.dsp_freq % lock_itercount);
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
 
  if(sel == 'r') {
    // we round the target frequency to a point that puts the
    // baseband between 150 and 250 KHz below the requested
    // frequency. and an even 100kHz multiple.

    target_rx_freq = 100e3 * floor(freq / 100e3);
    debugMsg(boost::format("freq = %lf 1st target = %lf\n") % freq % target_rx_freq);
    while((freq - target_rx_freq) < 100e3) {
      target_rx_freq -= 100.0e3;
      debugMsg(boost::format("\tfreq = %lf new target = %lf\n") % freq % target_rx_freq);
    }

    /// This code depends on the integer-N tuning features in libuhd 3.7
    /// earlier libraries will revert to fractional-N tuning and might
    /// see a rise in the noisefloor and perhaps some troublesome spurs
    /// at multiples of the reference frequency divided by the fractional divisor.
    uhd::tune_request_t rx_trequest(target_rx_freq); 
    if(supports_IntN_Mode) {
      rx_trequest.target_freq = target_rx_freq;
      rx_trequest.rf_freq = getNearestStep(target_rx_freq, 1.0e6);
      rx_trequest.rf_freq_policy = uhd::tune_request_t::POLICY_MANUAL;
      rx_trequest.dsp_freq_policy = uhd::tune_request_t::POLICY_AUTO;
      rx_trequest.args = uhd::device_addr_t("mode_n=integer");
    }
    else {
      rx_trequest.target_freq = target_rx_freq;
      rx_trequest.rf_freq = target_rx_freq; 
      rx_trequest.rf_freq_policy = uhd::tune_request_t::POLICY_AUTO;
      rx_trequest.dsp_freq_policy = uhd::tune_request_t::POLICY_AUTO;
    }

    last_rx_tune_result = usrp->set_rx_freq(rx_trequest);
    last_rx_tune_result = checkLock(rx_trequest, 'r', last_rx_tune_result);
    debugMsg(boost::format("RX Tune RF_actual %lf DDC = %lf tuned = %lf target = %lf request  rf = %lf request ddc = %lf\n")
	     % last_rx_tune_result.actual_rf_freq
	     % last_rx_tune_result.actual_dsp_freq
	     % freq
	     % target_rx_freq
	     % rx_trequest.rf_freq
	     % rx_trequest.dsp_freq);
  }
  else {
    // On the transmit side, we're using a minimal IF rate and
    // using the full range of the tuning hardware.

    // If the transmitter is off, we retune anyway to park the
    // transmit LO as far away as possible.   This is especially 
    // important for the UBX.
    
    uhd::tune_request_t tx_request(freq);
    
    if(tvrt_lo_mode) {
      tx_request.rf_freq_policy = uhd::tune_request_t::POLICY_MANUAL;
      tx_request.rf_freq = tvrt_lo_fe_freq;
    }
    else if(supports_IntN_Mode) {
      tx_request.rf_freq = freq; 
      tx_request.rf_freq_policy = uhd::tune_request_t::POLICY_MANUAL;
      tx_request.dsp_freq_policy = uhd::tune_request_t::POLICY_AUTO;
      tx_request.args = uhd::device_addr_t("mode_n=integer");
    }
    else {
      tx_request.rf_freq_policy = uhd::tune_request_t::POLICY_AUTO;
    }

    debugMsg(boost::format("Tuning TX unit to new frequency %f (request = %f  (%f %f))\n")
	     % freq % tx_request.target_freq % tx_request.rf_freq % tx_request.dsp_freq);

    last_tx_tune_result = usrp->set_tx_freq(tx_request);

    debugMsg(boost::format("Tuned TX unit to new frequency %g t.rf %g a.rf %g t.dsp %g a.dsp %g\n")
	     % freq
	     % last_tx_tune_result.target_rf_freq
	     % last_tx_tune_result.actual_rf_freq
	     % last_tx_tune_result.target_dsp_freq
	     % last_tx_tune_result.actual_dsp_freq);

    last_tx_tune_result = checkLock(tx_request, 't', last_tx_tune_result);
    // tx_fe_subtree->access<bool>("enabled").set(last_tx_ena);

    double txfreqs[2];
    txfreqs[0] = usrp->get_tx_freq(0);
    if(tvrt_lo_mode) {
      txfreqs[1] = usrp->get_tx_freq(1);
      debugMsg(boost::format("TX LO = %g  TVRT LO = %g\n") % txfreqs[0] % txfreqs[1]);
    }
  }

  // If we are setting the RX mode, then we need to send
  // a message to the USRPRX to tell it what its IF freq should be.
  if((sel == 'r') && set_if_freq) {
    cmd_stream->put(new Command(Command::SET, Command::RX_LO3_FREQ,
				freq - target_rx_freq)); 
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
void SoDa::USRPCtrl::execSetCommand(Command * cmd)
{
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
      debugMsg(boost::format("Enabling TX\nCurrent TXENA %d\n") % tx_fe_subtree->access<bool>("enabled").get());
      if(supports_tx_gpio) {
	debugMsg(boost::format("Current GPIO = %x ") %
		 dboard->get_gpio_out(uhd::usrp::dboard_iface::UNIT_TX));
      }
      setTXEna(true);

      debugMsg(boost::format("New TXENA %d\n") % tx_fe_subtree->access<bool>("enabled").get());
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
      debugMsg(boost::format("Disabling TX -- Got TXENA %d") % tx_fe_subtree->access<bool>("enabled").get());
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

void SoDa::USRPCtrl::setTXEna(bool val)
{
  unsigned short enabits = val ? TX_RELAY_CTL : 0;
  if(supports_tx_gpio) {
    dboard->set_gpio_out(uhd::usrp::dboard_iface::UNIT_TX,
			 enabits, TX_RELAY_CTL);
  }

  // switch the relay BEFORE transmit on....
  if(val) {
    tr_control->setTXOn(); 
  }

  // enable the transmitter (or disable it)
  tx_fe_subtree->access<bool>("enabled").set(val);
  debugMsg(boost::format("Got %d from call to en/dis TX with val = %d")
	   % tx_fe_subtree->access<bool>("enabled").get() % val);

  debugMsg(boost::format("Got rx_fe enabled = %d from call to en/dis TX with val = %d")
	   % rx_fe_subtree->access<bool>("enabled").get() % val);

  // if we're enabling, set the power, freq, and other stuff
  if(val) {
    usleep(400);    
    // set the tx antenna
    usrp->set_tx_antenna(tx_ant); 
    // set the tx gain. 
    usrp->set_tx_gain(tx_rf_gain);
    // tx freq
    set1stLOFreq(tx_freq + tx_freq_rxmode_offset, 't', false);  

    double r = usrp->get_tx_rate(); 
    debugMsg(boost::format("TX rate = %g\n") % r);
  }

  if(!val) {
    tr_control->setTXOff(); 
  }
  
}


bool SoDa::USRPCtrl::getTXEna()
{
  unsigned int enabits = 0; 
  if(supports_tx_gpio) {
    enabits = dboard->get_gpio_out(uhd::usrp::dboard_iface::UNIT_TX);
  }

  return ((enabits & TX_RELAY_CTL) != 0); 
}


bool SoDa::USRPCtrl::getTXRelayOn()
{
  unsigned int enabits = 0; 
  if(supports_tx_gpio) {
    enabits = dboard->read_gpio(uhd::usrp::dboard_iface::UNIT_TX);
  }

  return ((enabits & TX_RELAY_MON) != 0); 
}

void SoDa::USRPCtrl::setTransverterLOFreqPower(double freq, double power)
{
  uhd::gain_range_t tx_gain_range = usrp->get_tx_gain_range(1);
  double plo = tx_gain_range.start();
  double phi = tx_gain_range.stop();
  tvrt_lo_gain = plo + power * (phi - plo);
  tvrt_lo_freq = freq; 
  
  debugMsg(boost::format("Setting Transverter LO freq = %10lg power = %g gain = %g\n") % tvrt_lo_freq % power % tvrt_lo_gain);
  
  debugMsg("About to report Transverter LO setting.");
  cmd_stream->put(new Command(Command::REP, Command::TVRT_LO_CONFIG, tvrt_lo_freq, power));  

}

void SoDa::USRPCtrl::enableTransverterLO()
{
  if(!tvrt_lo_capable) {
    tvrt_lo_mode = false; 
    return;
  }

  debugMsg("Enabling transverter LO\n");
  usrp->set_tx_antenna("TX2", 1);
    
  usrp->set_tx_gain(tvrt_lo_gain, 1);
  // tune the first LO 4MHz below the target, and let the DDC make up the rest. 
  uhd::tune_request_t lo_freq_req(tvrt_lo_freq, -4.0e6);
  uhd::tune_result_t tres = usrp->set_tx_freq(lo_freq_req, 1);

  tvrt_lo_mode = true;
  
  debugMsg(boost::format("LO frequency = %10lg power %g  number of channels = %d target_rf %g actual rf %g target dsp %g actual dsp %g\n")
	     % usrp->get_tx_freq(1) % usrp->get_tx_gain(1) % usrp->get_tx_num_channels()
	     % tres.target_rf_freq % tres.actual_rf_freq % tres.target_dsp_freq % tres.actual_dsp_freq);

  tvrt_lo_fe_freq = tres.target_rf_freq; 
}

void SoDa::USRPCtrl::disableTransverterLO()
{
  tvrt_lo_mode = false;
  if(!tvrt_lo_capable) return; 
  usrp->set_tx_gain(0.0, 1);
  usrp->set_tx_freq(100.0e6, 1);
}

double SoDa::USRPCtrl::getNearestStep(double freq, double offset)
{
  double ret = freq - offset;

  // Without integer-N mode, this is a wash....
  if(supports_IntN_Mode) {
    if(lo_step_map.find(freq - offset) != lo_step_map.end()) {
      ret = lo_step_map[freq - offset]; 
    }
  }
  return ret; 
}

void SoDa::USRPCtrl::freq_search_message_handler(uhd::msg::type_t type, const std::string & msg)
{
  switch (type) {
  case uhd::msg::status:
    // do nothing -- these are the rx freq setting complaints.
    break; 
  case uhd::msg::error:
    std::cerr << "UHD ERROR: " << msg << std::flush;
    break;
  case uhd::msg::warning:
    std::cerr << "UHD WARNING: " << msg << std::flush;
    break;
  default:
    SoDa::USRPCtrl::singleton_ctrl_obj->debugMsg(msg);
    break; 
  }
}

void SoDa::USRPCtrl::normal_message_handler(uhd::msg::type_t type, const std::string & msg)
{
  switch (type) {
  case uhd::msg::error:
    std::cerr << "UHD ERROR: " << msg << std::flush;
    break;
  case uhd::msg::warning:
    std::cerr << "UHD WARNING: " << msg << std::flush;
    break;
  default:
    SoDa::USRPCtrl::singleton_ctrl_obj->debugMsg(msg);
    break; 
  }
}

void SoDa::USRPCtrl::initStepMap(bool force_int_N, bool force_frac_N)
{
  uhd::tune_result_t tunres_int, tunres_frac;  

  debugMsg("In initStepMap\n");
  
  supports_IntN_Mode = false;
  if(force_int_N) {
    debugMsg("Forced IntN Tuning support ON.");
    supports_IntN_Mode = true; 
  }
  else if(force_frac_N) {
    debugMsg("Forced IntN Tuning support OFF.");
    supports_IntN_Mode = false; 
  }
  else if(is_B2xx) return; 
  else {
    // first, do we have this capability?
    // pick a frequency halfway between the min and max freq for this MB.
    double tf = (rx_rf_freq_range.stop() + rx_rf_freq_range.start()) * 0.5;
    // Now bump it by some silly amount
    tf += 123456.789;

    debugMsg(boost::format("got tf = %g\n") % tf);
  
    // and tune with and without intN
    uhd::tune_request_t tunreq_int(tf);
    uhd::tune_request_t tunreq_frac(tf);
    tunreq_int.args = uhd::device_addr_t("mode_n=integer");
    debugMsg("About to tune int...\n");
    tunres_int = usrp->set_rx_freq(tunreq_int);
    debugMsg("About to tune frac...\n");
    tunres_frac = usrp->set_rx_freq(tunreq_frac);
  
    // are there differences?
    if(tunres_int.actual_rf_freq != tunres_frac.actual_rf_freq) {
      supports_IntN_Mode = true;
    }

    debugMsg(boost::format("int rf = %g  frac rf = %g  tf = %g\n")
	     % tunres_int.actual_rf_freq % tunres_frac.actual_rf_freq % tf);
  }



  if(supports_IntN_Mode) {
    debugMsg("Supports INT_N tuning mode.\n");
  }
  else {
    debugMsg("Does not support INT_N tuning mode.\n");
  }

  // set the message handler for status messages to throw stuff away.
  // the driver complains a lot about unsupported settings, but that
  // is the whole point of this loop.... so....
  uhd::msg::register_handler(freq_search_message_handler);
  
  // Now sweep from min to max freq, and find the steps along the way.
  double ff_incr = 1.0e6; // start with a small step...
  double r_st, r_en, target, target2;
  r_st = rx_rf_freq_range.start();

  // first find the first setting. 
  uhd::tune_request_t treq(r_st);
  treq.args = uhd::device_addr_t("mode_n=integer");
  tunres_int = usrp->set_rx_freq(treq);
  target = tunres_int.actual_rf_freq;

  // now look through the range
  for(double ff = target + ff_incr;
      ff < rx_rf_freq_range.stop();
      ff += ff_incr) {
    uhd::tune_request_t treq(ff);
    treq.args = uhd::device_addr_t("mode_n=integer");
    tunres_int = usrp->set_rx_freq(treq);
    if(target != tunres_int.actual_rf_freq) {
      // we found a new one...
      lo_step_map[SoDa::Range<double>(r_st, tunres_int.actual_rf_freq)] = target;
      r_st = tunres_int.actual_rf_freq;
      // ff_incr = (target - r_st) * 0.8;  // this is the step size... ? 
      target = r_st;
    }
    // debugMsg(boost::format("ff = %g   rf = %g\n")
    // 	     % ff % tunres_int.actual_rf_freq); 
  }

  // now go back to the normal handler. 
  uhd::msg::register_handler(normal_message_handler);
  
  if(getDebugLevel() > 2) {
    typedef std::pair<SoDa::Range<double>, double> fmap_el;
    BOOST_FOREACH( fmap_el const & el, lo_step_map) {
      std::cerr << boost::format("Range: %g to %g --- Base: %g\n")
	% el.first.getMin() % el.first.getMax() % el.second; 
    }
  }

  return; 
}
