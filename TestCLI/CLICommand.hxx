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
 * @file CLICommand.hxx
 * @brief Command parsing and command-socket I/O for SoDaCLI.
 */

#include "Command.hxx"
#include "UDSockets.hxx"
#include <fstream>
#include <string>

namespace SoDaCLI {

  /**
   * @brief Parse an upper-cased verb ("SET","GET","REP") and the rest of
   *        the input line into a SoDa::Command.
   *
   * Parameter type is auto-detected:
   *   - Known enum names (USB, TX_ON_1, EXTERNAL, …) → integer
   *   - Values containing '.', 'e', or 'E' → double
   *   - Anything parseable as a plain integer → integer
   *   - Remaining text → string
   *
   * An explicit single-character type indicator (I / D / S, case-insensitive)
   * may precede the value to force a particular encoding.
   *
   * @param verb  Upper-cased command verb.
   * @param rest  Everything on the line after the verb.
   * @return Shared pointer to the new command, or nullptr on error.
   */
  SoDa::CommandPtr parseCommand(const std::string & verb,
                                 const std::string & rest);

  /**
   * @brief Transmit a command via the command socket and log it.
   * @param sock  Connected ClientSocket to SoDaServer.
   * @param cmd   Command to send.
   * @param log   Open log stream.
   * @return true on success, false if the socket reported an error.
   */
  bool sendCommand(SoDa::UD::ClientSocket * sock,
                   SoDa::CommandPtr cmd,
                   std::ofstream & log);

  /**
   * @brief Drain all pending inbound commands from the server socket,
   *        print and log each one.
   * @param sock  Connected ClientSocket to SoDaServer.
   * @param log   Open log stream.
   * @return true if the socket is still healthy, false if the server is gone.
   */
  bool receiveCommands(SoDa::UD::ClientSocket * sock,
                       std::ofstream & log);

} // namespace SoDaCLI
