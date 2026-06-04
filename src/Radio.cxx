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
 *  @file Radio.hxx
 *
 * @brief Base class for all hardware modules (USRP, Pluto, RTLSDR, etc)
 * that starts all required processes (e.g. USRPCtrl, USRPRX, USRPTX) that
 * are particular to the hardware.
 *
 * This class encapsulates the entirety of the hardware specific code for
 * any radio. All interactions with the radio after this object is constructed
 * are via the mailboxes. 
 * 
 *  @author M. H. Reilly (kb1vc)
 *  @date   May 2026
 */

#include "Params.hxx"
#include <memory>

namespace SoDa {

  class Radio;
  typedef std::shared_ptr<Radio> RadioPtr;
  
  class Radio {
  protected:
    /**
     * @brief create the base radio. 
     */
    Radio(Params * params) {
      // do nothing for now
    }

    /// implement the subscription method
    void subscribeToMailBoxes(const std::vector<MailBoxBasePtr> & mailboxes) {
      ctrl->subscribeToMailBoxes(mailboxes);
      rx->subscribeToMailBoxes(mailboxes);
      tx->subscribeToMailBoxes(mailboxes);      
    }
    
    
    /**
     * @brief start the control, rx, tx, and any other hardware
     * specific processes. Setup all state. 
     */ 
    virtual void start() = 0;

    /**
     * @brief shutdown all threads and clean up any state
     */
    virtual void stop() = 0;
  };
}



