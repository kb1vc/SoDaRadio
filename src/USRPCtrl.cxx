
/*
  Copyright (c) 2012,2026 Matthew H. Reilly (kb1vc)
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

#include <sys/time.h>

namespace SoDa {
  const unsigned int USRPCtrl::TX_RELAY_CTL = 0x1000;
  const unsigned int USRPCtrl::TX_RELAY_MON = 0x0800;

  const double USRPCtrl::rxmode_offset = 1.0e6;

  USRPCtrl::USRPCtrl(ParamsPtr _params) : RadioControl(_params, "USRPCtrl")
  {
  
    // turn off logging below ERROR
    uhd::log::set_console_level(uhd::log::severity_level::warning);

    // let the library know that the usrp thread should take the default level
    // but treat it as a real-time thread.
    uhd::set_thread_priority_safe();   

  
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
      throw SDR::Exception(SoDa::Format("Unable to allocate USRP unit with arguments = [%0]\n").addS(params->getRadioArgs()), self.lock());
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
      usrp->set_tx_subdev_spec(std::string("A:A"), 0);      
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

    // Probe whether the target IF rate is supported; fall back to 2.5 MS/s if not.
    double hw_rate = params->getRXRate();  // 625000
    uhd::meta_range_t rx_rates = usrp->get_rx_rates(0);
    if (rx_rates.start() > hw_rate) {
        hw_rate = 2500000.0;
        std::cerr << SoDa::Format("USRPCtrl: 625 kS/s not supported (min=%0 S/s); using 2.5 MS/s with 4:1 resampling\n")
            .addF(rx_rates.start(), 'g');
        params->setHWSampleRate(hw_rate);
    }
    usrp->set_rx_rate(hw_rate);
    usrp->set_tx_rate(hw_rate);

    // setup the control IO pins (for TX/RX external relay)
    // Note that there are no GPIOs available for the B2xx right now.
    initControlGPIO();

    // setup a widget to control external devices 
    tr_control = USRPTRControl::makeUSRPTRControl(usrp);     

    // turn off the transmitter
    locSetTXEna(false);

    // if we are in integer-N mode, setup the step table.
    testIntNMode(params->forceIntN(), params->forceFracN()); 

  }

  bool USRPCtrl::isLOLocked(RXTX rxtx) {
    switch(rxtx) {
    case SoDa::RX:
      return !rx_has_lo_locked_sensor || usrp->get_rx_sensor("lo_locked", 0).to_bool(); 
      break;
    case SoDa::TX:
      return !tx_has_lo_locked_sensor || usrp->get_tx_sensor("lo_locked", 0).to_bool();     
      break; 
    default:
      return true;
    };
  }


  uhd::tune_result_t USRPCtrl::checkLock(uhd::tune_request_t & req, char sel, uhd::tune_result_t & cur)
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

  double USRPCtrl::getLOFreq(SoDa::RXTX rxtx) {
    if(rxtx == SoDa::RX) {
      return last_rx_tune_result.actual_rf_freq + last_rx_tune_result.actual_dsp_freq;     
    }
    else if(rxtx == SoDa::TX) {
      return last_tx_tune_result.actual_rf_freq + last_tx_tune_result.actual_dsp_freq; 
    }

    return 0.0; /// no idea how we'd get here. 
  }

  double USRPCtrl::setLOFreq(double freq, SoDa::RXTX rxtx) 
  {
    double target_rx_freq;

    if(rxtx == SoDa::RX) {
      if(freq < rx_rf_freq_range.start()) freq = rx_rf_freq_range.start();
      if(freq > rx_rf_freq_range.stop()) freq = rx_rf_freq_range.stop();
    
      target_rx_freq = freq; 

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
	       .addF(last_rx_tune_result.actual_rf_freq, 'e', 10, 6)
	       .addF(last_rx_tune_result.actual_dsp_freq, 'e', 10, 6)
	       .addF(freq, 'e', 10, 6)
	       .addF(target_rx_freq, 'e', 10, 6)
	       .addF(rx_trequest.rf_freq, 'e', 10, 6)
	       .addF(rx_trequest.dsp_freq, 'e', 10, 6));
      return last_rx_tune_result.actual_rf_freq - last_rx_tune_result.actual_dsp_freq;
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
    
      if(supports_IntN_Mode) {
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
	       .addF(freq, 'e', 10, 6)
	       .addF(tx_request.target_freq, 'e', 10, 6)
	       .addF(tx_request.rf_freq, 'e', 10, 6)
	       .addF(tx_request.dsp_freq, 'e', 10, 6));

      last_tx_tune_result = usrp->set_tx_freq(tx_request);

      debugMsg(SoDa::Format("Tuned TX unit to new frequency %0 t.rf %1 a.rf %2 t.dsp %3 a.dsp %4\n")
	       .addF(freq, 'e', 10, 6)
	       .addF(last_tx_tune_result.target_rf_freq, 'e', 10, 6)
	       .addF(last_tx_tune_result.actual_rf_freq, 'e', 10, 6)
	       .addF(last_tx_tune_result.target_dsp_freq, 'e', 10, 6)
	       .addF(last_tx_tune_result.actual_dsp_freq, 'e', 10, 6));

      last_tx_tune_result = checkLock(tx_request, 't', last_tx_tune_result);

      return last_tx_tune_result.actual_rf_freq + last_tx_tune_result.actual_dsp_freq;
    
      double txfreqs[2];
      txfreqs[0] = usrp->get_tx_freq(0);
      if(tvrt_lo_mode) {
	txfreqs[1] = usrp->get_tx_freq(1);
	debugMsg(SoDa::Format("TX LO = %0  TVRT LO = %1\n")
		 .addF(txfreqs[0], 'e', 10, 6)
		 .addF(txfreqs[1], 'e', 10, 6));
      }
      return last_tx_tune_result.actual_rf_freq + last_tx_tune_result.actual_dsp_freq;
    }
  }

  float USRPCtrl::setRFGain(float gain, SoDa::RXTX rxtx) {
    if(rxtx == SoDa::RX) {
      // dparameters ranges from 0 to 100... normalize this
      // to the actual range; 
      rx_rf_gain = rx_rf_gain_range.stop() + gain;
      if(rx_rf_gain > rx_rf_gain_range.stop()) rx_rf_gain = rx_rf_gain_range.stop();
      if(rx_rf_gain < rx_rf_gain_range.start()) rx_rf_gain = rx_rf_gain_range.start();
      if(!tx_on) {
	usrp->set_rx_gain(rx_rf_gain);
      }
      return usrp->get_rx_gain();
    }
    if(rxtx == SoDa::TX) {
      tx_rf_gain = tx_rf_gain_range.stop() + gain;
      if(tx_rf_gain > tx_rf_gain_range.stop()) tx_rf_gain = tx_rf_gain_range.stop();
      if(tx_rf_gain < tx_rf_gain_range.start()) tx_rf_gain = tx_rf_gain_range.start();
      auto tmp = gain;
      debugMsg(Format("Setting TX gain to %0 from power %1 range start = %2 stop = %3\n") 
	       .addF(tx_rf_gain, 'e')
	       .addF(tmp, 'e')
	       .addF(tx_rf_gain_range.start(), 'e')
	       .addF(tx_rf_gain_range.stop(), 'e'));
      if(tx_on) {
	usrp->set_tx_gain(tx_rf_gain);
      }
      return usrp->get_tx_gain(); 
    }
    return 0.0; 
  }


  std::string USRPCtrl::getHardwareDescription() {
    return SoDa::Format("%0\t%1 to %2 MHz")
      .addS(motherboard_name)
      .addF((rx_rf_freq_range.start() * 1e-6), 'e', 10, 6)
      .addF((rx_rf_freq_range.stop() * 1e-6), 'e', 10, 6).str();
  }

  void USRPCtrl::initControlGPIO()
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

  void USRPCtrl::locSetTXEna(bool val)
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
      setAntenna(tx_ant, SoDa::TX);
      // write the TX gain now that tx_on is true
      usrp->set_tx_gain(tx_rf_gain);
      std::cerr << SoDa::Format("USRPCtrl: setTXEna TX gain set to %0\n")
        .addF(tx_rf_gain, 'f', 2);
    }

    if(!val) {
      tr_control->setTXOff();
    }
  }

  void USRPCtrl::setTXFrontEndEnable(bool val) 
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
 
  bool USRPCtrl::getTXEna()
  {
    unsigned int enabits = 0; 
    if(supports_tx_gpio) {
      enabits = dboard->get_gpio_out(uhd::usrp::dboard_iface::UNIT_TX);
    }

    return ((enabits & TX_RELAY_CTL) != 0); 
  }


  bool USRPCtrl::getTXRelayOn()
  {
    unsigned int enabits = 0; 
    if(supports_tx_gpio) {
      enabits = dboard->read_gpio(uhd::usrp::dboard_iface::UNIT_TX);
    }

    return ((enabits & TX_RELAY_MON) != 0); 
  }


  void USRPCtrl::applyTargetFreqCorrection(double target_freq, double avoid_freq, uhd::tune_request_t * treq)
  {
    debugMsg(SoDa::Format("######   aTFC(%0...)") .addF(target_freq, 'e', 10, 6)); 

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
	       .addF(rf_freq, 'e', 10, 6)
	       .addF(steps[i], 'e', 10, 6));
      if(fabs(rf_freq - avoid_freq) > 1.0e6) {
	// this is an OK choice. 

	debugMsg(SoDa::Format("\t\tACCEPT rf_freq = %0 step = %1\n")
		 .addF(rf_freq, 'e', 10, 6)
		 .addF(steps[i], 'e', 10, 6));
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
	     .addF(rf_freq, 'e', 10, 6)
	     .addF(steps[0], 'e'));
    if(fabs(rf_freq - avoid_freq) < 1.0e6) {
      if(rf_freq > avoid_freq) N = N - 1.0; 
      else N = N + 1.0; 
      rf_freq = N * steps[0]; 
    }
    // this is an OK choice. 
    debugMsg(SoDa::Format("\t\tGRUDGINGLY ACCEPT rf_freq = %0 step = %1\n")
	     .addF(rf_freq, 'e', 10, 6)
	     .addF(steps[0], 'e'));
  
    treq->rf_freq = rf_freq; 
    treq->rf_freq_policy = uhd::tune_request_t::POLICY_MANUAL; 
    treq->args = uhd::device_addr_t((SoDa::Format("mode_n=integer,int_n_step=%0")
				     .addF(steps[0], 'e')).str());


  }



  void USRPCtrl::testIntNMode(bool force_int_N, bool force_frac_N)
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

      debugMsg(SoDa::Format("got tf = %0\n").addF(tf, 'e', 10, 6));
  
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
	       .addF(tunres_int.actual_rf_freq, 'e', 10, 6)
	       .addF(tunres_frac.actual_rf_freq, 'e', 10, 6)
	       .addF(tf, 'e', 10, 6));
    }


    if(supports_IntN_Mode) {
      debugMsg("Supports INT_N tuning mode.\n");
    }
    else {
      debugMsg("Does not support INT_N tuning mode.\n");
    }

    return; 
  }

  std::vector<std::string>  USRPCtrl::listAntennas(SoDa::RXTX rxtx) 
  {
    if(rxtx == SoDa::RX) {
      return usrp->get_rx_antennas();
    }
    else if(rxtx == SoDa::TX) {
      return usrp->get_tx_antennas();    
    }
    else {
      return std::vector<std::string>(0);
    }
  }

  void USRPCtrl::setSampleRate(float rate, SoDa::RXTX rxtx) {
    if(rxtx == SoDa::RX) {
      usrp->set_rx_rate(rate);
    }
    else if(rxtx == SoDa::TX) {
      usrp->set_tx_rate(rate);
    }
  }

  float USRPCtrl::getSampleRate(SoDa::RXTX rxtx) {
    if(rxtx == SoDa::RX) {
      return usrp->get_rx_rate();
    }
    else if(rxtx == SoDa::TX) {
      return usrp->get_tx_rate();
    }
    return 1; 
  }
  
  std::string USRPCtrl::getAntenna(SoDa::RXTX rxtx) {
    if(rxtx == SoDa::RX) {
      return usrp->get_rx_antenna();
    }
    else {
      return usrp->get_tx_antenna();
    }
  }
    
  void USRPCtrl::setAntenna(const std::string & ant, SoDa::RXTX rxtx)
  {
    std::vector<std::string> ants;

    ants = (rxtx == SoDa::RX) ? usrp->get_rx_antennas() : usrp->get_tx_antennas();

    // Exact match first.
    for(auto & a : ants) {
      if(ant == a) {
        if(rxtx == SoDa::RX) usrp->set_rx_antenna(a);
        else                  usrp->set_tx_antenna(a);
        return;
      }
    }

    // Case-insensitive match.
    std::string ant_lc = ant;
    std::transform(ant_lc.begin(), ant_lc.end(), ant_lc.begin(), ::tolower);
    for(auto & a : ants) {
      std::string a_lc = a;
      std::transform(a_lc.begin(), a_lc.end(), a_lc.begin(), ::tolower);
      if(ant_lc == a_lc) {
        if(rxtx == SoDa::RX) usrp->set_rx_antenna(a);
        else                  usrp->set_tx_antenna(a);
        return;
      }
    }

    // No match: warn and list valid names.
    std::string dir = (rxtx == SoDa::RX) ? "RX" : "TX";
    std::cerr << "Warning: antenna \"" << ant << "\" not found for " << dir
              << ". Valid names:";
    for(auto & a : ants) std::cerr << " \"" << a << "\"";
    std::cerr << ".  No change made.\n";
  }

  bool USRPCtrl::setClockSource(Command::ClockSource src) {
    if(src == Command::ClockSource::EXTERNAL) {
      usrp->set_clock_source("external");
    }
    else {
      usrp->set_clock_source("internal");    
    }
    return true; 
  }

  Command::ClockSource USRPCtrl::getClockSource() {
    if(usrp->get_clock_source(0) == "external") {
      return Command::ClockSource::EXTERNAL;
    }
    else {
      return Command::ClockSource::INTERNAL; 
    }
  }
}
