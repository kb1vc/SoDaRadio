#pragma once
/*
Copyright (c) 2026 Matthew H. Reilly (kb1vc)
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
 *  @file USRPRadio.hxx
 *
 * @brief Create the Control, RX, and TX threads for the
 * USRP radio hardware.
 *
 *  @author M. H. Reilly (kb1vc)
 *  @date   May 2026
 */

#include "Radio.hxx"
#include "Params.hxx"
#include "USRPCtrl.hxx"
#include "USRPRX.hxx"
#include "USRPTX.hxx"

#include <memory>

namespace SoDa {

  class USRPRadio;
  typedef std::shared_ptr<USRPRadio> USRPRadioPtr;
  
  class USRPRadio : public Radio {
  protected:
    USRPRadio(Params * params);

  public:
    /**
     * @brief Create a radio object. 
     *
     * @param params list of initial settings and such. 
     */ 
    static RadioPtr make(Params * params) {
      return std::shared_ptr<Radio>(new USRPRadio(params)); 
    }
    
    /**
     * @brief start the control, rx, tx, and any other hardware
     * specific processes. Setup all state.
     *
     * We don't actually do anything here. 
     */ 
    void start() { }

    /**
     * @brief shutdown all threads and clean up any state
     *
     * We don't do anything here
     */
    void stop() { }

  private:
    USRPCtrlPtr ctrl;
    USRPRXPtr rx;
    USRPTXPtr tx; 
  };
}



