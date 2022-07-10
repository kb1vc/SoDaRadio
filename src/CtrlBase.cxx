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

#include "CtrlBase.hxx"

#include <SoDa/Format.hxx>

/** 
 * These are the commands that the Ctrl unit must handle 
 * 
SoDa::Command(SoDa::Command::SET, SoDa::Command::RX_RETUNE_FREQ, freq
SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_RETUNE_FREQ, freq
SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_STATE, tx_state, full_duplex

 * 
 * Optionally, the GUI will send these messages which can be ignored or 
 * answered pro-forma by devices that do not support the function. 
SoDa::Command(SoDa::Command::SET, SoDa::Command::RX_ANT, antname.toStdString()
SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_ANT, antname.toStdString()
SoDa::Command(SoDa::Command::SET, SoDa::Command::CLOCK_SOURCE, clock_source
SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_RF_GAIN, dgain
SoDa::Command(SoDa::Command::SET, SoDa::Command::RX_RF_GAIN, dgain

*
* These messages are handled by the BasebandRX and RX units
SoDa::Command(SoDa::Command::SET, SoDa::Command::RX_AF_FILTER, id
SoDa::Command(SoDa::Command::SET, SoDa::Command::RX_AF_GAIN, dgain
SoDa::Command(SoDa::Command::SET, SoDa::Command::RX_AF_SIDETONE_GAIN, dgain
SoDa::Command(SoDa::Command::SET, SoDa::Command::RX_AF_SIDETONE_GAIN, ((double) vol)

* The BaseBandTX and TX unit
SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_BEACON, carrier_state
SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_CW_FLUSHTEXT
SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_CW_SPEED, speed
SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_CW_TEXT, cwbuf
SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_MODE, mod_id


* The RX IF recorder
SoDa::Command(SoDa::Command::SET, SoDa::Command::RF_RECORD_START, fname.toStdString()
SoDa::Command(SoDa::Command::SET, SoDa::Command::RF_RECORD_STOP

SoDa::Command(SoDa::Command::SET, SoDa::Command::NBFM_SQUELCH, ((double) lev)
SoDa::Command(SoDa::Command::SET, SoDa::Command::RX_MODE, mod_id
* The Spectrum widget
SoDa::Command(SoDa::Command::SET, SoDa::Command::SPEC_AVG_WINDOW, window
SoDa::Command(SoDa::Command::SET, SoDa::Command::SPEC_CENTER_FREQ, freq
SoDa::Command(SoDa::Command::SET, SoDa::Command::SPEC_UPDATE_RATE, rate

* Everybody
SoDa::Command(SoDa::Command::SET, SoDa::Command::STOP, 0

