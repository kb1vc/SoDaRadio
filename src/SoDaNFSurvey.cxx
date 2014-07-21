/*
  Copyright (c) 2014, Matthew H. Reilly (kb1vc)
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

/**
 * @file SoDaNFSurvey.cxx
 *
 * @brief The MAIN process that creates and supervises all the threads that make
 * up the SoDa Noise Figure Survey application
 *
 * @author Matt Reilly (kb1vc)
 */
/**
 * @mainpage SoDaBench: Classes and a Server process to build a Linux USRP RF testbench
 *
 *
 * The SoDa Noise Figure Survey is a multi-threaded application
 * developed for the Linux operating system and the Ettus Radio USRP family
 * of SDR platforms. 
 *
 * SoDa was written and is maintained by Matt Reilly, kb1vc.
 *
 * @section structure Structure of the Radio
 *
 *
 * - the SDR controller program, called SoDaNFSurvey
 *
 *
 * The GUI is built on the wxWidgets GUI toolset, and communicates
 * with the SDR control program via a Unix Domain socket connection.
 * The GUI and the SDR controller run on a single Linux host.
 *
 * The SDR signal processing and control functions are executed within
 * multiple threads of the SoDa application.  The threads communicate via
 * a simple mailbox-in-shared-memory communications scheme where each
 * thread can "subscribe" to one or more message streams, and place messages
 * into any message stream. (See SoDa::MultiMBox and SoDa::MBoxMessage)
 *
 * 
 *
 * The SoDa receiver architecture is a 3 stage heterodyne design.  
 * The first two IF conversions are performed within the USRP SDR platform.
 * The final stage of conversion is completed in the USRPRX module.
 * @see SoDa::USRPRX
 *
 * @image html SoDa_Radio_RX_Signal_Path.svg
 *
 * The reason for the three stage conversion scheme is threefold:
 * -# The first stage (Front End) LO is configured to operate in Integer-N
 *    synthesis mode. This reduces spurious "birdies" caused by harmonics of the
 *    reference oscillator divided by the fractional N denominator. 
 * -# The second stage LO is tuned to place the signal of interest at least
 *    50 kHz away from both the first and second local oscillators.  This
 *    ensures that the DC offset "spike" in the second IF stage is well away
 *    from the frequency of interest, and is not displayed in a waterfall or
 *    periodogram display.
 * -# by placing the second IF well away from the target frequency, we can
 *    put the target signal well off the skirts of the phase noise curves for
 *    both the first and second LOs. The third LO is implemented as a full
 *    32 bit floating point quadrature oscillator, so the quantization noise
 *    is comparatively low. 
 *
 * @image html SoDa_Radio_TX_Signal_Path.svg
 */
#include <uhd/usrp/multi_usrp.hpp>

#include "SoDaBase.hxx"
#include "MultiMBox.hxx"

// the radio parts. 
#include "Params.hxx"
#include "USRPCtrl.hxx"
#include "USRPRX.hxx"
#include "AudioRX.hxx"
#include "AudioIfc.hxx"
#include "USRPTX.hxx"
#include "NFThermometer.hxx"
#include "Command.hxx"

namespace SoDa {
  class NFControl : public SoDaThread {
  public:
    NFControl(Params * params, CmdMBox * _cmd_stream) :
      SoDaThread("Noise Figure Survey Control")
    {
      cmd_stream = _cmd_stream; 
      cmd_subs = cmd_stream->subscribe();
      exitflag = false; 
    }

    void initRadio() {
      usleep(100000);

      cmd_stream->put(new SoDa::Command(Command::SET, Command::RX_FE_FREQ, 144.2e6));
      cmd_stream->put(new SoDa::Command(Command::SET, Command::TX_FE_FREQ, 144.2e6));
      cmd_stream->put(new SoDa::Command(Command::SET, Command::RX_LO3_FREQ, 100e3));
      // 2KHz? 
      // setup bandwidth
      cmd_stream->put(new SoDa::Command(Command::SET, Command::RX_AF_FILTER, (int) SoDa::Command::BW_2000));
      // setup mode for AM
      cmd_stream->put(new SoDa::Command(Command::SET, Command::RX_MODE, (int) SoDa::Command::AM));
      cmd_stream->put(new SoDa::Command(Command::SET, Command::TX_STATE, 0));

      cmd_stream->put(new SoDa::Command(Command::SET, Command::RX_AF_GAIN, 40.0));
      cmd_stream->put(new SoDa::Command(Command::SET, Command::RX_RF_GAIN, 100000.0));
    }

    
    void run() {
      // first do the initial setup.
      initRadio();

      therm_buf[0] = '\000';
      
      // now loop
      double freq;
      while(1) {
	for(freq = 51.3176e6; freq < 6000.0e6; freq += 100.0e6) {
	  if(exitflag) return; 
	  // tune to the new freq
	  tune(freq);
	  // wait a second
	  for(int i = 0; i < 100; i++) {
	    pollCommands(); 
	    usleep(10000);	  
	  }
	  // clear the power count
	  cmd_stream->put(new SoDa::Command(Command::SET, Command::CLEAR_ENV_POWER));

	  // ask for the temperature and for the last envelope power
	  waiting_for_temp = true;
	  waiting_for_power = true;
	  while(waiting_for_temp || waiting_for_power) {
	    pollCommands();
	    usleep(10000); 
	  }

	  // now dump the report
	  std::cout << boost::format("%g %s %g\n") % freq % therm_buf % env_power;
	  std::cout.flush();
	}
      }
    }

