#pragma once
/*
Copyright (c) 2012,2013,2014, 2026 Matthew H. Reilly (kb1vc)
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
#include "SoDaBase.hxx"
#include "Params.hxx"
#include "RadioTX.hxx"

#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/stream.hpp>

#include <SoDa/MailBox.hxx>
#include <queue>
#include <memory>

namespace SoDa {
  class USRPTX;
  typedef std::shared_ptr<USRPTX> USRPTXPtr;
  
  /**
   * The Transmit RF Path
   *
   * @image html SoDa_Radio_TX_Signal_Path.svg
   *
   * In SSB/AM/FM modes, the USRPTX unit accepts an I/Q audio
   * stream from the BaseBandTX unit and forwards it to the USRP.
   * In CW mode, the USRPTX unit impresses a CW envelope (received
   * from the CW unit) onto a carrier and passes this to the USRP. 
   *
   */
  class USRPTX : public RadioTX {
  protected:
    /**
     * @brief Constructor for RF Transmit/modulator process
     *
     * @param params block describing intial setup of the radio
     * @param _usrp libuhd handle for the USRP radio
     *
     */
    USRPTX(ParamsPtr params, uhd::usrp::multi_usrp::sptr _usrp);

  public:
    static USRPTXPtr make(ParamsPtr params, uhd::usrp::multi_usrp::sptr _usrp) {
      auto ret = std::shared_ptr<USRPTX>(new USRPTX(params, _usrp));
      ret->self = ret;
      ret->registerThread(ret);
      return ret;
    }

    /**
     * @brief most transmit chains will require multiple stages to enable the
     * transmitter.  -- switch the antenna, turn on the power amplifier, start
     * the modulator.  Actual control of the antenna is left to the control unit.
     * The transmitter need only provide a "transmitEnable" function that will
     * be called after the antenna switch has flipped. 
     *
     * @param tx_on if true, enable the transmit chain. Disable the transmitter
     * if false.
     *
     * @return true if the unit was in Trasnmit mode, false otherwise. 
     */ 
    bool transmitSwitch(bool tx_on);

    /**
     *
     * Some initialization must occurr after all the components are created.
     * Units may optionally implement this method to do that. 
     *
     */
    void init() { }; 

    /**
     * @brief The TX hardware unit doesn't do much at all.  Get bits, Put bits.
     *
     * @param buf a shared pointer to a buffer of complex samples. These may be
     * enqueued or sent directly to the hardware.  But @c put must not block.
     *
     * @return true if the buffer was sent directly to the hardware.
     */
    bool put(CBufPtr buf);

    /**
     * @brief used for exception processing and such. 
     * @return a pointer to myself. 
     */ 
    RadioTXPtr getSelfPtr() { return self.lock(); }
    
  private:
    uhd::usrp::multi_usrp::sptr usrp; ///< the radio.
    
    uhd::stream_args_t * stream_args;
    uhd::tx_streamer::sptr tx_bits; ///< USRP (UHD) transmit stream handle
    uhd::tx_metadata_t tx_md; ///< metadata describing USRP transmit buffer

    std::weak_ptr<USRPTX> self;
  }; 

}
