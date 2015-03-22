/*
Copyright (c) 2015 Matthew H. Reilly (kb1vc)
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

#ifndef TransLO_HDR
#define TransLO_HDR
#include "SoDaBase.hxx"
#include "MultiMBox.hxx"
#include "Command.hxx"
#include "Params.hxx"
#include "QuadratureOscillator.hxx"
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/stream.hpp>

namespace SoDa {
  /**
   * The Transverter LO object -- used in the B210 10GHz rig
   *
   * Originates a continuous tone on TX2 when activated
   */
  class TransLO : public SoDaThread {
  public:
    /**
     * @brief Constructor for Transverter LO process
     *
     * @param params block describing intial setup of the radio
     * @param usrp libuhd handle for the USRP radio
     * @param _cmd_stream command stream
     *
     */
    TransLO(Params * params, uhd::usrp::multi_usrp::sptr _usrp,
	   CmdMBox * _cmd_stream);
    /**
     * @brief TransLO run loop: handle commands, and modulate the tx carrier
     */
    void run(); 

  private:

    uhd::usrp::multi_usrp::sptr usrp; ///< the radio.

    /**
     * @brief setup transverter LO streamer.
     */
    void getLOStreamer();

    /**
     * @brief handle SET commands from the command channel
     * @param cmd the incoming command
     */
    void execSetCommand(Command * cmd); 
    /**
     * @brief handle Report commands from the command channel
     * @param cmd the incoming command
     */
    void execRepCommand(Command * cmd);     

    unsigned int cmd_subs; ///< subscription handle for command stream
    CmdMBox * cmd_stream; ///< command stream
    
    double tx_sample_rate; ///< sample rate for buffer going to USRP (UHD)
    unsigned int tx_buffer_size; ///< size of buffer going to USRP
    
    uhd::stream_args_t * stream_args;
    uhd::tx_streamer::sptr tx_bits; ///< USRP (UHD) transmit stream handle
    uhd::tx_metadata_t md; ///< metadata describing USRP transmit buffer

    uhd::rx_streamer::sptr rx_dummy_bits; ///< USRP (UHD) rx stream handle to workaround goofy bug in libuhd "2 RX 1 TX and 1 RX 2 TX configurations not possible"    

    // transverter local oscillator support
    bool LO_enabled; ///< if true, we're in local transverter mode
    bool LO_configured; ///< if true, the LO has had its gain/freq set.

    std::complex<float> * const_buf; ///< envelope for constant carrier.
    std::complex<float> * zero_buf; ///< envelope for zero output carrier.
  }; 
}


#endif
