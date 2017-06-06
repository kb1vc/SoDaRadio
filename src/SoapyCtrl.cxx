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
#include <boost/format.hpp>
#include <boost/property_tree/exceptions.hpp>


// Mac OSX doesn't have a clock_gettime, it has
// the microsecond resolution gettimeofday. 
#include <sys/time.h>

static void SoDaLogHandler(const SoapySDRLogLevel lev, const char * message) 
{
  switch(lev) {
  case SoapySDRLogLevel::SOAPY_SDR_FATAL:
    std::cerr << boost::format("FATAL: [%s]\n") % message;     
    break; 
  case SoapySDRLogLevel::SOAPY_SDR_INFO:
    // do nothing
    // break; 
  default:
    std::cerr << boost::format("[%s]  level = %d\n") % message % lev;       
    break; 
  }

}

SoDa::SoapyCtrl::SoapyCtrl(const std::string & driver_name, Params * _params, CmdMBox * _cmd_stream) : SoDa::SoDaThread("SoapyCTRL")
{
  // register a log handler.  
  SoapySDR::registerLogHandler(SoDaLogHandler);

  // we're not ready yet. 
  is_ready = false; 

  // initialize variables
  last_rx_tune_freq = 0.0; // at least this is a number...
  tx_on = false;
  rx_rf_gain = 0.0;
  tx_rf_gain = 0.0;
  tx_freq = 0.0;
  tx_freq_rxmode_offset = 0.0;
  rxmode_offset = 1.52e6; // oddball offset. 
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

  // get the capabilities
  rx_rf_gain_range = radio->getGainRange(SOAPY_SDR_RX, 0);
  tx_rf_gain_range = radio->getGainRange(SOAPY_SDR_TX, 0);   

  SoapySDR::RangeList rxrl = radio->getFrequencyRange(SOAPY_SDR_RX, 0);
  SoapySDR::RangeList txrl = radio->getFrequencyRange(SOAPY_SDR_TX, 0);  

  rx_freq_range = SoapySDR::Range(rxrl[0].minimum(), rxrl[rxrl.size()-1].maximum());
  tx_freq_range = SoapySDR::Range(txrl[0].minimum(), txrl[txrl.size()-1].maximum());  


  // it appears that the only antenna port that works correctly on my
  // widget is LNAH
  radio->setAntenna(SOAPY_SDR_RX, 0, "LNAL");
  radio->setAntenna(SOAPY_SDR_TX, 0, "BAND1");  

  // for now set the master clock rate to 40MHz as that gets us the 
  // sample rate we need. 
  radio->setMasterClockRate(40.0e6);

  // set the sample rates  
  radio->setSampleRate(SOAPY_SDR_RX, 0, params->getRXRate());
  radio->setSampleRate(SOAPY_SDR_TX, 0, params->getTXRate());

  // setup the DC correction to auto.
  radio->setDCOffsetMode(SOAPY_SDR_RX, 0, true);
  radio->setDCOffsetMode(SOAPY_SDR_TX, 0, false); 

  // guess at the IQ balance for now
  radio->setIQBalance(SOAPY_SDR_RX, 0, std::complex<double>(0.99, 0.01)); 
  // initialize the GPIO pins (for TX/RX external relay)
  initControlGPIO();

  // setup the TR control widget
  // do nothing for now... 

  // put us in RX mode. 
  setTXEna(false);

  // now report out
  debugMsg(boost::format("\n\n\n\nTX sample rate: %g RX sample rate: %g\n\n\n\n") 
	   % radio->getSampleRate(SOAPY_SDR_TX, 0) % radio->getSampleRate(SOAPY_SDR_RX, 0));

  // 
}

void SoDa::SoapyCtrl::initControlGPIO() 
{
  // get the list of GPIO registers we can manipulate. 
  GPIO_list = radio->listGPIOBanks(); 

  // we'll use the first register. 
  tr_control_reg = GPIO_list[0]; 

  // TX on is bit 0, RX on is bit 1
  radio->writeGPIODir(tr_control_reg, 0x3); 

  // go to RX mode
  setTXEna(false); 
}

