/*
  Copyright (c) 2019,2023,2024 Matthew H. Reilly (kb1vc)
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
#include "SoDaBase.hxx"
#include "SoDaThread.hxx"


/**
 * @file SimpleAccessory.hxx
 *
 * an example of a simple bolt-on accessory for SoDaRadio.  It counts
 * the number of commands that go by and reports the count at the end
 * of the run. 
 *
 * @author Matt Reilly (kb1vc)
 *
 */

// namespace doesn't matter here... let's do without. 
class SimpleAccessory : public SoDa::Thread {
public:
  SimpleAccessory(const std::string & name);

  /**
   * @brief connect to useful mailboxes. 
   * 
   * @param cmd_boxes The name/mailbox pairs for all CmdMailBox objects
   * @param dat_boxes The name/mailbox pairs for all DatMailBox objects     
   */
  void subscribeToMailBoxList(SoDa::CmdMailBoxMap & cmd_boxes,
			      SoDa::DatMailBoxMap & dat_boxes);

  void run();

  /**
   * @brief execute GET commands from the command channel
   * @param cmd the incoming command
   */
  void execGetCommand(SoDa::CommandPtr  cmd) {
    get_count++; 
  }
  /**
   * @brief handle SET commands from the command channel
   * @param cmd the incoming command
   */
  void execSetCommand(SoDa::CommandPtr  cmd) {
    set_count++;
  }
  /**
   * @brief handle Report commands from the command channel
   * @param cmd the incoming command
   */
  void execRepCommand(SoDa::CommandPtr  cmd) {
    rep_count++;
  }

  void shutDown() {
    printReport();
  }

  void printReport();

  SoDa::CmdMBoxPtr cmd_stream; ///< mailbox producing command stream from user
    
  unsigned int get_count, set_count, rep_count; 
};
