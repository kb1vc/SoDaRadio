/*
Copyright (c) 2012,2013,2014 Matthew H. Reilly (kb1vc)
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

#ifndef USRPLO_HDR
#define USRPLO_HDR
#include "SoDaBase.hxx"
#include "MultiMBox.hxx"
#include "Command.hxx"
#include "Params.hxx"
#include "QuadratureOscillator.hxx"
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/stream.hpp>

namespace SoDa {
  /**
   * The Transverter LO unit 
   *
   * @image html SoDa_Radio_TX_Signal_Path.svg
   *
   * The B210 has two independent transmit channels.
   * We can use the second channel as a transverter LO
   * to feed into a mixer and extend the frequency range
   * covered by the base unit.
   *
   * In particular, SoDaRadio will set the second TX
   * channel to 5.000 GHz to create a mixer LO that
   * will translate 10.368 GHz down to 5.368 GHz. 
   *
   */
  class USRPLO : public SoDaThread {
  public:
    /**
     * @brief Constructor for Transverter LO process
     *
     * @param params block describing intial setup of the radio
     * @param usrp libuhd handle for the USRP radio
     * @param _cmd_stream command stream
     *
     */
    USRPLO(Params * params, uhd::usrp::multi_usrp::sptr usrp,
	   CmdMBox * _cmd_stream);
    /**
     * @brief USRPLO run loop: handle commands, and modulate the tx carrier
     */
    void run(); 
  private:
    /**
     * @brief establish stream if and only if this unit supports it.
     */
    void initLOStream(); 
    /**
     * @brief start/stop LO
     * @param tx_on if true, go into transmit mode
     */
    void enableLO(bool LO_on);

    /**
     * @brief execute GET commands from the command channel
     * @param cmd the incoming command
     */
    void execGetCommand(Command * cmd); 
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
    
    bool LO_enabled; ///< if true, we're in local transverter mode
    bool LO_configured; ///< if true, the LO has had its gain/freq set.
    bool LO_capable; ///< if true, this hardware model supports LO config

    double LO_frequency;
    double LO_gain;

    std::complex<float> * const_env; ///< envelope for dead silence

    double LO_sample_rate; ///< sample rate for buffer going to USRP (UHD)
    unsigned int LO_buffer_size; ///< size of buffer going to USRP
    
    bool waiting_to_run_dry; ///< When set, we should send out a report when we run out of CW buffer
    
    uhd::tx_streamer::sptr LO_bits; ///< USRP (UHD) transmit stream handle
    uhd::tx_metadata_t md; ///< metadata describing USRP transmit buffer

    uhd::usrp::multi_usrp::sptr usrp; 
  }; 

}


#endif