void SoDa::SoapyCtrl::setTXEna(bool tx_on)
{
  unsigned int dir = tx_on ? 1 : 2; 

  radio->writeGPIO(tr_control_reg, dir, 0x3); 
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
  kwl[0]["cacheCalibrations"] = "1";
  radio = SoapySDR::Device::make(kwl[0]);

  model_name = kwl[0]["name"]; 
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

  double freq, fdiff; 
  if(cmd->cmd != Command::SET) {
    std::cerr << "execSetCommand got a non-set command!  " << cmd->toString() << std::endl;
    return; 
  }

  switch (cmd->target) {
  case Command::RX_RETUNE_FREQ:
    last_rx_tune_freq = cmd->dparms[0]; 
    freq = cmd->dparms[0];
    fdiff = freq - last_rx_tune_freq;
    
    debugMsg(boost::format("Got RX RETUNE request -- frequency %f diff = %f  last actual_rf %f\n")
	     % freq % fdiff % last_rx_tune_freq);

    if((fdiff < 200e3) && (fdiff > 100e3)) {
      cmd_stream->put(new Command(Command::SET, Command::RX_LO3_FREQ, fdiff)); 
      cmd_stream->put(new Command(Command::REP, Command::RX_FE_FREQ, last_rx_tune_freq));
      break; 
    }
    // else -- treat this as a RX_TUNE_FREQ request.
  case Command::RX_TUNE_FREQ:    
  case Command::RX_FE_FREQ:
    last_rx_tune_freq = cmd->dparms[0]; 
    set1stLOFreq(cmd->dparms[0], SOAPY_SDR_RX, cmd->target != Command::RX_TUNE_FREQ);
    fdiff = freq - last_rx_tune_freq;    
    cmd_stream->put(new Command(Command::SET, Command::RX_LO3_FREQ, fdiff));     
    cmd_stream->put(new Command(Command::REP, Command::RX_FE_FREQ, last_rx_tune_freq)); 
    break;

  case Command::LO_CHECK:
    if(cmd->dparms[0] == 0.0) {
      set1stLOFreq(last_rx_tune_freq, 'r', false);
    }
    else {
      debugMsg(boost::format("setting lo check freq to %lf\n") % cmd->dparms[0]);
      radio->setFrequency(SOAPY_SDR_RX, 0, cmd->dparms[0]);
      // now send a GET lo offset command
      cmd_stream->put(new Command(Command::GET, Command::LO_OFFSET, 0));

      std::cerr << "Temporary fix for DC OFFSET\n";     
      radio->setIQBalance(SOAPY_SDR_TX, 0, std::complex<double>(0.999563,0.000293756));
      radio->setDCOffset(SOAPY_SDR_TX, 0, std::complex<double>(-0.6, 0.1));
      
    }
    break;

  case Command::TX_RETUNE_FREQ:
  case Command::TX_TUNE_FREQ:
  case Command::TX_FE_FREQ:
    debugMsg(boost::format("\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n!!! GOT TX TUNE REQ !!!!!!!!\n!!!!!!!!!!!!!!!!!!\n"));
    debugMsg(boost::format(" freq = %10lg\n") % freq);
    debugMsg(boost::format("\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n!!! GOT TX TUNE REQ !!!!!!!!\n!!!!!!!!!!!!!!!!!!"));
    set1stLOFreq(cmd->dparms[0] + tx_freq_rxmode_offset, SOAPY_SDR_TX, false);
    tx_freq = cmd->dparms[0]; 
    cmd_stream->put(new Command(Command::REP, Command::TX_FE_FREQ, last_tx_tune_freq)); 
    break; 

  case Command::RX_SAMP_RATE:
    radio->setSampleRate(SOAPY_SDR_RX, 0, cmd->dparms[0]);
    cmd_stream->put(new Command(Command::REP, Command::RX_SAMP_RATE, 
				radio->getSampleRate(SOAPY_SDR_RX, 0)));
    break; 
  case Command::TX_SAMP_RATE:
    radio->setSampleRate(SOAPY_SDR_TX, 0, cmd->dparms[0]);
    cmd_stream->put(new Command(Command::REP, Command::TX_SAMP_RATE, 
				radio->getSampleRate(SOAPY_SDR_TX, 0)));
    break;
    
  case Command::RX_RF_GAIN:
    // dparameters ranges from 0 to 100... normalize this
    // to the actual range; 
    rx_rf_gain = rx_rf_gain_range.minimum() + cmd->dparms[0] * 0.01 * (rx_rf_gain_range.maximum() - rx_rf_gain_range.minimum());
    if(!tx_on) {
      radio->setGain(SOAPY_SDR_RX, 0, rx_rf_gain);
      cmd_stream->put(new Command(Command::REP, Command::RX_RF_GAIN, 
				  radio->getGain(SOAPY_SDR_RX, 0)));
    }
    break; 
  case Command::TX_RF_GAIN:
    tx_rf_gain = tx_rf_gain_range.minimum() + cmd->dparms[0] * 0.01 * (tx_rf_gain_range.maximum() - tx_rf_gain_range.minimum());
    if(tx_on) {
      radio->setGain(SOAPY_SDR_TX, 0, tx_rf_gain);
      cmd_stream->put(new Command(Command::REP, Command::TX_RF_GAIN, 
				  radio->getGain(SOAPY_SDR_TX, 0)));
    }
    break; 
  case SoDa::Command::TX_STATE: // SET TX_ON
    debugMsg(boost::format("TX_STATE arg = %d\n") % cmd->iparms[0]);
    if(cmd->iparms[0] == 1) {
      // set the txgain to where it is supposed to be.
      tx_on = true; 
      // set the rxgain to "a little bit" as the LimeSDR 
      // seems to have a little bit more isolation on tx to rx 
      // than the USRP. 
      radio->setGain(SOAPY_SDR_RX, 0, 20.0);
      radio->setGain(SOAPY_SDR_TX, 0, tx_rf_gain);      
      debugMsg(boost::format("TX gain set to %f got %f\n") % tx_rf_gain % radio->getGain(SOAPY_SDR_TX, 0));
      cmd_stream->put(new Command(Command::REP, Command::TX_RF_GAIN, 
				  radio->getGain(SOAPY_SDR_TX, 0)));
      // to move a birdie away, we bumped the TX LO,, move it back. 
      tx_freq_rxmode_offset = 0.0; // so tuning works.
      set1stLOFreq(tx_freq + tx_freq_rxmode_offset, 't', false);

      // tickle the transmit relay
      setTXEna(true);

      // and tell the TX unit to turn on the TX
      // This avoids the race between CTRL and TX/RX units for setup and teardown.... 
      cmd_stream->put(new Command(Command::SET, Command::TX_STATE, 3));
    }
    if(cmd->iparms[0] == 0) {
      tx_on = false; 
      // set txgain to zero
      radio->setGain(SOAPY_SDR_TX, 0, 0.0);
      radio->setGain(SOAPY_SDR_RX, 0, rx_rf_gain);      
      // tune the TX unit 1MHz away from where we want to be.
      tx_freq_rxmode_offset = rxmode_offset; // so tuning works.
      set1stLOFreq(tx_freq + tx_freq_rxmode_offset, 't', false);

      // turn off the transmit relay and the TX chain.
      // This also turns off the TX LO on a WBX module, so
      // the above tx_freq_rxmode  trick may not be necessary
      // We keep the rxmode_offset here in case other modules
      // leave the TXLO on.
      setTXEna(false);

      // and tell the RX unit to turn on the RX
      // This avoids the race between CTRL and TX/RX units for setup and teardown.... 
      cmd_stream->put(new Command(Command::SET, Command::TX_STATE, 
				  2));
    }
    // the first time we get this message, it is time to 
    // set the ready flag. 
    is_ready = true; 
    break; 

  case Command::CLOCK_SOURCE:
    break; 

  case Command::RX_ANT:
    break; 

  case Command::TX_ANT:
    break;

  case Command::TVRT_LO_CONFIG:
    break;

  case SoDa::Command::TVRT_LO_ENABLE:
    break; 

  case SoDa::Command::TVRT_LO_DISABLE:
    break;
  default:
    break; 
  }

}

