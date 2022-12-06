/*
Copyright (c) 2012,2022 Matthew H. Reilly (kb1vc)
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

#ifndef RTLsdrRX_HDR
#define RTLsdrRX_HDR
#include "SoDaBase.hxx"
#include "RXBase.hxx"
#include "Thread.hxx"
#include "Command.hxx"
#include "Params.hxx"
#include "UI.hxx"
#include <iostream>
#include <fstream>

namespace SoDa {
  /**
   * The Receive RF Path
   *
   * @image html SoDa_Radio_RX_Signal_Path.svg
   */
  class RTLsdrRX : public RXBase {
  public:
    /**
     * The constructor
     *
     * @param params a pointer to a params object that will tell us
     *        about sample rates and other configuration details.
     */
    RTLsdrRX(Params_p params);

    /**
     * RTLsdrRX is RXBase -- this is the thing it will run on each tick. 
     */
    bool runTick();

    void init();

    void cleanUp();
    
    double sampleRate();
  protected:
    void execGetCommand(CmdMsg cmd); 
    void execSetCommand(CmdMsg cmd); 
    void execRepCommand(CmdMsg cmd);
  }; 
}


#endif
