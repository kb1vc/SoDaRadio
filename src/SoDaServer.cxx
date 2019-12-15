/*
  Copyright (c) 2012,2013,2014,2015,2016,2017 Matthew H. Reilly (kb1vc)
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
 * @file SoDaServer.cxx
 *
 * @brief The MAIN process that creates and supervises all the threads that make
 * up the SoDa SDR. 
 *
 * @author Matt Reilly (kb1vc)
 */
/**
 * @mainpage SoDaRadio: Classes and a Server process to build a Linux USRP VHF/UHF Radio
 *
 *
 * The SoDa VHF/UHF Software Defined Radio is a multi-threaded application
 * developed for the Linux operating system and the Ettus Radio USRP family
 * of SDR platforms. 
 *
 * SoDa was written and is maintained by Matt Reilly, kb1vc.
 *
 * @section structure Structure of the Radio
 *
 * The SoDa program is partitioned into two parts:
 *
 * - the SDR controller program, called SoDaServer
 * - the GUI program, called SoDaRadio implemented in qtgui/main.cpp
 *
 * SoDaServer and SoDaRadio are connected to the USRP and host OS sound
 * system as shown here: 
 * @image html SoDa_Radio_System.svg
 *
 * The GUI is built on the Qt GUI toolset, and communicates
 * with the SDR control program via a Unix Domain socket connection.
 * The GUI and the SDR controller run on a single Linux host.
 *
 * The SDR signal processing and control functions are executed within
 * multiple threads of the SoDa application.  The threads communicate via
 * a simple mailbox-in-shared-memory communications scheme where each
 * thread can "subscribe" to one or more message streams, and place messages
 * into any message stream. (See SoDa::MultiMBox and SoDa::MBoxMessage)
 *
 * The image below shows the thread objects that make up the SoDa
 * SDR radio, and the message streams that link them. 
 *
 * @image html SoDa_Radio_Toplevel.svg "The SoDa Radio Components"
 *
 * @li SoDa::USRPCtrl executes all control and status functions on the USRP
 * @li SoDa::USRPRX manages the inbound IF signal stream from the USRP receive chain
 * @li SoDa::USRPTX manages the outbound IF signal stream to the USRP transmit chain
 * @li SoDa::BaseBandRX demodulates an incoming RX intermediate frequency signal
 * @li SoDa::BaseBandTX converts incoming audio into the appropriately modulated TX intermediate-frequency signal.
 * @li SoDa::CWTX converts text strings received from the UI thread into amplitude envelopes (keying envelopes)
 * for the USRPTX process. 
 * @li SoDa::UI waits for requests and CW text on the UDP socket from the GUI, and forwards status and
 *     spectrum plots back to the GUI. 
 * @li SoDa::GPSmon monitors a connection to the gpsd server (if any)
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
// #include <uhd/usrp/multi_usrp.hpp>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <fstream>

#include "SoDaBase.hxx"
#include "SoDaThread.hxx"
#include "SoDaThreadRegistry.hxx"
#include "MultiMBox.hxx"

#include "ProcInfo.hxx"

#include <dlfcn.h>

// the radio parts. 
#include "Params.hxx" 
// For USRP devices
#if HAVE_UHD
#  include "USRPCtrl.hxx"
#  include "USRPRX.hxx"
#  include "USRPTX.hxx"
#endif

#include "BaseBandRX.hxx"
#include "BaseBandTX.hxx"
#include "CWTX.hxx"
#include "UI.hxx"
#include "GPSmon.hxx"
#include "AudioQt.hxx"
#include "IFRecorder.hxx"
#include "Command.hxx"
#include "Debug.hxx"

void createLockFile(const std::string & lock_file_name)
{
  std::ofstream lfile; 
  lfile.open(lock_file_name.c_str());
  lfile << "If this file exists, there is likely an active SoDaServer process somewhere on this machine.\n";
  lfile.close();
}

void deleteLockFile(const std::string & lock_file_name)
{
  remove(lock_file_name.c_str());
}

int loadAccessories(const std::vector<std::string> & libs, SoDa::Debug & d) {
  // are there loadable modules we want to run?
  for(auto l : libs) {
    dlopen(l.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    d.debugMsg((boost::format("Loaded shared object %s\n") % l).str());    
  }
  return 1; 
}

/// do the work of creating the SoDa threads
/// @param params command line parameter parser object
int doWork(SoDa::Params & params)
{
  /// create the components of the radio
  SoDa::Debug d(params.getDebugLevel(), "SoDaServer");
  d.setDefaultLevel(params.getDebugLevel());
  
  loadAccessories(params.getLibs(), d);
  
  // These are the mailboxes that connect
  // the various widgets
  // the rx and tx streams are vectors of complex floats.
  // we don't declare the extent here, as it will be set
  // by a negotiation.  
  SoDa::DatMBox rx_stream, tx_stream, if_stream, cw_env_stream;
  SoDa::CmdMBox cmd_stream(false);
  // create a separate gps stream to avoid "leaks" and latency problems... 
  SoDa::CmdMBox gps_stream(false);
  SoDa::CmdMBox cwtxt_stream(false);

  SoDa::Thread * ctrl;
  SoDa::Thread * rx;
  SoDa::Thread * tx;

  SoDa::MailBoxMap mailbox_map;

  mailbox_map["RX"] = &rx_stream;
  mailbox_map["TX"] = &tx_stream;
  mailbox_map["CMD"] = &cmd_stream;
  mailbox_map["CW_TXT"] = &cwtxt_stream;
  mailbox_map["CW_ENV"] = &cw_env_stream;  
  mailbox_map["GPS"] = &gps_stream;
  mailbox_map["IF"] = &if_stream;
  
  if(params.isRadioType("USRP")) {
    /// create the USRP Control, RX Streamer, and TX Streamer threads
    /// @see SoDa::USRPCtrl @see SoDa::USRPRX @see SoDa::USRPTX
    ctrl = new SoDa::USRPCtrl(&params);
    rx = new SoDa::USRPRX(&params, ((SoDa::USRPCtrl *)ctrl)->getUSRP());
    tx = new SoDa::USRPTX(&params, ((SoDa::USRPCtrl *)ctrl)->getUSRP());
  }
  else {
    std::cerr << boost::format("Radio type [%s] is not yet supported\nHit ^C to exit.\n") % params.getRadioType(); 
    exit(-1);
  }

  /// Create the audio server on the host machine.
  /// choices include a PortAudio interface and an ALSA interface.
  /// These are subclasses of the more generic SoDa::AudioIfc class
  SoDa::AudioQt audio_ifc(params.getAudioSampleRate(),
			  params.getAFBufferSize(),
			  params.getServerSocketBasename(),
			  params.getAudioPortName());
  /// Create the audio RX and audio TX unit threads
  /// These are also responsible for implementing IF tuning and modulation. 
  /// @see SoDa::BaseBandRX @see SoDa::BaseBandTX
  SoDa::BaseBandRX bbrx(&params, &audio_ifc);

  SoDa::BaseBandTX bbtx(&params, &audio_ifc);

  /// Create the morse code (CW) tx handler thread @see SoDa::CWTX
  SoDa::CWTX cwtx(&params);
    
  /// Create the user interface (UI) thread @see SoDa::UI
  SoDa::UI ui(&params);

  /// Create an IF listener process that copies the IF stream to an output file
  /// when requested.
  SoDa::IFRecorder ifrec(&params);

#if HAVE_GPSLIB    
  SoDa::GPSmon gps(&params);
#endif
  
  d.debugMsg("Created units.");

  auto registrar = SoDa::ThreadRegistry::getRegistrar();  
  // hook everyone up to the mailboxes. 
  registrar->apply([mailbox_map](SoDa::Thread * el) 
		   bool { 
		     el->subscribeToMailBoxList(mailbox_map); 
		     return true;
		   });
  
  // Now start each of the activities -- they may or may not
  // implement the "start" method -- not all objects need to be threads.

  d.debugMsg("Starting UI");
  // start the UI -- configure stuff and all that. 
  ui.start();
  d.debugMsg("Starting radio units");
  // start command consumers first.
  // start everyone
  auto ui_p = & ui; 


  registrar->apply([ui_p](SoDa::Thread * el) 
		   bool {
		     if(el != ui_p) {
		       el->start();
		     }
		     return true; 
		   });

  if(params.reportMemInfo()) {
    kb1vc::ProcInfo pi(params.getReportFileName(), "SoDaServer");

    // wait for the user interface to tell us that it is time to quit.
    // do this in a loop so that we can check on VM status. 
    int watchdog_count = 0; 
    pi.reportInfo();
    while(!ui.waitForJoin(1000)) {
      watchdog_count++; 
      // do this check every 10 seconds
      if(watchdog_count > 10) {
	pi.reportInfo(true);
	watchdog_count = 0; 
      }
    }
  }
  else {
    ui.join();
  }

  registrar->apply([ui_p](SoDa::Thread * el)   
		   bool {
		     if(el != ui_p) {
		       el->join();
		     }
		     return true; 
		   });
  
  d.debugMsg("Exit");
  
  // when we get here, we are done... (UI should not return until it gets an "exit/quit" command.)

  registrar->apply([](SoDa::Thread * el) 
		   bool { 
		     el->shutDown();
		     return true;
		   });
  
  return 0; 
}

/// main entrypoint
///
/// Start the SoDa USRP radio server threads
/// @param argc number of command line arguments
/// @param argv command line arguments @see doWork
int main(int argc, char * argv[])
{
  /// the command line parameter list is used to
  /// create a param object that holds configuration
  /// information from the command line and from
  /// the stored configuration files.
  /// @see SoDa::Params
  SoDa::Params params(argc, argv);

  /// create a lock file to signal that we're alive. 
  createLockFile(params.getLockFileName()); 

  try {
    doWork(params); 
  }
  catch (SoDa::Exception exc) {
    std::cerr << "Exception caught at SoDa main: " << std::endl;
    std::cerr << "\t" << exc.toString() << std::endl; 
  }

  deleteLockFile(params.getLockFileName());   
}
