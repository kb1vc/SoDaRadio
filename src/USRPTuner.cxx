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

#include "USRPTuner.hxx"
#include "SoDaBase.hxx"
#include <uhd/utils/thread_priority.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/utils/msg.hpp>
#include <uhd/types/tune_request.hpp>
#include <uhd/types/tune_result.hpp>
#include <boost/format.hpp>

SoDa::USRPTuner::USRPTuner * makeTuner(uhd::usrp::multi_usrp::sptr usrp, 
				       double min_separation, 
				       bool force_fracN,
				       bool force_simple)
{
  bool choose_simple = force_simple; 


  if(!choose_simple) { // look at the module name
    
  }

  if(choose_simple) return new SimpleTuner(usrp); 
  else return IntNTuner(usrp, min_separation, force_fracN);

}

SoDa::USRPTuner::USRPTuner(uhd::usrp::multi_usrp::sptr _usrp, double _min_separation, std::string _unit_name) : Debug(_unit_name)
{
  /// do nothing here, just remember our separation spec.
  min_separation = _min_separation; 
  usrp = _usrp; 

  std::vector<std::string> sensor_names = usrp->get_rx_sensor_names(0); 
  has_lock_detect = std::find(sensor_names.begin(), sensor_names.end(), "lo_locked") != sensor_names.end(); 
}

bool SoDa::IntNTuner::setRXFreq(double rx_freq, double avoid_freq, uhd::tune_result_t & tune_result)
{
  double target_rx_freq = 100e3 * floor(rx_freq / 100.0e3);
  while((rx_freq - target_rx_freq) < 100.0e3) {
    target_rx_freq -= 100.0e3;
  }

  uhd::tune_request_t rx_trequest(target_rx_freq); 
  rx_trequest.target_freq = target_rx_freq;
  rx_trequest.rf_freq = target_rx_freq; 
  rx_trequest.rf_freq_policy = uhd::tune_request_t::POLICY_AUTO;
  rx_trequest.dsp_freq_policy = uhd::tune_request_t::POLICY_AUTO;

  tune_result = usrp->set_rx_freq(rx_trequest);

  debugMsg(boost::format("USRPTuner: RX Tune RF_actual %lf DDC = %lf tuned = %lf target = %lf request  rf = %lf request ddc = %lf\n")
	     % tune_result.actual_rf_freq
	     % tune_result.actual_dsp_freq
	     % rx_freq
	     % target_rx_freq
	     % rx_trequest.rf_freq
	     % rx_trequest.dsp_freq);

  return checkLock(rx_trequest, 'r', tune_result);   
}

bool SoDa::IntNTuner::setTXFreq(double tx_freq, double avoid_freq, uhd::tune_result_t & tune_result)
{
  // On the transmit side, we're using a minimal IF rate and
  // using the full range of the tuning hardware.

  // If the transmitter is off, we retune anyway to park the
  // transmit LO as far away as possible.   This is especially 
  // important for the UBX.
    
  uhd::tune_request_t tx_request(tx_freq);
    
  tx_request.rf_freq_policy = uhd::tune_request_t::POLICY_AUTO;

  debugMsg(boost::format("Tuning TX unit to new frequency %f (request = %f  (%f %f))\n")
	   % tx_freq % tx_request.target_freq % tx_request.rf_freq % tx_request.dsp_freq);

  tune_result = usrp->set_tx_freq(tx_request);

  debugMsg(boost::format("Tuned TX unit to new frequency %g t.rf %g a.rf %g t.dsp %g a.dsp %g\n")
	   % tx_freq
	   % tune_result.target_rf_freq
	   % tune_result.actual_rf_freq
	   % tune_result.target_dsp_freq
	   % tune_result.actual_dsp_freq);

  return checkLock(tx_request, 't', tune_result);
}

bool SoDa::USRPTuner::checkLock(uhd::tune_request_t & req,
			  char sel,
			  uhd::tune_result_t & cur)
{
  int lock_itercount = 0;
  uhd::tune_result_t ret = cur;

  // if we don't do lock, then return.  
  if(!has_lock_detect) return true; 

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

  return true; 
}

