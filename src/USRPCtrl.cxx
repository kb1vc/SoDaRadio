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
#include "USRPFrontEnd.hxx"
#include <uhd/version.hpp>
#include <uhd/utils/log.hpp>
#include <uhd/utils/thread.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/types/tune_request.hpp>
#include <uhd/types/tune_result.hpp>
#include <SoDa/Format.hxx>

// Mac OSX doesn't have a clock_gettime, it has
// the microsecond resolution gettimeofday. 
#include <sys/time.h>

const unsigned int SoDa::USRPCtrl::TX_RELAY_CTL = 0x1000;
const unsigned int SoDa::USRPCtrl::TX_RELAY_MON = 0x0800;

const double SoDa::USRPCtrl::rxmode_offset = 1.0e6;


SoDa::USRPCtrl * SoDa::USRPCtrl::singleton_ctrl_obj = NULL; 


SoDa::USRPCtrl::USRPCtrl(Params * _params) : SoDa::Thread("USRPCtrl")
{
  // point to myself.... 
  SoDa::USRPCtrl::singleton_ctrl_obj = this;

  // setup a normal message handler that doesn't babble
  // so much.
  
  // turn off logging below ERROR
  uhd::log::set_console_level(uhd::log::severity_level::warning);
  
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
  
  params = _params;

  
  // make the device.
  usrp = uhd::usrp::multi_usrp::make(params->getRadioArgs());

  if(usrp == NULL) {
    throw SoDa::Exception(SoDa::Format("Unable to allocate USRP unit with arguments = [%0]\n").addS(params->getRadioArgs()), this);
  }

  // We need to find out if this is a B2xx or something like it -- they don't
  // have daughter cards and there are other things to watch out for....
  PropTree tree(usrp, getObjName()); 

  motherboard_name = tree.getStringProp("name", "unknown");

  if((motherboard_name == "B200") || (motherboard_name == "B210")) {
    // B2xx needs a master clock rate of 50 MHz to generate a sample rate of 625 kS/s.
    // B2xx needs a master clock rate of 25 MHz to generate a sample rate of 625 kS/s.
    usrp->set_master_clock_rate(25.0e6);
    debugMsg(SoDa::Format("Initial setup %0").addS(usrp->get_pp_string()));
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
  // truncate to whole number of seconds -- paranoia
  first_gettime = floor(tmp); 

  // get the tx front end subtree
  tx_fe_has_enable = false; 
  tx_fe_subtree = getUSRPFrontEnd(tree, 'T');

  // do we care?  If the tx_fe_subtree doesn't have an enable property,
  // we want to avoid setting and getting it....
  if(tx_fe_subtree != NULL) {
    tx_fe_has_enable = tx_fe_subtree->hasProperty("enabled");
  }


  // get the rx front end subtree
  rx_fe_has_enable = false; 
  rx_fe_subtree = getUSRPFrontEnd(tree, 'R');

  // do we care?  If the rx_fe_subtree doesn't have an enable property, 
  // we want to avoid setting and getting it....
  if(rx_fe_subtree != NULL) {
    rx_fe_has_enable = rx_fe_subtree->hasProperty("enabled");
  }
  if(rx_fe_has_enable) rx_fe_subtree->setBoolProp("enabled",true);


  // do we have lock sensors? 
  std::vector<std::string> rx_snames, tx_snames; 
  rx_snames = usrp->get_rx_sensor_names(0);
  tx_snames = usrp->get_tx_sensor_names(0);
  rx_has_lo_locked_sensor = std::find(rx_snames.begin(), rx_snames.end(), "lo_locked") != rx_snames.end();
  tx_has_lo_locked_sensor = std::find(tx_snames.begin(), tx_snames.end(), "lo_locked") != tx_snames.end();


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
  
  // we need a cmd stream
  cmd_stream = NULL; 
}

/// implement the subscription method
void SoDa::USRPCtrl::subscribeToMailBox(const std::string & mbox_name, 
					SoDa::BaseMBox * mbox_p) {
  if(mbox_name == "CMD") {
    SoDa::CmdMBox * _cmd_stream = dynamic_cast<SoDa::CmdMBox *>(mbox_p);
    if(_cmd_stream != NULL) {
      cmd_stream = _cmd_stream;

      // subscribe to the command stream.
      subid = cmd_stream->subscribe();
    }
    else {
      throw SoDa::Exception(SoDa::Format("Bad mailbox pointer for mailbox named = [%0]\n") 
			     .addS(mbox_name) , this);	
    }
  }
}


void SoDa::USRPCtrl::run()
{
  if(cmd_stream == NULL) {
      throw SoDa::Exception(SoDa::Format("Never got command stream subscription\n"), 
			  this);	
  }
  
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
  debugMsg(SoDa::Format("Sending TX_ANT as [%0]\n").addS(params->getTXAnt()));
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
	debugMsg(SoDa::Format("USRPCtrl processed %0 commands").addI(cmds_processed));
      }
      cmds_processed++; 
      execCommand(cmd);
      exitflag |= (cmd->target == Command::STOP); 
      cmd_stream->free(cmd); 
    }
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

  // not all front ends even have LOs....  
  if(!((sel == 'r') ? rx_has_lo_locked_sensor : tx_has_lo_locked_sensor)) {
    return cur; 
  }

  while(1) {
    uhd::sensor_value_t lo_locked = (sel == 'r') ? usrp->get_rx_sensor("lo_locked",0) : usrp->get_tx_sensor("lo_locked",0);
    if(lo_locked.to_bool()) break;
    else sleep_ms(1);
    if((lock_itercount & 0xfff) == 0) {
      debugMsg(SoDa::Format("Waiting for %0 LO lock to freq = %1 (%2:%3)  count = %4\n")
	       .addC(sel)
	       .addF(req.target_freq, 'e')
	       .addF(req.rf_freq, 'e')
	       .addF(req.dsp_freq, 'e')
	       .addI(lock_itercount));
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

  double target_rx_freq;


  if(sel == 'r') {
    // we round the target frequency to a point that puts the
    // baseband between 150 and 250 KHz below the requested
    // frequency. and an even 100kHz multiple.

    // we need to fiddle this a bit, as we can hang the radio if 
    // the requested frequency is out of range... 
    if(freq < rx_rf_freq_range.start()) freq = rx_rf_freq_range.start();
    if(freq > rx_rf_freq_range.stop()) freq = rx_rf_freq_range.stop();
    
    target_rx_freq = freq; 

    target_rx_freq = 100e3 * floor(freq / 100e3);
    debugMsg(SoDa::Format("freq = %0 1st target = %1\n").addF(freq, 'e').addF(target_rx_freq, 'e'));
    while((freq - target_rx_freq) < 100e3) {
      target_rx_freq -= 100.0e3;
      debugMsg(SoDa::Format("\tfreq = %0 new target = %1\n").addF(freq, 'e').addF(target_rx_freq, 'e'));
    }

    /// This code depends on the integer-N tuning features in libuhd 3.7 and after
    /// earlier libraries will revert to fractional-N tuning and might
    /// see a rise in the noisefloor and perhaps some troublesome spurs
    /// at multiples of the reference frequency divided by the fractional divisor.
    uhd::tune_request_t rx_trequest(target_rx_freq); 
    if(supports_IntN_Mode) {
      // look for a good target RX freq that doesn't cause inband/nearband spurs... 
      applyTargetFreqCorrection(target_rx_freq, target_rx_freq, &rx_trequest);
      debugMsg(SoDa::Format("\t*****target_rx_freq = %0 corrected to %1\n")
	       .addF(target_rx_freq, 'e').addF(rx_trequest.rf_freq, 'e')); 
    }
    else {
      // just use the vanilla tuning.... 
      rx_trequest.target_freq = target_rx_freq;
      rx_trequest.rf_freq = target_rx_freq; 
      rx_trequest.rf_freq_policy = uhd::tune_request_t::POLICY_AUTO;
      rx_trequest.dsp_freq_policy = uhd::tune_request_t::POLICY_AUTO;
    }

    last_rx_tune_result = usrp->set_rx_freq(rx_trequest);
    last_rx_tune_result = checkLock(rx_trequest, 'r', last_rx_tune_result);
    debugMsg(SoDa::Format("RX Tune RF_actual %0 DDC = %1 tuned = %2 target = %3 request  rf = %4 request ddc = %5\n")
	     .addF(last_rx_tune_result.actual_rf_freq, 10, 6, 'e')
	     .addF(last_rx_tune_result.actual_dsp_freq, 10, 6, 'e')
	     .addF(freq, 10, 6, 'e')
	     .addF(target_rx_freq, 10, 6, 'e')
	     .addF(rx_trequest.rf_freq, 10, 6, 'e')
	     .addF(rx_trequest.dsp_freq, 10, 6, 'e'));
  }
  else {
    // On the transmit side, we're using a minimal IF rate and
    // using the full range of the tuning hardware.

    // If the transmitter is off, we retune anyway to park the
    // transmit LO as far away as possible.   This is especially 
    // important for the UBX.
    if(freq < tx_rf_freq_range.start()) freq = tx_rf_freq_range.start();
    if(freq > tx_rf_freq_range.stop()) freq = tx_rf_freq_range.stop();    

    
    uhd::tune_request_t tx_request(freq);
    
    if(tvrt_lo_mode) {
      tx_request.rf_freq_policy = uhd::tune_request_t::POLICY_MANUAL;
      tx_request.rf_freq = tvrt_lo_fe_freq;
    }
    else if(supports_IntN_Mode) {
      // This is a little complicated.
      // For the UBX, at least, the RF oscillator was bleeding through
      // to the output and appearing > -40dBc.   That is not sufficient
      // for HF, as many amplifier chains rely on LPFs rather than BPFs
      // where harmonic suppression is the intent.  However, with a
      // 12.5 MHz step for the RF PLL, we can end up with a honking
      // big spur in the TX output at 12.5 MHz when we're transmitting
      // on 30m or 20m.  Below 30MHz, we turn off the small integer
      // steps -- the default will be something good... one hopes. 
      if(freq > 30.0e6) {
	applyTargetFreqCorrection(freq, last_rx_req_freq, &tx_request);
      }
      else {
	// don't fiddle with adjusting the step size here.
	// we don't want to use fractional mode if we can avoid it,
	// as the RF PLL output <still> bleeds through, but it is
	// within +/- 10 kHz of the carrier.  That's really noxious.
	// Measured results saw less than 40dB suppression. 
	tx_request.args = uhd::device_addr_t("mode_n=integer");	
      }
    }
    else {
      tx_request.rf_freq_policy = uhd::tune_request_t::POLICY_AUTO;
    }

    debugMsg(SoDa::Format("Tuning TX unit to new frequency %0 (request = %1  (%2 %3))\n")
	     .addF(freq, 10, 6, 'e')
	     .addF(tx_request.target_freq, 10, 6, 'e')
	     .addF(tx_request.rf_freq, 10, 6, 'e')
	     .addF(tx_request.dsp_freq, 10, 6, 'e'));

    last_tx_tune_result = usrp->set_tx_freq(tx_request);

    debugMsg(SoDa::Format("Tuned TX unit to new frequency %0 t.rf %1 a.rf %2 t.dsp %3 a.dsp %4\n")
	     .addF(freq, 10, 6, 'e')
	     .addF(last_tx_tune_result.target_rf_freq, 10, 6, 'e')
	     .addF(last_tx_tune_result.actual_rf_freq, 10, 6, 'e')
	     .addF(last_tx_tune_result.target_dsp_freq, 10, 6, 'e')
	     .addF(last_tx_tune_result.actual_dsp_freq, 10, 6, 'e'));

    last_tx_tune_result = checkLock(tx_request, 't', last_tx_tune_result);

    double txfreqs[2];
    txfreqs[0] = usrp->get_tx_freq(0);
    if(tvrt_lo_mode) {
      txfreqs[1] = usrp->get_tx_freq(1);
      debugMsg(SoDa::Format("TX LO = %0  TVRT LO = %1\n")
	       .addF(txfreqs[0], 10, 6, 'e')
	       .addF(txfreqs[1], 10, 6, 'e'));
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
    
    debugMsg(SoDa::Format("Got RX RETUNE request -- frequency %0 diff = %1  last actual_rf %2  dsp %3\n")
	     .addF(freq, 10, 6, 'e')
	     .addF(fdiff, 10, 6, 'e')
	     .addF(last_rx_tune_result.actual_rf_freq, 10, 6, 'e')
	     .addF(last_rx_tune_result.actual_dsp_freq, 10, 6, 'e'));

    if((fdiff < 200e3) && (fdiff > 100e3)) {
      cmd_stream->put(new Command(Command::SET, Command::RX_LO3_FREQ, fdiff)); 
      cmd_stream->put(new Command(Command::REP, Command::RX_FE_FREQ, 
				  last_rx_tune_result.actual_rf_freq - last_rx_tune_result.actual_dsp_freq));
      cmd_stream->put(new Command(Command::REP, Command::RX_CENTER_FREQ, last_rx_tune_result.actual_rf_freq));
      
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
    cmd_stream->put(new Command(Command::REP, Command::RX_CENTER_FREQ, last_rx_tune_result.actual_rf_freq));
    break;

  case Command::LO_CHECK:
    if(cmd->dparms[0] == 0.0) {
      set1stLOFreq(last_rx_req_freq, 'r', false);
    }
    else {
      debugMsg(SoDa::Format("setting lo check freq to %0\n") .addF(cmd->dparms[0], 10, 6, 'e'));
      usrp->set_rx_freq(cmd->dparms[0]);
      // now send a GET lo offset command
      cmd_stream->put(new Command(Command::GET, Command::LO_OFFSET, 0));
    }
    break;

  case Command::TX_RETUNE_FREQ:
  case Command::TX_TUNE_FREQ:
  case Command::TX_FE_FREQ:
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
    rx_rf_gain = rx_rf_gain_range.stop() + cmd->dparms[0];
    if(rx_rf_gain > rx_rf_gain_range.stop()) rx_rf_gain = rx_rf_gain_range.stop();
    if(rx_rf_gain < rx_rf_gain_range.start()) rx_rf_gain = rx_rf_gain_range.start();
    if(!tx_on) {
      usrp->set_rx_gain(rx_rf_gain);
      cmd_stream->put(new Command(Command::REP, Command::RX_RF_GAIN, 
				  usrp->get_rx_gain()));
    }
    break; 
  case Command::TX_RF_GAIN:
    tx_rf_gain = tx_rf_gain_range.stop() + cmd->dparms[0];
    if(tx_rf_gain > tx_rf_gain_range.stop()) tx_rf_gain = tx_rf_gain_range.stop();
    if(tx_rf_gain < tx_rf_gain_range.start()) tx_rf_gain = tx_rf_gain_range.start();
    tmp = cmd->dparms[0];
    debugMsg(SoDa::Format("Setting TX gain to %0 from power %1 range start = %2 stop = %3\n") 
	     .addF(tx_rf_gain, 'e')
	     .addF(tmp, 'e')
	     .addF(tx_rf_gain_range.start(), 'e')
	     .addF(tx_rf_gain_range.stop(), 'e'));
    if(tx_on) {
      usrp->set_tx_gain(tx_rf_gain);
      cmd_stream->put(new Command(Command::REP, Command::TX_RF_GAIN, 
				  usrp->get_tx_gain())); 
    }
    break; 
  case SoDa::Command::TX_STATE: // SET TX_ON
    debugMsg(SoDa::Format("TX_STATE arg = %0\n").addI(cmd->iparms[0]));
    if(cmd->iparms[0] == 1) {
      // set the txgain to where it is supposed to be.
      tx_on = true; 
      bool full_duplex = cmd->iparms[1] != 0;
      if(!full_duplex) usrp->set_rx_gain(0.0); 
      usrp->set_tx_gain(tx_rf_gain); 
      cmd_stream->put(new Command(Command::REP, Command::TX_RF_GAIN, 
				  usrp->get_tx_gain()));
      // to move a birdie away, we bumped the TX LO,, move it back. 
      tx_freq_rxmode_offset = 0.0; // so tuning works.

      // enable the transmit relay
      debugMsg(SoDa::Format("Enabling TX\nCurrent TXENA %0\n")
	       .addI(tx_fe_subtree->getBoolProp("enabled")));
      if(supports_tx_gpio) {
	debugMsg(SoDa::Format("Current GPIO = %0 ")
		 .addU(dboard->get_gpio_out(uhd::usrp::dboard_iface::UNIT_TX), 'x', 4));
      }
      setTXEna(true);

      debugMsg(SoDa::Format("New TXENA %0\n").addU(tx_fe_subtree->getBoolProp("enabled"), 'x', 4));
      if(supports_tx_gpio) {
	debugMsg(SoDa::Format("New GPIO = %0 ")
		 .addU(dboard->get_gpio_out(uhd::usrp::dboard_iface::UNIT_TX), 'x', 4));
      }
      // and tell the TX unit to turn on the TX
      // This avoids the race between CTRL and TX/RX units for setup and teardown.... 
      cmd_stream->put(new Command(Command::SET, Command::TX_STATE, 
				  3, cmd->iparms[1]));
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
      debugMsg(SoDa::Format("Disabling TX -- Got TXENA %0")
	       .addU(tx_fe_subtree->getBoolProp("enabled"), 'x', 4));
      if(supports_tx_gpio) {
	debugMsg(SoDa::Format("Got GPIO = %0 ")
		 .addU(dboard->get_gpio_out(uhd::usrp::dboard_iface::UNIT_TX), 'x', 4));
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
    setAntenna(cmd->sparm, 'r');
    debugMsg(SoDa::Format("Got RX antenna as [%0]\n").addS(usrp->get_rx_antenna()));
    cmd_stream->put(new Command(Command::REP, Command::RX_ANT, usrp->get_rx_antenna()));
    break; 

  case Command::TX_ANT:
    tx_ant = cmd->sparm; 
    setAntenna(cmd->sparm, 't');
    debugMsg(SoDa::Format("Got TX antenna as [%0]\n").addS(usrp->get_tx_antenna()));    
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

  case Command::TX_GAIN_RANGE:
    cmd_stream->put(new Command(Command::REP, Command::TX_GAIN_RANGE,
				tx_rf_gain_range.start(), 
				tx_rf_gain_range.stop()));
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
				SoDa::Format("%0\t%1 to %2 MHz")
				.addS(motherboard_name)
				.addF((rx_rf_freq_range.start() * 1e-6), 10, 6, 'e')
				.addF((rx_rf_freq_range.stop() * 1e-6), 10, 6, 'e').str()));
    reportAntennas(); 
    reportModes();
    reportAFFilters();
    cmd_stream->put(new Command(Command::REP, Command::INIT_SETUP_COMPLETE, 0));
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
    dboard = NULL; 
    supports_tx_gpio = false;
  }

  if(supports_tx_gpio) {

    // now get the old version of the GPIO enable mask.
    unsigned short dir = dboard->get_gpio_ddr(uhd::usrp::dboard_iface::UNIT_TX); 
    unsigned short ctl = dboard->get_pin_ctrl(uhd::usrp::dboard_iface::UNIT_TX);
    unsigned short out = dboard->get_gpio_out(uhd::usrp::dboard_iface::UNIT_TX);
  
    // and print it.
    debugMsg(SoDa::Format("TX GPIO direction = %0  control = %1  output = %2  ctlmask = %3  monmask = %4")
	     .addU(dir, 'x', 4)
	     .addU(ctl, 'x', 4)
	     .addU(out , 'x', 4)
	     .addU(TX_RELAY_CTL, 'x', 4)
	     .addU(TX_RELAY_MON, 'x', 4));


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

  // if the front end has an enable property, set it. 
  setTXFrontEndEnable(val); 

  // if we're enabling, set the power, freq, and other stuff
  if(val) {
    sleep_us(400);    
    // set the tx antenna
    setAntenna(tx_ant, 't');
    // set the tx gain. 
    usrp->set_tx_gain(tx_rf_gain);
    // tx freq
    set1stLOFreq(tx_freq + tx_freq_rxmode_offset, 't', false);  

    double r = usrp->get_tx_rate(); 
    debugMsg(SoDa::Format("TX rate = %0\n") .addF(r, 'e'));
  }

  if(!val) {
    tr_control->setTXOff(); 
  }
  
}

void SoDa::USRPCtrl::setTXFrontEndEnable(bool val) 
{
  if(!tx_fe_has_enable) return; 

  // enable the transmitter (or disable it)
  tx_fe_subtree->setBoolProp("enabled", val);

  debugMsg(SoDa::Format("Got %0 from call to en/dis TX with val = %1")
	   .addI(tx_fe_subtree->getBoolProp("enabled"))
	   .addI(val));
  if(rx_fe_has_enable) {
    debugMsg(SoDa::Format("Got rx_fe enabled = %0 from call to en/dis TX with val = %1")
	     .addI(rx_fe_subtree->getBoolProp("enabled"))
	     .addI(val));
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
  
  debugMsg(SoDa::Format("Setting Transverter LO freq = %0 power = %1 gain = %2\n") 
	   .addF(tvrt_lo_freq, 10, 6, 'e')
	   .addF(power, 'e')
	   .addF(tvrt_lo_gain, 'e'));
  
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
  
  debugMsg(SoDa::Format("LO frequency = %0 power %1  number of channels = %2 target_rf %3 actual rf %4 target dsp %5 actual dsp %6\n")
	   .addF(usrp->get_tx_freq(1), 10, 6, 'e')
	   .addF(usrp->get_tx_gain(1), 10, 6, 'e')
	   .addI(usrp->get_tx_num_channels())
	   .addF(tres.target_rf_freq, 10, 6, 'e')
	   .addF(tres.actual_rf_freq, 10, 6, 'e')
	   .addF(tres.target_dsp_freq, 10, 6, 'e')
	   .addF(tres.actual_dsp_freq, 10, 6, 'e'));

  tvrt_lo_fe_freq = tres.target_rf_freq; 
}

void SoDa::USRPCtrl::disableTransverterLO()
{
  tvrt_lo_mode = false;
  if(!tvrt_lo_capable) return; 
  usrp->set_tx_gain(0.0, 1);
  usrp->set_tx_freq(100.0e6, 1);
}

void SoDa::USRPCtrl::applyTargetFreqCorrection(double target_freq, double avoid_freq, uhd::tune_request_t * treq)
{
  debugMsg(SoDa::Format("######   aTFC(%0...)") .addF(target_freq, 10, 6, 'e')); 

  // if we can't find a really good answer, at least setup a "correct" answer... 
  treq->dsp_freq_policy = uhd::tune_request_t::POLICY_AUTO; 
  treq->target_freq = target_freq; 
  // default
  treq->rf_freq = target_freq; 
  treq->rf_freq_policy = uhd::tune_request_t::POLICY_AUTO; 
  treq->args = uhd::device_addr_t("");
  
  // try three step sizes.  
  double steps[] = {12.5e6, 10.0e6, 50.0e6/3.0};
  int i; 
  if(target_freq < 10.0e6) {
    // take the default...
    debugMsg("\t\treturns default request\n");
    return; 
  }

  double N; 

  // now look for a good integer N setting that puts the reference spurs 
  // at least 1MHz away from our "avoid" frequency...
  for(i = 0; i < 3; i++) {
    N = round(target_freq / steps[i]);
    
    double rf_freq = N * steps[i]; 
    debugMsg(SoDa::Format("\t\tTRY rf_freq = %0 step = %1\n") 
	     .addF(rf_freq, 10, 6, 'e')
	     .addF(steps[i], 10, 6, 'e'));
    if(fabs(rf_freq - avoid_freq) > 1.0e6) {
      // this is an OK choice. 
      debugMsg(SoDa::Format("\t\tACCEPT rf_freq = %0 step = %1\n")
	       .addF(rf_freq, 10, 6, 'e')
	       .addF(steps[i], 10, 6, 'e'));
      treq->rf_freq = rf_freq; 
      treq->rf_freq_policy = uhd::tune_request_t::POLICY_MANUAL; 
      treq->args = uhd::device_addr_t((SoDa::Format("mode_n=integer,int_n_step=%0")
				       .addF(steps[i], 'e')).str());
      return;
    }
  }
  
  // if we get here, we're probably a multiple of 50 MHz.... tough point to make....
  // there are no good answers... 
  N = round(target_freq / steps[0]);
  double rf_freq = N * steps[0];
  debugMsg(SoDa::Format("\t\tTRY rf_freq = %0 step = %1\n") 
	   .addF(rf_freq, 10, 6, 'e')
	   .addF(steps[0], 'e'));
  if(fabs(rf_freq - avoid_freq) < 1.0e6) {
    if(rf_freq > avoid_freq) N = N - 1.0; 
    else N = N + 1.0; 
    rf_freq = N * steps[0]; 
  }
  // this is an OK choice. 
  debugMsg(SoDa::Format("\t\tGRUDGINGLY ACCEPT rf_freq = %0 step = %1\n")
	   .addF(rf_freq, 10, 6, 'e')
	   .addF(steps[0], 'e'));
  
  treq->rf_freq = rf_freq; 
  treq->rf_freq_policy = uhd::tune_request_t::POLICY_MANUAL; 
  treq->args = uhd::device_addr_t((SoDa::Format("mode_n=integer,int_n_step=%0")
				   .addF(steps[0], 'e')).str());


}



void SoDa::USRPCtrl::testIntNMode(bool force_int_N, bool force_frac_N)
{
  uhd::tune_result_t tunres_int, tunres_frac;  

  debugMsg("In testIntNMode\n");
  
  supports_IntN_Mode = false;
  if(force_int_N) {
    debugMsg("Forced IntN Tuning support ON.");
    supports_IntN_Mode = true; 
  }
  else if(force_frac_N) {
    debugMsg("Forced IntN Tuning support OFF.");
    supports_IntN_Mode = false; 
  }
  else if(is_B2xx) {
    supports_IntN_Mode = false; 
    return; 
  }
  else {
    // first, do we have this capability?
    // LFTX/LFRX/BASICRX/BASICTX don't.   Check the RX/RF front end name
    // if it contains LFRX or BASICRX then we don't support intN mode.
    // get the db name for the rx
    std::string dbname;
    dbname = rx_fe_subtree->getStringProp("name");

    std::string lfrx("LFRX");
    std::string basrx("BASICRX");
    if((dbname.compare(0, lfrx.length(), lfrx) == 0) ||
       (dbname.compare(0, basrx.length(), basrx) == 0)) {
      std::cerr << SoDa::Format("This is a simple front end -- no front-end LO. [%0]\n")
	.addS(dbname);
      supports_IntN_Mode = false; 
      return;
    }

    // if we get here, the front end has an LO
    // pick a frequency halfway between the min and max freq for this MB.
    double tf = (rx_rf_freq_range.stop() + rx_rf_freq_range.start()) * 0.5;
    // Now bump it by some silly amount
    tf += 123456.789;

    debugMsg(SoDa::Format("got tf = %0\n").addF(tf, 10, 6, 'e'));
  
    // and tune with and without intN
    uhd::tune_request_t tunreq_int(tf);
    uhd::tune_request_t tunreq_frac(tf);
    tunreq_int.args = uhd::device_addr_t("mode_n=integer,int_n_step=12.5e6");
    debugMsg("About to tune int...\n");
    tunres_int = usrp->set_rx_freq(tunreq_int);
    debugMsg("About to tune frac...\n");
    tunres_frac = usrp->set_rx_freq(tunreq_frac);
  
    // are there differences?
    if(tunres_int.actual_rf_freq != tunres_frac.actual_rf_freq) {
      supports_IntN_Mode = true;
    }

    debugMsg(SoDa::Format("int rf = %0 frac rf = %1  tf = %2\n")
	     .addF(tunres_int.actual_rf_freq, 10, 6, 'e')
	     .addF(tunres_frac.actual_rf_freq, 10, 6, 'e')
	     .addF(tf, 10, 6, 'e'));
  }



  if(supports_IntN_Mode) {
    debugMsg("Supports INT_N tuning mode.\n");
  }
  else {
    debugMsg("Does not support INT_N tuning mode.\n");
  }

  return; 
}

void SoDa::USRPCtrl::reportModes()
{
    cmd_stream->put(new Command(Command::REP, Command::MOD_SEL_ENTRY, 
				"CW_U", ((int) SoDa::Command::CW_U)));
    cmd_stream->put(new Command(Command::REP, Command::MOD_SEL_ENTRY, 
				"USB", ((int) SoDa::Command::USB)));
    cmd_stream->put(new Command(Command::REP, Command::MOD_SEL_ENTRY, 
				"CW_L", ((int) SoDa::Command::CW_L)));
    cmd_stream->put(new Command(Command::REP, Command::MOD_SEL_ENTRY, 
				"LSB", ((int) SoDa::Command::LSB)));
    cmd_stream->put(new Command(Command::REP, Command::MOD_SEL_ENTRY, 
				"AM", ((int) SoDa::Command::AM)));
    cmd_stream->put(new Command(Command::REP, Command::MOD_SEL_ENTRY, 
				"WBFM", ((int) SoDa::Command::WBFM)));
    cmd_stream->put(new Command(Command::REP, Command::MOD_SEL_ENTRY, 
				"NBFM", ((int) SoDa::Command::NBFM)));
}

void SoDa::USRPCtrl::reportAFFilters()
{
    cmd_stream->put(new Command(Command::REP, Command::AF_FILT_ENTRY,
				"100", ((int) SoDa::Command::BW_100)));
    cmd_stream->put(new Command(Command::REP, Command::AF_FILT_ENTRY,
				"500", ((int) SoDa::Command::BW_500)));
    cmd_stream->put(new Command(Command::REP, Command::AF_FILT_ENTRY,
				"2000", ((int) SoDa::Command::BW_2000)));
    cmd_stream->put(new Command(Command::REP, Command::AF_FILT_ENTRY,
				"6000", ((int) SoDa::Command::BW_6000)));
    cmd_stream->put(new Command(Command::REP, Command::AF_FILT_ENTRY,
				"WSPR", ((int) SoDa::Command::BW_WSPR)));
    cmd_stream->put(new Command(Command::REP, Command::AF_FILT_ENTRY,
				"PASS", ((int) SoDa::Command::BW_PASS)));
}

void SoDa::USRPCtrl::reportAntennas() 
{
  std::vector<std::string> rx_ants = usrp->get_rx_antennas();
  for(auto ant: rx_ants) {
    debugMsg(SoDa::Format("Sending RX antenna list element [%0]\n")
	     .addS(ant));
    cmd_stream->put(new Command(Command::REP, Command::RX_ANT_NAME, 
				ant)); 
  }
  std::vector<std::string> tx_ants = usrp->get_tx_antennas();
  for(auto ant: tx_ants) {
    debugMsg(SoDa::Format("Sending TX antenna list element [%0]\n")
	     .addS(ant));
    cmd_stream->put(new Command(Command::REP, Command::TX_ANT_NAME, 
				ant)); 

  }
}


void SoDa::USRPCtrl::setAntenna(const std::string & ant, char sel)
{
  std::vector<std::string> ants; 

  ants = (sel == 'r') ? usrp->get_rx_antennas() : usrp->get_tx_antennas();  

  std::string choice = ants[0];

  for(auto a: ants) {
    if (ant == a) {
      choice = ant; 
      break; 
    }
  }

  if(sel == 'r') {
    usrp->set_rx_antenna(choice); 
  }
  if(sel == 't') {
    usrp->set_tx_antenna(choice); 
  }
  
  return; 
}
