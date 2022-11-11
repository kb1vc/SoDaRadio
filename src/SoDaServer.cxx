/*
  Copyright (c) 2012,2013,2014,2015,2016,2017,2022 Matthew H. Reilly (kb1vc)
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
 * @li One or more user loadable plugins may connect to any or all of the streams to implement
 * special user-defined functions. IFServer presents a simple example of a plugin that subscribes 
 * to the command and the RX IF stream. 
 *
 * Audio is handled in two ways in the SoDa::AudioQt class.
 * 
 * @li Transmit audio is read from an ALSA device by the SoDaServer process
 * via the SoDa::AudioQt::recv method.  With time, this function will migrate
 * to a socket connection the Qt GUI.
 * @li Receive audio is written by the SoDa::AudioQt send method
 *  to a socket that, in the normal configuration, 
 * is connected to the Qt based GUI.  This allows for better flow control and
 * also simplifies interfacing the audio stream to external modems like "fldigi"
 * and WSJT-X. 
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
#include <unistd.h>
#include <stdio.h>
#include <sys/resource.h>

#include <fstream>
#include <SoDa/Format.hxx>

#include "SoDaBase.hxx"
#include "Thread.hxx"
#include "ThreadRegistry.hxx"
#include "Radio.hxx"
#include "Buffer.hxx"
#include "MailBoxRegistry.hxx"
#include "MailBoxTypes.hxx"
#include "RadioRegistry.hxx"
#include "NullRadio.hxx"
#include "RTLsdr.hxx"

// Include functions to dynamically link any user supplied plugins
#include <dlfcn.h>

// the radio parts. 
#include "Params.hxx" 
// For USRP devices
#if HAVE_UHD
# include "USRP.hxx"
#endif

#include "BaseBandRX.hxx"
#include "BaseBandTX.hxx"
#include "CWTX.hxx"
#include "UI.hxx"
#include "GPSmon.hxx"
#include "IFRecorder.hxx"
#include "Command.hxx"
#include "Debug.hxx"


#ifdef HAVE_ASOUND
#  include "AudioQtRXTX.hxx"
   using AudioQt = SoDa::AudioQtRXTX;
#else
#  include "AudioQtRX.hxx"
   using AudioQt = SoDa::AudioQtRX;  
#endif



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
  typedef void (*makeit_t)(const std::string &);
  for(auto l : libs) {
    auto lib = dlopen(l.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if(!lib) {
      std::cerr << SoDa::Format("Could not open library %0: error [%1]\n")
	.addS(l).addS(dlerror());
    }
    d.debugMsg(SoDa::Format("Loaded shared object %0\n").addS(l));
    makeit_t makeit = (makeit_t) dlsym(lib, "initLib");
    const char * dle = dlerror();
    if(dle != nullptr) {
      std::cerr << "Could not run initLib function for library " << l << " got error [" << dle << "]\n";
    }
    
    makeit("Hey there, from SoDaServer!\n");
  }
  return 1; 
}

/// do the work of creating the SoDa threads
/// @param params command line parameter parser object
int doWork(SoDa::Params_p params)
{
  /// create the components of the radio
  SoDa::Debug d(params->getDebugLevel(), "SoDaServer");
  d.setDefaultLevel(params->getDebugLevel());

  // register all our "built-in" radio models
  SoDa::RadioRegistry::addRadio("USRP", SoDa::USRP::makeRadio);
  SoDa::RadioRegistry::addRadio("NullRadio", SoDa::NullRadio::makeRadio);
  SoDa::RadioRegistry::addRadio("RTLsdr", SoDa::RTLsdr::makeRadio);    

  loadAccessories(params->getLibs(), d);
  
  // These are the mailboxes that connect
  // the various widgets
  // the rx and tx streams are vectors of complex floats.
  // we don't declare the extent here, as it will be set
  // by a negotiation.  
  SoDa::CFMBoxPtr rx_stream = SoDa::registerMailBox<SoDa::CFMBox>("RX");
  SoDa::CFMBoxPtr tx_stream = SoDa::registerMailBox<SoDa::CFMBox>("TX");
  SoDa::CFMBoxPtr if_stream = SoDa::registerMailBox<SoDa::CFMBox>("IF");
  SoDa::FMBoxPtr cw_env_stream = SoDa::registerMailBox<SoDa::FMBox>("CW_ENV");
  SoDa::MsgMBoxPtr cmd_stream = SoDa::registerMailBox<SoDa::MsgMBox>("CMD");
  // create a separate gps stream to avoid "leaks" and latency problems... 
  SoDa::MsgMBoxPtr gps_stream = SoDa::registerMailBox<SoDa::MsgMBox>("GPS");
  SoDa::MsgMBoxPtr cwtxt_stream = SoDa::registerMailBox<SoDa::MsgMBox>("CW_TXT");

  SoDa::Radio * radio; 
  radio =   SoDa::RadioRegistry::make("USRP", params);

  /// Though it is rather sad, it looks like the cleanest approach is to
  /// create the resamplers here. The resampler determines the size of both the
  /// TX/RX RF buffers and the TX/RX baseband buffers.  This is largely driven by
  /// the sample rates supported by the radio.
  std::cerr << SoDa::Format("Radio sample rates RX %0 TX %1\n")
    .addF(radio->getRXSampleRate())
    .addF(radio->getTXSampleRate());

  /// The radio must supply a TX and RX sample rate
  SoDa::ReSampler rx_resampler(radio->getRXSampleRate(), 
			 params->getAudioSampleRate(), 
			 params->getSampleChunkDuration());
  SoDa::ReSampler tx_resampler(params->getAudioSampleRate(),
			 radio->getTXSampleRate(), 
			 params->getSampleChunkDuration());

  params->setRXRate(radio->getRXSampleRate());
  params->setTXRate(radio->getTXSampleRate());
  
  params->setRXAFBufferSize(rx_resampler.getOutputBufferSize());
  params->setTXAFBufferSize(tx_resampler.getInputBufferSize());
  params->setRXRFBufferSize(rx_resampler.getInputBufferSize());
  params->setTXRFBufferSize(tx_resampler.getOutputBufferSize());

  
  // Initialize the radio. 
  radio->init();
  
  /// Create the audio server on the host machine.
  /// Audio is either via Qt for RX and ALSA for TX.
  /// If ALSA is not present, the server will be RX only.
  /// These are subclasses of the more generic SoDa::AudioIfc class
  
  AudioQt audio_ifc(params);
  
  /// Create the audio RX and audio TX unit threads
  /// These are also responsible for implementing IF tuning and modulation. 
  /// @see SoDa::BaseBandRX @see SoDa::BaseBandTX
						    
  SoDa::BaseBandRX bbrx(params, &rx_resampler, &audio_ifc);

  SoDa::BaseBandTX bbtx(params, &tx_resampler, &audio_ifc);

  /// Create the morse code (CW) tx handler thread @see SoDa::CWTX
  SoDa::CWTX cwtx(params);
    
  /// Create the user interface (UI) thread @see SoDa::UI
  SoDa::UI ui(params);

  /// Create an IF listener process that copies the IF stream to an output file
  /// when requested.
  SoDa::IFRecorder ifrec(params);

#if HAVE_GPSLIB    
  SoDa::GPSmon gps(params);
#endif
  
  d.debugMsg("Created units.");

  // get the thing that knows which threads are part of the radio.
  // (Some may be loaded dynamically with the "--load" command line parameter.
  auto registrar = SoDa::ThreadRegistry::getRegistrar();  

  // hook everyone up to the mailboxes. 
  registrar->subscribeThreads();
  
  // Now start each of the activities -- they may or may not
  // implement the "start" method -- not all objects need to be threads.

  d.debugMsg("Starting Threads");
  
  // start all the threads
  registrar->startThreads();

  registrar->joinThreads();

  // once everyone has joined, we're due to stop
  registrar->shutDownThreads();  
  
  // when we get here, we are done... (UI should not return until it gets an "exit/quit" command.)
  radio->cleanUp();
  
  d.debugMsg("Exit");
  
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
  SoDa::Params_p params = std::make_shared<SoDa::Params>(argc, argv);

  /// create a lock file to signal that we're alive. 
  createLockFile(params->getLockFileName()); 

  try {
    doWork(params); 
  }
  catch (SoDa::Radio::Exception exc) {
    std::cerr << "Exception caught at SoDa main: " << std::endl;
    std::cerr << "\t" << exc.what() << std::endl; 
  }

  deleteLockFile(params->getLockFileName());   
}
