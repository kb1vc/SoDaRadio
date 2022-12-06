#pragma once
/*
Copyright (c) 2012,2013,2014,2022 Matthew H. Reilly (kb1vc)
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
#include "TXBase.hxx"
#include "Thread.hxx"
#include "Command.hxx"
#include "Params.hxx"
#include "MailBoxTypes.hxx"

namespace SoDa {
  /**
   * The Transmit RF Path
   *
   * @image html SoDa_Radio_TX_Signal_Path.svg
   *
   * In SSB/AM/FM modes, the NullRadioTX unit accepts an I/Q audio
   * stream from the BaseBandTX unit and forwards it to the NullRadio.
   * In CW mode, the NullRadioTX unit impresses a CW envelope (received
   * from the CW unit) onto a carrier and passes this to the NullRadio. 
   *
   */
  class NullRadioTX : public TXBase {
  public:
    /**
     * @brief Constructor for RF Transmit/modulator process
     *
     * @param params block describing intial setup of the radio
     *
     */
    NullRadioTX(Params_p params);
    
    /**
     * @brief NullRadioTX run step: modulate the tx carrier
     */
    bool runTick(); 

    /**
     * @brief report the RX stream sample rate.  This is the sample rate
     * for both the IF and RX streams. The run method will use this value
     * to notify the Ctrl, BaseBandRX, UI, and spectrogram widgets. 
     */
    double sampleRate();
        
    /**
     * @brief perform any actions that need to occur before the first
     * call to runTick(). This is called *after* all mailboxes are subscribed. 
     * The init method should NOT loop and should not attempt to *read* a mailbox. 
     */
    void init();
    
    /**
     * @brief perform any actions that need to occur *after* the final 
     * call to runTick. (This isn't really a destructor, as it probably 
     * makes sense to do this before any wrap up.  We might need to do
     * things in a particular order?  Idunno.)
     */
    void cleanUp();
    
    
  private:

    
    /**
     * @brief execute GET commands from the command channel
     * @param cmd the incoming command
     */
    void execGetCommand(CmdMsg  cmd); 
    /**
     * @brief handle SET commands from the command channel
     * @param cmd the incoming command
     */
    void execSetCommand(CmdMsg  cmd); 
    /**
     * @brief handle Report commands from the command channel
     * @param cmd the incoming command
     */
    void execRepCommand(CmdMsg  cmd);
  }; 

}