void SoDa::SoapyCtrl::execGetCommand(Command * cmd)
{
  int res;

  double rf_freq, dsp_freq; 
  
  switch (cmd->target) {
  case Command::RX_FE_FREQ:
    rf_freq = radio->getFrequency(SOAPY_SDR_RX, 0, "RF");
    dsp_freq = radio->getFrequency(SOAPY_SDR_RX, 0, "BB");
    cmd_stream->put(new Command(Command::REP, Command::RX_FE_FREQ,
				rf_freq, dsp_freq)); 				
    break; 
  case Command::TX_FE_FREQ:
    rf_freq = radio->getFrequency(SOAPY_SDR_TX, 0, "RF");
    dsp_freq = radio->getFrequency(SOAPY_SDR_TX, 0, "BB");
    cmd_stream->put(new Command(Command::REP, Command::TX_FE_FREQ, 
				rf_freq, dsp_freq)); 
    break; 

  case Command::RX_SAMP_RATE:
    cmd_stream->put(new Command(Command::REP, Command::RX_SAMP_RATE, 
				radio->getSampleRate(SOAPY_SDR_RX, 0))); 
    break; 
  case Command::TX_SAMP_RATE:
    cmd_stream->put(new Command(Command::REP, Command::TX_SAMP_RATE,
				radio->getSampleRate(SOAPY_SDR_TX, 0)));
    break;

  case Command::CLOCK_SOURCE:
    res = 0;
       
    cmd_stream->put(new Command(Command::REP, Command::CLOCK_SOURCE,
				res));
    break;

  case Command::HWMB_REP:
    cmd_stream->put(new Command(Command::REP, Command::HWMB_REP,
				(boost::format("%s\t%6.1f to %6.1f MHz")
				 % model_name
				 % (rx_freq_range.minimum() * 1e-6)
				 % (rx_freq_range.maximum() * 1e-6)).str()));
    break; 
  default:
    break; 
  }
}

