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
 * @brief Main REPL for the SoDaServer command-line interface.
 *
 * See @ref SoDaCLI_manual for the user documentation.
 */

#include "CLICommand.hxx"
#include "CLIAudio.hxx"

#include <QCoreApplication>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <cerrno>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace {

std::ofstream         log_stream;
SoDa::UD::ClientSocket * cmd_socket     = nullptr;
bool                  connected         = false;
std::string           socket_basename   = "/tmp/SoDa_";
pid_t                 server_pid        = -1;
bool                  quit_flag         = false;

SoDaCLI::AudioOutThread * audio_out     = nullptr;
SoDaCLI::AudioInThread  * audio_in      = nullptr;

// -------------------------------------------------------------------
void logLine(const std::string & prefix, const std::string & msg)
{
  log_stream << prefix << " " << msg << "\n";
  log_stream.flush();
}

// -------------------------------------------------------------------
// Forward declaration
void processLine(const std::string & line, bool interactive);

// -------------------------------------------------------------------
void doStart(const std::string & args_rest)
{
  std::vector<std::string> args;
  {
    std::istringstream iss(args_rest);
    std::string tok;
    while(iss >> tok) args.push_back(tok);
  }

  socket_basename = "/tmp/SoDa_";
  for(size_t i = 0; i < args.size(); i++) {
    if((args[i] == "--uds_name" || args[i] == "-S") && i + 1 < args.size()) {
      socket_basename = args[i + 1];
      break;
    }
  }

  server_pid = fork();
  if(server_pid == 0) {
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
  connected  = true;
  std::cout << "Connected to SoDaServer\n";
  logLine("STATUS:", "connected to " + sock_path);
}

// -------------------------------------------------------------------
void doAudio(const std::string & rest)
{
  std::istringstream iss(rest);
  std::string subverb;
  iss >> subverb;
  std::transform(subverb.begin(), subverb.end(), subverb.begin(), ::toupper);

  if(subverb == "OUT") {
    if(!connected) { std::cerr << "Not connected -- use START first\n"; return; }

    std::string devname;
    std::getline(iss, devname);
    // trim leading whitespace
    auto first = devname.find_first_not_of(" \t");
    devname = (first != std::string::npos) ? devname.substr(first) : "default";
    if(devname.empty()) devname = "default";

    if(audio_out) { audio_out->stopAudio(); delete audio_out; audio_out = nullptr; }

    auto dev = SoDaCLI::findOutputDevice(devname);
    std::cout << "Audio output: " << dev.description().toStdString() << "\n";
    logLine("AUDIO_OUT:", dev.description().toStdString());

    audio_out = new SoDaCLI::AudioOutThread(socket_basename + "_rxa", dev, 0.0f);
    audio_out->start();

  } else if(subverb == "IN") {
    if(!connected) { std::cerr << "Not connected -- use START first\n"; return; }

    std::string devname;
    std::getline(iss, devname);
    auto first = devname.find_first_not_of(" \t");
    devname = (first != std::string::npos) ? devname.substr(first) : "default";
    if(devname.empty()) devname = "default";

    if(audio_in) { audio_in->stopAudio(); delete audio_in; audio_in = nullptr; }

    auto dev = SoDaCLI::findInputDevice(devname);
    std::cout << "Audio input: " << dev.description().toStdString() << "\n";
    logLine("AUDIO_IN:", dev.description().toStdString());

    audio_in = new SoDaCLI::AudioInThread(socket_basename + "_txa", dev, 0.0f);
    audio_in->start();

  } else if(subverb == "VOLUME") {
    float gain = 0.0f;
    iss >> gain;
    if(audio_out) audio_out->setVolume(gain);
    else std::cerr << "No audio output active -- use AUDIO OUT first\n";
    logLine("AUDIO_VOLUME:", std::to_string(gain));

  } else if(subverb == "GAIN") {
    float gain = 0.0f;
    iss >> gain;
    if(audio_in) audio_in->setGain(gain);
    else std::cerr << "No audio input active -- use AUDIO IN first\n";
    logLine("AUDIO_GAIN:", std::to_string(gain));

  } else {
    std::cerr << "Unknown AUDIO subcommand [" << subverb
              << "] -- try OUT, IN, VOLUME, GAIN\n";
    logLine("ERROR:", "unknown AUDIO subcommand: " + subverb);
  }
}

// -------------------------------------------------------------------
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
    if(connected) SoDaCLI::receiveCommands(cmd_socket, log_stream);
  }
}

