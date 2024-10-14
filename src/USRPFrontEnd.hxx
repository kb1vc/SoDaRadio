/*
  Copyright (c) 2017, Matthew H. Reilly (kb1vc)
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

#pragma once

#include <string>
#include <iostream>
#include "PropTree.hxx"

namespace SoDa {
  /** 
   * @file USRPFrontEnd.hxx
   * Manage front end settings for Ettus USRP devices
   * Functions that return a property tree corresponding to 
   * a TX or RX front end on the first daughterboard.  The "best" choice
   * will be returned.  Best is the first option that provides an IQ 
   * or QI connection type.
   */

  /**
   * @brief return a pointer to a PropTree object for the first "T" or "R" front end
   * that provides an IQ or QI stream.
   * 
   * @param tree pointer to a PropTree root 
   * @param tr_choice 'T', 't' for transmit, 'R', 'r' for receive.
   * @return PropTree object or NULL if none found
   */
  PropTree * getUSRPFrontEnd(PropTree * tree, char tr_choice);  
  /**
   * @brief return a pointer to a PropTree object for the first "T" or "R" front end
   * that provides an IQ or QI stream.
   * 
   * @param tree reference to a PropTree root 
   * @param tr_choice 'T', 't' for transmit, 'R', 'r' for receive.
   * @return PropTree object or NULL if none found
   */
  PropTree * getUSRPFrontEnd(PropTree & tree, char tr_choice);
}
