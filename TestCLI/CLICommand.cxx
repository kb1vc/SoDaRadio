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

#include "CLICommand.hxx"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <map>
#include <stdexcept>
#include <cctype>

namespace SoDaCLI {

// -------------------------------------------------------------------
// Enum name -> integer resolver

static std::map<std::string, int> buildEnumMap()
{
  std::map<std::string, int> m;
  // ModulationType
  m["LSB"]  = SoDa::Command::LSB;
  m["USB"]  = SoDa::Command::USB;
  m["CW_U"] = SoDa::Command::CW_U;
  m["CW_L"] = SoDa::Command::CW_L;
  m["AM"]   = SoDa::Command::AM;
  m["WBFM"] = SoDa::Command::WBFM;
  m["NBFM"] = SoDa::Command::NBFM;
  // RxTxState
  m["TX_OFF_0"] = SoDa::Command::TX_OFF_0;
  m["TX_OFF_1"] = SoDa::Command::TX_OFF_1;
  m["TX_OFF_2"] = SoDa::Command::TX_OFF_2;
  m["TX_ON_0"]  = SoDa::Command::TX_ON_0;
  m["TX_ON_1"]  = SoDa::Command::TX_ON_1;
  m["TX_ON_2"]  = SoDa::Command::TX_ON_2;
  // ClockSource
  m["EXTERNAL"] = SoDa::Command::EXTERNAL;
  m["INTERNAL"] = SoDa::Command::INTERNAL;
  // TXAudioSelector
  m["MIC"]   = SoDa::Command::MIC;
  m["NOISE"] = SoDa::Command::NOISE;
  return m;
}

static const std::map<std::string, int> enum_map = buildEnumMap();

// -------------------------------------------------------------------
// Build a Command from a single parsed token + remainder stream.

static SoDa::CommandPtr buildCommand(SoDa::Command::CmdType ct,
                                      SoDa::Command::CmdTarget targ,
                                      const std::string & tok,
                                      std::istringstream & iss)
{
  if(tok.empty()) {
    return SoDa::Command::make(ct, targ);
  }

  // Explicit single-character type indicator (I / D / S, case-insensitive)
  char tc = std::toupper((unsigned char)tok[0]);
  if(tok.size() == 1 && (tc == 'I' || tc == 'D' || tc == 'S')) {
    std::string val;
    iss >> val;
    if(tc == 'I') {
      auto it = enum_map.find(val);
      int v = (it != enum_map.end()) ? it->second : std::stoi(val);
      return SoDa::Command::make(ct, targ, v);
    }
    if(tc == 'D') {
      return SoDa::Command::make(ct, targ, std::stod(val));
    }
    return SoDa::Command::make(ct, targ, val);   // S
  }

  // Auto-detect: known enum name
  auto it = enum_map.find(tok);
  if(it != enum_map.end()) {
    return SoDa::Command::make(ct, targ, it->second);
  }

  // Auto-detect: double (has '.', 'e', or 'E')
  if(tok.find('.') != std::string::npos ||
     tok.find('e') != std::string::npos ||
     tok.find('E') != std::string::npos) {
    try { return SoDa::Command::make(ct, targ, std::stod(tok)); }
    catch(...) {}
  }

  // Auto-detect: integer
  try { return SoDa::Command::make(ct, targ, std::stoi(tok)); }
  catch(...) {}

  // Fall back to string — warn the user since this is likely a mistyped enum or number.
  std::cerr << "Warning: [" << tok
            << "] is not a recognised enum name or number; sending as string.\n";
  return SoDa::Command::make(ct, targ, tok);
}

// -------------------------------------------------------------------

SoDa::CommandPtr parseCommand(const std::string & verb, const std::string & rest)
{
  if(SoDa::Command::table_needs_init) SoDa::Command::initTables();

  std::istringstream iss(rest);
  std::string targ_str;
  iss >> targ_str;

  std::string tu = targ_str;
  std::transform(tu.begin(), tu.end(), tu.begin(), ::toupper);

  if(tu == "?") {
    std::cout << verb << " targets:\n";
    int col = 0;
    for(const auto & kv : SoDa::Command::target_map_s2v) {
      std::cout << "  " << std::left << std::setw(24) << kv.first;
      if(++col % 4 == 0) std::cout << "\n";
    }
    if(col % 4 != 0) std::cout << "\n";
    return nullptr;
  }

  auto tit = SoDa::Command::target_map_s2v.find(tu);
  if(tit == SoDa::Command::target_map_s2v.end()) {
    std::cerr << "Unknown command target: [" << targ_str << "]\n";
    return nullptr;
  }
  auto targ = tit->second;

  SoDa::Command::CmdType ct;
  if(verb == "GET")     ct = SoDa::Command::GET;
  else if(verb == "SET") ct = SoDa::Command::SET;
  else                   ct = SoDa::Command::REP;

  if(ct == SoDa::Command::GET) {
    return SoDa::Command::make(ct, targ);
  }

  std::string tok;
  iss >> tok;
  return buildCommand(ct, targ, tok, iss);
}

// -------------------------------------------------------------------

bool sendCommand(SoDa::UD::ClientSocket * sock,
                 SoDa::CommandPtr cmd,
                 std::ofstream & log)
{
  std::string s = cmd->toString();
  std::cout << "  >> " << s << "\n";
  log << "SEND: " << s << "\n";
  log.flush();
  int r = sock->put(cmd.get(), sizeof(SoDa::Command));
  return r >= 0;
}

// -------------------------------------------------------------------

// Returns false if the socket signals that the server has gone away.
bool receiveCommands(SoDa::UD::ClientSocket * sock,
                     std::ofstream & log)
{
  SoDa::Command cmd;
  int r;
  while((r = sock->get(&cmd, sizeof(SoDa::Command))) > 0) {
    std::string s = cmd.toString();
    std::cout << "\n  << " << s << "\n";
    log << "RECV: " << s << "\n";
    log.flush();
  }
  // r == 0 means EAGAIN (no data yet) — normal.
  // r < 0 means a hard error — server gone.
  return r >= 0;
}

} // namespace SoDaCLI