// -------------------------------------------------------------------
void doQuit()
{
  logLine("STATUS:", "QUIT");
  if(connected && cmd_socket != nullptr) {
    auto stop = SoDa::Command::make(SoDa::Command::SET, SoDa::Command::STOP);
    SoDaCLI::sendCommand(cmd_socket, stop, log_stream);
    sleep(1);
    SoDaCLI::receiveCommands(cmd_socket, log_stream);
    delete cmd_socket;
    cmd_socket = nullptr;
    connected  = false;
  }
  if(audio_out) { audio_out->stopAudio(); delete audio_out; audio_out = nullptr; }
  if(audio_in)  { audio_in->stopAudio();  delete audio_in;  audio_in  = nullptr; }
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
    std::string rest; std::getline(iss, rest);
    doStart(rest);
  } else if(vu == "SET" || vu == "GET" || vu == "REP") {
    if(!connected) { std::cerr << "Not connected -- use START first\n"; return; }
    std::string rest; std::getline(iss, rest);
    auto cmd = SoDaCLI::parseCommand(vu, rest);
    if(cmd) SoDaCLI::sendCommand(cmd_socket, cmd, log_stream);
  } else if(vu == "AUDIO") {
    std::string rest; std::getline(iss, rest);
    doAudio(rest);
  } else if(vu == "RUN") {
    std::string filename; iss >> filename;
    doRun(filename);
  } else if(vu == "QUIT") {
    doQuit();
  } else {
    std::cerr << "Unknown verb [" << verb << "] -- try SET GET REP AUDIO START RUN QUIT\n";
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
      while(!quit_flag) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        FD_SET(cmd_socket->conn_socket, &rfds);
        int maxfd = std::max(STDIN_FILENO, cmd_socket->conn_socket);
        struct timeval tv;
        tv.tv_sec  = 0;
        tv.tv_usec = 100000;
        select(maxfd + 1, &rfds, nullptr, nullptr, &tv);

        if(FD_ISSET(cmd_socket->conn_socket, &rfds)) {
          SoDaCLI::receiveCommands(cmd_socket, log_stream);
        }
        if(FD_ISSET(STDIN_FILENO, &rfds)) break;
      }
    }

    if(quit_flag) break;
    if(!std::getline(std::cin, line)) break;
    processLine(line, true);
    if(connected) SoDaCLI::receiveCommands(cmd_socket, log_stream);

    // Give Qt's internal housekeeping a chance to run.
    QCoreApplication::processEvents();
  }
}

} // anonymous namespace

// -------------------------------------------------------------------
int main(int argc, char * argv[])
{
  // QCoreApplication must exist before any Qt multimedia objects are created.
  QCoreApplication app(argc, argv);

  log_stream.open("CLIHarnessLog.md", std::ios::app);
  log_stream << "\n# SoDaCLI Session\n\n";

  std::cout
    << "SoDaCLI -- command line interface to SoDaServer\n"
    << "  START [server-args]             -- launch server and connect\n"
    << "  SET   <target> [I|D|S] v        -- send SET (int/double/string)\n"
    << "  GET   <target>                  -- send GET\n"
    << "  REP   <target> [I|D|S] v        -- send REP\n"
    << "  AUDIO OUT  [device]             -- connect RX audio to speaker (muted)\n"
    << "  AUDIO VOLUME <v>                -- set output volume [0.0-1.0]\n"
    << "  AUDIO IN   [device]             -- connect mic audio to TX (muted)\n"
    << "  AUDIO GAIN <g>                  -- set input gain multiplier\n"
    << "  RUN   <filename>                -- execute script\n"
    << "  QUIT                            -- send STOP and exit\n"
    << "  Device name: \"default\" or substring of device description.\n"
    << "  Enum names (USB, TX_ON_1, ...) auto-resolve for integer params.\n\n";

  runREPL();

  log_stream.close();
  return 0;
}
