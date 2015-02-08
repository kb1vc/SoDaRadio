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

/**
 * @file SoDaSNAServer.cxx
 *
 * @brief The MAIN process that creates and supervises all the threads that make
 * up the SoDa Scalar Network Analyzer.
 *
 * @author Matt Reilly (kb1vc)
 */
/**
 * @mainpage SoDaSNA: Classes and a Server process to build a Linux USRP Scalar Network Analyzer
 *
 *
 * @li SoDa::USRPCtrl executes all control and status functions on the USRP
 * @li SoDa::USRPSNARX manages the inbound IF signal stream from the USRP receive chain
 * @li SoDa::USRPSNATX manages the outbound IF signal stream to the USRP transmit chain
 * @li SoDa::UI waits for requests and CW text on the UDP socket from the GUI, and forwards status and
 * The SoDa receiver architecture is a 3 stage heterodyne design.  
 * The first two IF conversions are performed within the USRP SDR platform.
 * The final stage of conversion is completed in the USRPSNARX module.
 * @see SoDa::USRPSNARX
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
 *
 * 
 * @section SNAProcess The SNA Control and Signal Flow
 *
 * The GUI sends a start command
 * SNA_SCAN_START <start-freq> <end-freq> <freq-step> <time-per-step> 
 *
 * This passes to the UI unit that passes the SCAN_START command onto 
 * the Ctrl unit which sets the TX and RX frequencies to an even 
 * FS MHz step that is at below <start-freq>  .
 * FS is 3 * sample-rate / 4. The initial FE freq is 3/8 *sample-rate 
 * above <start-freq>.
 *
 * On receiving a SCAN_INIT <start-freq> command, 
 * The TX Unit then transmits a signal at <start-freq> by imposing a 
 * -nkHz tone on the carrier via a USB modulator. It then puts a 
 * SNA_TX_CURFREQ <current-freq> command on the command bus.
 *
 * The RX unit waits for the SNA_TX_CURFREQ and demodulates the incoming
 * stream waiting for the incoming frequency -- it beats <current-freq>
 * against the stream, passes it through an LPF, and measures the amplitude. 
 * When the amplitude is stable, the RX unit sends SNA_RX_SENSE <current-freq> <amplitude>.
 *
 * When the TX unit gets SNA_RX_SENSE, it bumps the tone frequency by <freq-step>
 * and if <freq-step> is less than sample-rate / 4, it sends SAN_TX_CURFREQ. 
 * When <freq-step> gets to sample-rate / 4, it sends a SCAN_BUMP CMD
 *
 * The Ctrl gets the SCAN_BUMP command and sets the TX and RX oscillators
 * to current_fe_freq + FS.  Then it sends a SCAN_BUMP REPORT to RX and TX. 
 *
 * As a check: let's assume it takes 50mS to reset the FE oscillator. 
 * Then if the sample rate is 10MS/s we reset the FE in 7.5 MHz steps. 
 * A 1GHz wide sweep would take 133 steps.  This will take 6.7 seconds.
 * This is a bit long, so we might instead step the FE analog oscillator
 * in 22.5 MHz steps and move the DDC oscillator in 7.5 MHz steps.  
 * 
 */
#include <uhd/usrp/multi_usrp.hpp>

#include "SoDaBase.hxx"
#include "MultiMBox.hxx"

// the radio parts. 
#include "Params.hxx"
#include "USRPSNACtrl.hxx"
#include "USRPSNARX.hxx"
#include "USRPSNATX.hxx"
#include "UI.hxx"
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

  SoDa::Debug d(params.getDebugLevel(), "SoDaSNA");
  d.setDefaultLevel(params.getDebugLevel());
  
  // These are the mailboxes that connect
  // the various widgets
  // the rx and tx streams are vectors of complex floats.
  // we don't declare the extent here, as it will be set
  // by a negotiation.  
  SoDa::CmdMBox cmd_stream(false);

  /// doWork creates the USRP Control, RX Streamer, and TX Streamer threads
  /// @see SoDa::USRPCtrl @see SoDa::USRPRX @see SoDa::USRPTX
  SoDa::USRPSNACtrl ctrl(&params, &cmd_stream);
  SoDa::USRPSNARX rx(&params, ctrl.getUSRP(), &cmd_stream); 
  SoDa::USRPSNATX tx(&params, ctrl.getUSRP(), &cmd_stream);
  
  /// doWork creates the user interface (UI) thread @see SoDa::UI
  SoDa::UI ui(&params, &cmd_stream);

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

  // wait for the user interface to tell us that it is time to quit.
  ui.join();
  ctrl.join();
  rx.join();
  tx.join();
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
    std::cerr << "Exception caught at SoDa SNA main: " << std::endl;
    std::cerr << "\t" << exc->toString() << std::endl; 
  }
}