  private:
    void pollCommands() {
      Command * cmd; 
      while((cmd = cmd_stream->get(cmd_subs)) != NULL) {
	execCommand(cmd);
	exitflag |= (cmd->target == Command::STOP); 
	cmd_stream->free(cmd);
      }
    }

    void tune(double f) {
      cmd_stream->put(new SoDa::Command(Command::SET, Command::RX_RETUNE_FREQ, f));
    }

    void execRepCommand(SoDa::Command * cmd) {
      switch (cmd->target) {
      case SoDa::Command::NF_THERM:
	strncpy(therm_buf, cmd->sparm, 64);
	waiting_for_temp = false; 
	break;
      case SoDa::Command::ENV_POWER:
	env_power = cmd->dparms[0]; 
	waiting_for_power = false; 
	break; 
      }
    }
    
    CmdMBox * cmd_stream; ///< mailbox producing command stream from user
    unsigned int cmd_subs; ///< mailbox subscription ID for command stream

    bool exitflag; 

    bool waiting_for_temp;
    bool waiting_for_power;

    char therm_buf[64];
    float env_power; 
  }; 
}

/// do the work of creating the SoDa threads
/// @param argc number of command line arguments
/// @param argv command line arguments
int doWork(int argc, char * argv[])
{
  /// create the components of the radio

  /// the command line parameter list is used to
  /// create a param object that holds configuration
  /// information from the command line and from
  /// the stored configuration files.
  /// @see SoDa::Params
  SoDa::Params params(argc, argv);

  params.setRXRate(1.0e6);
  params.setRFBufferSize(10000);
  params.setTXRate(1.0e6);
  
  // These are the mailboxes that connect
  // the various widgets
  // the rx and tx streams are vectors of complex floats.
  // we don't declare the extent here, as it will be set
  // by a negotiation.  
  SoDa::DatMBox rx_stream, if_stream, cw_env_stream;
  SoDa::CmdMBox cmd_stream(false);

  /// doWork creates the USRP Control, RX Streamer, and TX Streamer threads
  /// @see SoDa::USRPCtrl @see SoDa::USRPRX @see SoDa::USRPTX
  SoDa::USRPCtrl ctrl(&params, &cmd_stream);
  SoDa::USRPRX rx(&params, ctrl.getUSRP(), &rx_stream, &if_stream, &cmd_stream); 
  SoDa::AudioNull audio_ifc(params.getAudioSampleRate(), SoDa::AudioIfc::FLOAT, params.getAFBufferSize());
  SoDa::AudioRX arx(&params, &rx_stream, &cmd_stream, &audio_ifc);
  SoDa::NFThermometer therm(&params, &cmd_stream);
  SoDa::NFControl nf(&params, &cmd_stream);
  
  // Now start each of the activities -- they may or may not
  // implement the "start" method -- not all objects need to be threads.

 
  // start command consumers first.
  ctrl.start();
  rx.start();
  arx.start();
  therm.start(); 
  // now do the survey.
  nf.start();
  
  // wait for the user interface to tell us that it is time to quit.
  ctrl.join();
  rx.join();
  arx.join();

  std::cerr << "waiting to join nf control thread.";
  nf.join(); 
  
  // when we get here, we are done... (UI should not return until it gets an "exit/quit" command.)

  return 0; 
}

/// main entrypoint
///
/// Start the SoDa USRP radio server threads
/// @param argc number of command line arguments
/// @param argv command line arguments @see doWork
int main(int argc, char * argv[])
{
  try {
    return doWork(argc, argv); 
  }
  catch (SoDa::SoDaException * exc) {
    std::cerr << "Exception caught at SoDa main: " << std::endl;
    std::cerr << "\t" << exc->toString() << std::endl; 
  }
}
