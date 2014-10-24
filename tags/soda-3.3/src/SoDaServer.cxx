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
 * - the GUI program, called SoDaRadio implemented in SoDaRadio_Top
 *
 * SoDaServer and SoDaRadio are connected to the USRP and host OS sound
 * system as shown here: 
 * @image html SoDa_Radio_System.svg
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
 * The image below shows the thread objects that make up the SoDa
 * SDR radio, and the message streams that link them. 
 *
 * @image html SoDa_Radio_Toplevel.svg "The SoDa Radio Components"
 *
 * @li SoDa::USRPCtrl executes all control and status functions on the USRP
 * @li SoDa::USRPRX manages the inbound IF signal stream from the USRP receive chain
 * @li SoDa::USRPTX manages the outbound IF signal stream to the USRP transmit chain
 * @li SoDa::AudioRX demodulates an incoming RX intermediate frequency signal
 * @li SoDa::AudioTX converts incoming audio into the appropriately modulated TX intermediate-frequency signal.
 * @li SoDa::CWTX converts text strings received from the UI thread into amplitude envelopes (keying envelopes)
 * for the USRPTX process. 
 * @li SoDa::UI waits for requests and CW text on the UDP socket from the GUI, and forwards status and
 *     spectrum plots back to the GUI. 
 * @li SoDa::GPS_TSIPmon monitors a serial connection to a Trimble Thunderbolt GPS receiver.
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
#include "USRPTX.hxx"
#include "AudioRX.hxx"
#include "AudioTX.hxx"
#include "CWTX.hxx"
#include "UI.hxx"
#include "GPS_TSIPmon.hxx"
#include "AudioPA.hxx"
#include "AudioALSA.hxx"
#include "Command.hxx"
#include "Debug.hxx"

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

  SoDa::Debug d(params.getDebugLevel(), "SoDaServer");
  d.setDefaultLevel(params.getDebugLevel());
  
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

  /// doWork creates the USRP Control, RX Streamer, and TX Streamer threads
  /// @see SoDa::USRPCtrl @see SoDa::USRPRX @see SoDa::USRPTX
  SoDa::USRPCtrl ctrl(&params, &cmd_stream);
  SoDa::USRPRX rx(&params, ctrl.getUSRP(), &rx_stream, &if_stream, &cmd_stream); 
  SoDa::USRPTX tx(&params, ctrl.getUSRP(), &tx_stream, &cw_env_stream, &cmd_stream);
  

  /// doWork creates the audio server on the host machine.
  /// choices include a PortAudio interface and an ALSA interface.
  /// These are subclasses of the more generic SoDa::AudioIfc class
  SoDa::AudioALSA audio_ifc(params.getAudioSampleRate(),
			    SoDa::AudioIfc::FLOAT,
			    params.getAFBufferSize()); 
  /// doWork creates the audio RX and audio TX unit threads
  /// These are also responsible for implementing IF tuning and modulation. 
  /// @see SoDa::AudioRX @see SoDa::AudioTX
  SoDa::AudioRX arx(&params, &rx_stream, &cmd_stream, &audio_ifc);
  SoDa::AudioTX atx(&params, &tx_stream, &cmd_stream, &audio_ifc);


  
  /// doWork creates the morse code (CW) tx handler thread @see SoDa::CWTX
  SoDa::CWTX cwtx(&params, &cwtxt_stream, &cw_env_stream, &cmd_stream); 
  
  /// doWork creates the user interface (UI) thread @see SoDa::UI
  SoDa::UI ui(&params, &cwtxt_stream, &rx_stream, &if_stream, &cmd_stream, &gps_stream);

  SoDa::GPS_TSIPmon gps(&params, &gps_stream); 

  d.debugMsg("Created units.");
  
  // Now start each of the activities -- they may or may not
  // implement the "start" method -- not all objects need to be threads.

  d.debugMsg("Starting UI");
  // start the UI -- configure stuff and all that. 
  ui.start();
  d.debugMsg("Starting radio units");
  // start command consumers first.
  ctrl.start();
  rx.start();
  tx.start();
  arx.start();
  atx.start();
  cwtx.start();

  // now the gps...
  d.debugMsg("Starting gps");
  gps.start();

  // wait for the user interface to tell us that it is time to quit.
  ui.join();
  ctrl.join();
  rx.join();
  tx.join();
  arx.join();
  atx.join();
  cwtx.join();
  gps.join();
  d.debugMsg("Exit");
  
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
