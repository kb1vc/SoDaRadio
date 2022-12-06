#pragma once
/*
Copyright (c) 2022, Matthew H. Reilly (kb1vc)
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
 *   @file RTLsdrCtrl.hxx
 *   @brief Base class describing functions 
 *   provided by any supported radio.
 * 
 * 
 *   @author M. H. Reilly (kb1vc)
 *   @date   June 2022
 */


#include "SoDaBase.hxx"
#include "Thread.hxx"
#include "MailBoxTypes.hxx"
#include "Command.hxx"
#include "Params.hxx"

namespace SoDa {

  /**
   *   @class RTLsdrCtrl
   *  
   * RTLsdrCtrl listens on the Command Stream message channel for
   * requests from other components (including the SoDa::UI listener)
   * and dumps status and completion reports back onto the command
   * stream channel.
   * 
   * In the ideal case, derived classes would not need to worry about
   * the command stream at all. Any virtual methods that query state
   * return the relevant information, and the RTLsdrCtrl will package it
   * up to send out.
   * 
   * Likewise, incoming commands are dispatched by the RTLsdrCtrl
   * handlers.  The incoming stream is also passed to handlers in the
   * specific radio, but implementation of any handler is optional.
   */ 
  class RTLsdrCtrl : public SoDa::Thread {
  public:
    /**
     *  Constructor Build a RTLsdrCtrl thread @param params Pointer to a
     *  parameter object with all the initial settings and
     *  identification for the attached radio
     */
    RTLsdrCtrl(Params_p params);

    /**
     * @returns true if "runTick" has done all the work required in a run loop
     * (checked the command stream and handled stuff.)
     */
    virtual bool runTick() { return false; }

    void execGetCommand(CmdMsg cmd);
    void execSetCommand(CmdMsg cmd);
    void execRepCommand(CmdMsg cmd);

    void subscribe();
    
  protected:
    Params_p params;

    MsgMBoxPtr cmd_stream; ///< command stream channel
    MsgSubs cmd_subs;   ///< subscriber ID for this thread's connection to the command channel
    
  };
}