void SoDa::SoapyCtrl::execRepCommand(Command * cmd)
{
  switch (cmd->target) {
  default:
    break; 
  }
}

void SoDa::SoapyCtrl::set1stLOFreq(double freq, int sel, bool set_if_freq)
{
  // select "r" for rx and "t" for tx.
  // We only want to tune for one band :: 2m 144 to 148.
  // well... not really... I'd like to tune to 432 as well.
  // well.... this really should be made to work for all freqs.
  // 
  
  double target_rx_freq = freq;
 
  if(sel == SOAPY_SDR_RX) {
    // we round the target frequency to a point that puts the
    // baseband between 150 and 250 KHz below the requested
    // frequency. and an even 100kHz multiple.
    target_rx_freq = 100e3 * floor(freq / 100e3);
    debugMsg(boost::format("RX freq = %lf 1st target = %lf\n") % freq % target_rx_freq);
    while((freq - target_rx_freq) < 100e3) {
      target_rx_freq -= 100.0e3;
      debugMsg(boost::format("\tRX freq = %lf new target = %lf\n") % freq % target_rx_freq);
    }

    // just use the vanilla tuning....
    radio->setFrequency(SOAPY_SDR_RX, 0, target_rx_freq);
    last_rx_tune_freq = radio->getFrequency(SOAPY_SDR_RX, 0);
    debugMsg(boost::format("RX RF freq = %lf   RX BB freq = %lf\n")
	     % radio->getFrequency(SOAPY_SDR_RX, 0, "RF") % radio->getFrequency(SOAPY_SDR_RX, 0, "BB"));
    radio->setIQBalance(SOAPY_SDR_RX, 0, std::complex<double>(0.989624, 0.000244215));

    std::cerr << "Temporary fix for DC OFFSET\n";     
    radio->setIQBalance(SOAPY_SDR_TX, 0, std::complex<double>(0.999563,0.000293756));
    radio->setDCOffset(SOAPY_SDR_TX, 0, std::complex<double>(-0.6, 0.1));
    
    std::complex<double> iqb = radio->getIQBalance(SOAPY_SDR_RX, 0); 
    std::cerr << boost::format("Freq = %g MHz IQB [%g %g]\n")
      % target_rx_freq % iqb.real() % iqb.imag();
  }
  else {
    // On the transmit side, we're using a minimal IF rate and
    // using the full range of the tuning hardware.

    // If the transmitter is off, we retune anyway to park the
    // transmit LO as far away as possible.   This is especially 
    // important for the UBX.

    radio->setFrequency(SOAPY_SDR_TX, 0, freq);
    debugMsg(boost::format("TX freq = %lf \n") % freq);
    last_tx_tune_freq = radio->getFrequency(SOAPY_SDR_TX, 0);

    std::cerr << "Temporary fix for DC OFFSET\n";     
    radio->setIQBalance(SOAPY_SDR_TX, 0, std::complex<double>(0.999563,0.000293756));
    radio->setDCOffset(SOAPY_SDR_TX, 0, std::complex<double>(-0.6, 0.1));
    
  }

  // If we are setting the RX mode, then we need to send
  // a message to the USRPRX to tell it what its IF freq should be.
  if((sel == SOAPY_SDR_RX) && set_if_freq) {
    cmd_stream->put(new Command(Command::SET, Command::RX_LO3_FREQ,
				freq - target_rx_freq)); 
  }
}
