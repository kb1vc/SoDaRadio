/*
  Copyright (c) 2015, Matthew H. Reilly (kb1vc)
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

#include "USRPSNACtrl.hxx"

SoDa::USRPSNACtrl::USRPSNACtrl(Params * _params, CmdMBox * _cmd_stream, double _rx_offset) : 
  SoDa::USRPCtrl(_params, _cmd_stream)
{
  rx_offset = _rx_offset; ///< put it far away from DC spur

  rx_need_fe_retune = true;
  tx_need_fe_retune = true;   
}


void SoDa::USRPSNACtrl::run()
{
  uhd::set_thread_priority_safe(); 
  // now do the event loop.  we watch
  // for commands and responses on the command stream.
  debugMsg("SNA Starting Run Loop");  
  // do the initial commands
  cmd_stream->put(new Command(Command::SET, Command::RX_SAMP_RATE,
			     params->getRXRate())); 
  cmd_stream->put(new Command(Command::SET, Command::TX_SAMP_RATE,
			     params->getTXRate()));

  cmd_stream->put(new Command(Command::SET, Command::RX_ANT, 
			      "RX2")); 
  cmd_stream->put(new Command(Command::SET, Command::TX_ANT,
			      "TX/RX"));
  cmd_stream->put(new Command(Command::SET, Command::CLOCK_SOURCE,
			     params->getClockSource())); 

  cmd_stream->put(new Command(Command::SET, Command::TX_RF_GAIN, 100.0)); 
  cmd_stream->put(new Command(Command::SET, Command::RX_RF_GAIN, 100.0));

  // transmitter is off
  tx_on = false; 
  cmd_stream->put(new Command(Command::SET, Command::TX_STATE, 0)); 
  

  // turn off DC offset compensation
  usrp->set_rx_dc_offset(std::complex<double>(0.0,0.0));

  bool exitflag = false;
  unsigned int cmds_processed = 0;
  unsigned int loopcount = 0; 
  while(!exitflag) {
    loopcount++; 
    Command * cmd = cmd_stream->get(subid);
    // we don't wait long here -- we need to go through 100 tics per second
    if(cmd == NULL) {
      boost::this_thread::sleep(boost::posix_time::milliseconds(1));
    }
    else {
      // process the command.
      if((cmds_processed & 0xff) == 0) {
	debugMsg(boost::format("SNA processed %d commands") % cmds_processed);
      }
      execCommand(cmd);
      cmds_processed++; 
      exitflag |= (cmd->target == Command::STOP); 
      cmd_stream->free(cmd); 
    }
  }


}

void SoDa::USRPSNACtrl::setRXFreq()
{
  double new_freq = current_sweep_freq - rx_offset;  
  uhd::tune_request_t tunereq(new_freq);
  tunereq.args = uhd::device_addr_t("mode_n=integer");

  debugMsg(boost::format("SNA Setting RX frequency to %g MHz\n") % 
	   (new_freq * 1e-6));

  double new_ddc_freq = rx_last_fe_freq - new_freq;

  std::string tune_mode;
  uhd::tune_result_t rx_tune_res;        

  if(1 || rx_need_fe_retune || (new_ddc_freq > 15.0e6)) {
    rx_tune_res = usrp->set_rx_freq(tunereq);
    rx_tune_res = checkLock(tunereq, 'r', rx_tune_res);     
    rx_need_fe_retune = false;
    rx_last_fe_freq = rx_tune_res.actual_rf_freq;
    tune_mode = std::string("FE+DDC");
  }
  else {
    tunereq.rf_freq_policy = uhd::tune_request_t::POLICY_MANUAL;
    tunereq.rf_freq = rx_last_fe_freq;
    tunereq.dsp_freq_policy = uhd::tune_request_t::POLICY_MANUAL;    
    tunereq.dsp_freq = new_ddc_freq;
    rx_tune_res = usrp->set_rx_freq(tunereq);
    tune_mode = std::string("DDC");    
  }
  debugMsg(boost::format("RX Tune mode = %s new_ddc_freq = %g req freq %g got freq %g result %s\n") 
	   % tune_mode % new_ddc_freq % new_freq 
	   % usrp->get_rx_freq()
	   % rx_tune_res.to_pp_string());

}

void SoDa::USRPSNACtrl::setTXFreq()
{
  uhd::tune_request_t tunereq(current_sweep_freq);
  tunereq.args = uhd::device_addr_t("mode_n=integer");
  debugMsg(boost::format("SNA Setting TX frequency to %g MHz\n") % 
	   (current_sweep_freq * 1e-6));

  double new_ddc_freq = current_sweep_freq - tx_last_fe_freq; 
  uhd::tune_result_t tx_tune_res; 
  std::string tune_mode; 
  if(1 || tx_need_fe_retune || (new_ddc_freq < -15.0e6)) {
    tx_tune_res = usrp->set_tx_freq(tunereq);
    tx_tune_res = checkLock(tunereq, 't', tx_tune_res);     
    tx_need_fe_retune = false;
    tx_last_fe_freq = tx_tune_res.actual_rf_freq;    
    tune_mode = std::string("FE+DDC");    
  }
  else {
    tunereq.rf_freq_policy = uhd::tune_request_t::POLICY_MANUAL;
    tunereq.rf_freq = tx_last_fe_freq;    
    tunereq.dsp_freq_policy = uhd::tune_request_t::POLICY_MANUAL;    
    tunereq.dsp_freq = new_ddc_freq;
    tx_tune_res = usrp->set_tx_freq(tunereq);
    tune_mode = std::string("DDC");
    // boost::this_thread::sleep(boost::posix_time::milliseconds(1000000));    

  }

  debugMsg(boost::format("TX Tune mode = %s new_ddc_freq = %g req freq %g got freq %g result %s\n") 
	   % tune_mode % new_ddc_freq % current_sweep_freq 
	   % usrp->get_tx_freq()
	   % tx_tune_res.to_pp_string());

}

double SoDa::USRPSNACtrl::correctFreq(double freq) 
{
  // frequencies that are multiples of 10MHz are problematic. 
  // 50 MHz multiples are particularly bad. 
  // shift such frequencies by just a little bit....
  double eps = 87.0e3; 
  double mult = floor(freq / 2.5e6); 
  if(fabs(freq - (mult * 2.5e6)) < eps) {
    debugMsg(boost::format("Correcting problem frequency from %g to %g\n")
	     % freq % (freq + eps * 2.0));
    return freq + eps * 2.0;
  }
  else return freq; 
}

void SoDa::USRPSNACtrl::doStep() 
{
  double current_time; 
  // the RX unit completed its collection, now we move on. 

  // check the current frequency... 
  current_sweep_freq = correctFreq(current_sweep_freq); 

  // set the TX freq
  setTXFreq();
  // set the RX freq
  setRXFreq();  
  // find the time of the last command completion
  // allow 10mS settling time?
  current_time = usrp->get_time_now().get_real_secs() + 10e-3;
  cmd_stream->put(new Command(Command::SET, Command::SNAI_SCAN_READY, 
			      current_time, current_time + time_per_step, 
			      current_sweep_freq));
  sweep_state = WAIT_FOR_RX;    
  current_sweep_freq += step_sweep_freq; 

  return; 
}

void SoDa::USRPSNACtrl::execCommand(Command * cmd)
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


void SoDa::USRPSNACtrl::execSetCommand(Command * cmd)
{
  debugMsg(boost::format("SNA Got message [%s]\n") % cmd->toString());
  switch (cmd->target) {
  case Command::TX_STATE:
    if(cmd->iparms[0] == 1) {
      // set the txgain to where it is supposed to be.
      debugMsg(boost::format("SNA Setting TX state on tx_gain = %g\n") % tx_rf_gain);
      tx_on = true; 
      usrp->set_tx_gain(tx_rf_gain); 
      cmd_stream->put(new Command(Command::REP, Command::TX_RF_GAIN, 
				  usrp->get_tx_gain()));

      tx_fe_subtree->access<bool>("enabled").set(true);
      debugMsg(boost::format("SNA Got %d from call to en/dis TX with val = %d")
	       % tx_fe_subtree->access<bool>("enabled").get() % true);


      debugMsg(boost::format("SNA New TXENA %d\n") % tx_fe_subtree->access<bool>("enabled").get());
      // and tell the TX unit to turn on the TX
      cmd_stream->put(new Command(Command::SET, Command::TX_STATE, 
				  3));
    }
    if(cmd->iparms[0] == 0) {
      tx_on = false; 
      // set txgain to zero
      usrp->set_tx_gain(0.0);

      tx_fe_subtree->access<bool>("enabled").set(false);
      debugMsg(boost::format("SNA Got %d from call to en/dis TX with val = %d")
	       % tx_fe_subtree->access<bool>("enabled").get() % false);
      
      debugMsg(boost::format("Disabling TX\nGot TXENA %d") % tx_fe_subtree->access<bool>("enabled").get());
    }
    break;
  case Command::SNA_SCAN_START:
    start_sweep_freq = cmd->dparms[0];
    current_sweep_freq = start_sweep_freq; 
    end_sweep_freq = cmd->dparms[1];
    step_sweep_freq = cmd->dparms[2];
    time_per_step = cmd->dparms[3];
    tx_need_fe_retune = rx_need_fe_retune = true; 
    debugMsg(boost::format("SNA SCAN_START: %g %g %g %g\n") 
	     % start_sweep_freq % end_sweep_freq 
	     % step_sweep_freq % time_per_step); 
    doStep();
    break; 
  default:
    debugMsg(boost::format("SNA About to call USRPCtrl::execSetCommand [%s]\n") % 
	   cmd->toString());
    USRPCtrl::execSetCommand(cmd);
    debugMsg("SNA Called USRPCtrl::execSetCommand");      
    break; 
  }
}

void SoDa::USRPSNACtrl::execGetCommand(Command * cmd)
{
  switch (cmd->target) {
  default:
    USRPCtrl::execGetCommand(cmd); 
    break; 
  }
}

void SoDa::USRPSNACtrl::execRepCommand(Command * cmd)
{
  switch (cmd->target) {
  case Command::SNA_SCAN_REPORT:
    if(current_sweep_freq >= end_sweep_freq) {
      // we just completed the last step in the sweep. 
      sweep_state = IDLE; 
      cmd_stream->put(new Command(Command::REP, Command::SNA_SCAN_END));
    }
    else {
      doStep();
    }
    break; 
  default:
    USRPCtrl::execRepCommand(cmd); 
    break; 
  }
}

