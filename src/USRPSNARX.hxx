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

#ifndef USRPSNARX_HDR
#define USRPSNARX_HDR
#include "SoDaBase.hxx"
#include "MultiMBox.hxx"
#include "Command.hxx"
#include "Params.hxx"
#include "UI.hxx"
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/stream.hpp>

namespace SoDa {
  /**
   * The Receive RF Path for the Scalar Network Analyzer.
   *
   * @image html SoDa_Radio_RX_Signal_Path.svg
   */
  class USRPSNARX : public SoDaThread {
  public:
    /**
     * The constructor
     *
     * @param params a pointer to a params object that will tell us
     *        about sample rates and other configuration details.
     * @param usrp a pointer to the UHD USRP object that we are streaming data from.
     * @param _cmd_stream data mailbox used to carry command, query, and report messages
     */
    USRPSNARX(Params * params, uhd::usrp::multi_usrp::sptr usrp,
	   CmdMBox * _cmd_stream);

    /**
     * USRPRX is a thread -- this is its run loop. 
     */
    void run();
    
  private:   
    void execCommand(Command * cmd); 
    void execGetCommand(Command * cmd); 
    void execSetCommand(Command * cmd); 
    void execRepCommand(Command * cmd);

    void startStream();
    void stopStream(); 


    CmdMBox * cmd_stream;
    unsigned int cmd_subs; 

    // state for the USRP widget
    uhd::rx_streamer::sptr rx_bits;
    uhd::usrp::multi_usrp::sptr usrp; 

    double rx_sample_rate;
    unsigned int rx_buffer_size;

    // collection stuff
    double slot_start_time; //< time for start-of-measurement slot
    double slot_end_time; //< time at end-of-slot
    double test_freq;     //< the frequency we're testing now
    
    std::complex<float> * sample_buf; //< we collect input samples here. 
    int ring_elements;  //< How many mag measurements do we save? 
    int ring_count;     //< pointer to current ring element
    static const int SNARX_MAX_RING_SAMPS = 10;
    double sample_magsq[SNARX_MAX_RING_SAMPS]; //< the last <ring_elements>  
    bool do_collection; //< when true, we accumulate statistics. 

    //debug hooks
    int outf[2];
    int scount; 
  }; 
}


#endif
