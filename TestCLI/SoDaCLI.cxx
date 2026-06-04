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
 * @file SoDaCLI.cxx
 *
 * @brief Command-line REPL interface to SoDaServer's command socket.
 *
 * Usage:
 *   START [SoDaServer args...]    -- launch server and connect
 *   SET  <target> [I|D|S] <val>  -- send SET command
 *   GET  <target>                 -- send GET command
 *   REP  <target> [I|D|S] <val>  -- send REP command
 *   RUN  <filename>               -- execute a script
 *   QUIT                          -- stop server (sends STOP) and exit
 *
 * Parameter types: I = integer, D = double, S = string.
 * Enum names (e.g. USB, TX_ON_1, EXTERNAL) are auto-resolved to integers.
 * Doubles are detected by presence of '.' or 'e'.
 *
 * All dialog is logged to CLIHarnessLog.md in the current directory.
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <cerrno>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "Command.hxx"
#include "UDSockets.hxx"

namespace {

std::ofstream log_stream;

void logLine(const std::string & prefix, const std::string & msg) {
  if(log_stream.is_open()) {
    log_stream << prefix << " " << msg << "\n";
    log_stream.flush();
  }
}

SoDa::UD::ClientSocket * cmd_socket = nullptr;
bool connected = false;
std::string socket_basename = "/tmp/SoDa_";
pid_t server_pid = -1;
bool quit_flag = false;

// -------------------------------------------------------------------
// Enum name -> integer resolver

static std::map<std::string, int> buildEnumMap() {
  std::map<std::string, int> m;
  // ModulationType
  m["LSB"] = SoDa::Command::LSB;
  m["USB"] = SoDa::Command::USB;
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
// Parse one parameter token into a Command.
// tok is the first token after the target name.
// iss holds any remaining tokens on the line.
SoDa::CommandPtr buildCommand(SoDa::Command::CmdType ct,
                               SoDa::Command::CmdTarget targ,
                               const std::string & tok,
                               std::istringstream & iss)
{
  if(tok.empty()) {
    return SoDa::Command::make(ct, targ);
  }

  // Explicit type indicator: single char I, D, or S (case-insensitive)
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
    // S
    return SoDa::Command::make(ct, targ, val);
  }

  // Auto-detect: enum name (all uppercase letters/digits/underscores)
  auto it = enum_map.find(tok);
  if(it != enum_map.end()) {
    return SoDa::Command::make(ct, targ, it->second);
  }

  // Auto-detect: double (has '.', 'e', or 'E')
  if(tok.find('.') != std::string::npos ||
     tok.find('e') != std::string::npos ||
     tok.find('E') != std::string::npos) {
    try {
      return SoDa::Command::make(ct, targ, std::stod(tok));
    } catch(...) {}
  }

  // Auto-detect: integer
  try {
    return SoDa::Command::make(ct, targ, std::stoi(tok));
  } catch(...) {}

  // Fall through: string
  return SoDa::Command::make(ct, targ, tok);
}

SoDa::CommandPtr parseCommand(const std::string & verb, const std::string & rest)
{
  if(SoDa::Command::table_needs_init) SoDa::Command::initTables();

  std::istringstream iss(rest);
  std::string targ_str;
  iss >> targ_str;

  std::string tu = targ_str;
  std::transform(tu.begin(), tu.end(), tu.begin(), ::toupper);

  auto tit = SoDa::Command::target_map_s2v.find(tu);
  if(tit == SoDa::Command::target_map_s2v.end()) {
    std::cerr << "Unknown command target: [" << targ_str << "]\n";
    logLine("ERROR:", "Unknown target: " + targ_str);
    return nullptr;
  }
  auto targ = tit->second;

  SoDa::Command::CmdType ct;
  if(verb == "GET") ct = SoDa::Command::GET;
  else if(verb == "SET") ct = SoDa::Command::SET;
  else ct = SoDa::Command::REP;

  if(ct == SoDa::Command::GET) {
    return SoDa::Command::make(ct, targ);
  }

  std::string tok;
  iss >> tok;
  return buildCommand(ct, targ, tok, iss);
}

// -------------------------------------------------------------------
void receiveCommands()
{
  if(!connected || cmd_socket == nullptr) return;
  SoDa::Command cmd;
  while(cmd_socket->get(&cmd, sizeof(SoDa::Command)) > 0) {
    std::string s = cmd.toString();
    std::cout << "\n  << " << s << "\n";
    logLine("RECV:", s);
  }
}

void sendCommand(SoDa::CommandPtr cmd)
{
  if(!connected || cmd_socket == nullptr) {
    std::cerr << "Not connected -- use START first\n";
    return;
  }
  std::string s = cmd->toString();
  std::cout << "  >> " << s << "\n";
  logLine("SEND:", s);
  cmd_socket->put(cmd.get(), sizeof(SoDa::Command));
}

// -------------------------------------------------------------------
void processLine(const std::string & line, bool interactive);

void doStart(const std::string & args_rest)
{
  // tokenize
  std::vector<std::string> args;
  {
    std::istringstream iss(args_rest);
    std::string tok;
    while(iss >> tok) args.push_back(tok);
  }

  // extract socket basename
  socket_basename = "/tmp/SoDa_";
  for(size_t i = 0; i < args.size(); i++) {
    if((args[i] == "--uds_name" || args[i] == "-S") && i + 1 < args.size()) {
      socket_basename = args[i + 1];
      break;
    }
  }

  server_pid = fork();
  if(server_pid == 0) {
    // child: exec SoDaServer
    std::vector<const char *> argv;
    argv.push_back("SoDaServer");
    for(auto & a : args) argv.push_back(a.c_str());
    argv.push_back(nullptr);
    execvp("SoDaServer", const_cast<char **>(argv.data()));
    std::cerr << "exec SoDaServer failed: " << strerror(errno) << "\n";
    _exit(-1);
  }
  if(server_pid < 0) {
    std::cerr << "fork() failed: " << strerror(errno) << "\n";
    return;
  }

  std::string sock_path = socket_basename + "_cmd";
  std::cout << "Waiting for server socket [" << sock_path << "]...\n";
  logLine("STATUS:", "waiting for " + sock_path);

  bool found = false;
  for(int i = 0; i < 30; i++) {
    struct stat st;
    if(stat(sock_path.c_str(), &st) == 0) { found = true; break; }
    int wstatus;
    if(waitpid(server_pid, &wstatus, WNOHANG) > 0) {
      std::cerr << "SoDaServer exited early (status " << wstatus << ")\n";
      server_pid = -1;
      return;
    }
    sleep(1);
  }

  if(!found) {
    std::cerr << "Timed out waiting for server socket\n";
    return;
  }

  cmd_socket = new SoDa::UD::ClientSocket(sock_path, 10);
  connected = true;
  std::cout << "Connected to SoDaServer\n";
  logLine("STATUS:", "connected to " + sock_path);
}

void doRun(const std::string & filename)
{
  logLine("RUN:", filename);
  std::ifstream script(filename);
  if(!script.is_open()) {
    std::cerr << "Cannot open script [" << filename << "]\n";
    logLine("ERROR:", "cannot open " + filename);
    return;
  }
  std::string line;
  while(std::getline(script, line)) {
    std::cout << "  > " << line << "\n";
    processLine(line, false);
    if(quit_flag) break;
    if(connected) receiveCommands();
  }
}

void doQuit()
{
  logLine("STATUS:", "QUIT");
  if(connected && cmd_socket != nullptr) {
    auto stop = SoDa::Command::make(SoDa::Command::SET, SoDa::Command::STOP);
    sendCommand(stop);
    sleep(1);
    receiveCommands();
    delete cmd_socket;
    cmd_socket = nullptr;
    connected = false;
  }
  if(server_pid > 0) {
    int wstatus;
    waitpid(server_pid, &wstatus, WNOHANG);
    server_pid = -1;
  }
  quit_flag = true;
}

// -------------------------------------------------------------------
void processLine(const std::string & line, bool interactive)
{
  // skip blank lines and comments
  std::string trimmed = line;
  size_t first = trimmed.find_first_not_of(" \t\r\n");
  if(first == std::string::npos) return;
  trimmed = trimmed.substr(first);
  if(trimmed[0] == '#') return;

  if(interactive) logLine("INPUT:", line);

  std::istringstream iss(trimmed);
  std::string verb;
  iss >> verb;

  std::string vu = verb;
  std::transform(vu.begin(), vu.end(), vu.begin(), ::toupper);

  if(vu == "START") {
    std::string rest;
    std::getline(iss, rest);
    doStart(rest);
  } else if(vu == "SET" || vu == "GET" || vu == "REP") {
    std::string rest;
    std::getline(iss, rest);
    auto cmd = parseCommand(vu, rest);
    if(cmd) sendCommand(cmd);
  } else if(vu == "RUN") {
    std::string filename;
    iss >> filename;
    doRun(filename);
  } else if(vu == "QUIT") {
    doQuit();
  } else {
    std::cerr << "Unknown verb [" << verb << "] -- try SET GET REP START RUN QUIT\n";
    logLine("ERROR:", "unknown verb: " + verb);
  }
}

// -------------------------------------------------------------------
void runREPL()
{
  std::string line;

  while(!quit_flag) {
    std::cout << "SoDaCLI> " << std::flush;

    if(connected && cmd_socket != nullptr) {
      // Wait for stdin, draining incoming socket data while we wait
      while(!quit_flag) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        FD_SET(cmd_socket->conn_socket, &rfds);
        int maxfd = std::max(STDIN_FILENO, cmd_socket->conn_socket);
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000;
        select(maxfd + 1, &rfds, nullptr, nullptr, &tv);

        if(FD_ISSET(cmd_socket->conn_socket, &rfds)) {
          receiveCommands();
        }
        if(FD_ISSET(STDIN_FILENO, &rfds)) break;
      }
    }

    if(quit_flag) break;
    if(!std::getline(std::cin, line)) break;
    processLine(line, true);
    if(connected) receiveCommands();
  }
}

} // anonymous namespace

int main(int /*argc*/, char * /*argv*/[])
{
  log_stream.open("CLIHarnessLog.md", std::ios::app);
  log_stream << "\n# SoDaCLI Session\n\n";

  std::cout << "SoDaCLI -- command line interface to SoDaServer\n"
            << "  START [server-args]        -- launch server and connect\n"
            << "  SET   <target> [I|D|S] v   -- send SET (int/double/string)\n"
            << "  GET   <target>             -- send GET\n"
            << "  REP   <target> [I|D|S] v   -- send REP\n"
            << "  RUN   <filename>           -- execute script\n"
            << "  QUIT                       -- send STOP and exit\n"
            << "  Parameter types are auto-detected; enum names (USB, TX_ON_1, ...) "
               "are accepted for integer params.\n\n";

  runREPL();

  log_stream.close();
  return 0;
}
