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

#ifndef USRPSNATX_HDR
#define USRPSNATX_HDR
#include "SoDaBase.hxx"
#include "MultiMBox.hxx"
#include "Command.hxx"
#include "Params.hxx"
#include "QuadratureOscillator.hxx"
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/stream.hpp>

namespace SoDa {
  /**
   * The Scalar Network Analyzer Transmit RF Path
   * 
   * The USRPSNATX unit generates an output carrier that sweeps
   * from a start to an end frequency, with each step lasting
   * for a specific span of time. 
   *
   * The output wave is constant amplitude, but the frequency
   * steps are discrete. 
   *
   */
  class USRPSNATX : public SoDaThread {
  public:
    /**
     * @brief Constructor for RF Transmit/modulator process
     *
     * @param params block describing intial setup of the radio
     * @param usrp libuhd handle for the USRP radio
     * @param _cmd_stream command stream
     *
     */
    USRPSNATX(Params * params, uhd::usrp::multi_usrp::sptr _usrp,
	      CmdMBox * _cmd_stream);
    /**
     * @brief USRPSNATX run loop: handle commands, and modulate the tx carrier
     */
    void run(); 
  private:

    uhd::usrp::multi_usrp::sptr usrp; ///< the radio.
    
    /**
     * @brief start/stop transmit stream
     * @param tx_on if true, go into transmit mode
     */
    void transmitSwitch(bool tx_on);

    /**
     * @brief setup transmit streamer.
     */
    void getTXStreamer();
    
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
    
    bool tx_enabled; ///< if true, we're transmitting. 

    double tx_sample_rate; ///< sample rate for buffer going to USRP (UHD)
    unsigned int tx_buffer_size; ///< size of buffer going to USRP

    float cw_env_amplitude;  ///< used to set CW output envelope, constant at 0.7
    
    uhd::stream_args_t * stream_args;
    uhd::tx_streamer::sptr tx_bits; ///< USRP (UHD) transmit stream handle
    uhd::tx_metadata_t tx_md; ///< metadata describing USRP transmit buffer

    std::complex<float> * const_buf;
    std::complex<float> * zero_buf;
  }; 

}


#endif
