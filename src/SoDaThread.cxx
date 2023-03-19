/*
  Copyright (c) 2012,2013,2014,2019,2023 Matthew H. Reilly (kb1vc)
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

#include "SoDaThread.hxx"
#include "SoDaThreadRegistry.hxx"
#include "Command.hxx"
#include "MultiMBox.hxx"
#include "Debug.hxx"
#include "version.h"

#include <string>
#include <SoDa/Format.hxx>

#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

extern "C" {
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
}

namespace SoDa {
  Thread::Thread(const std::string & oname, const std::string & version) : Base(oname), Debug(oname) {
  
    ThreadRegistry::getRegistrar()->addThread(this, version);
    thread_ptr = nullptr;
  }

  void Thread::execCommand(CommandPtr  cmd) 
  {
    switch (cmd->cmd) {
    case Command::GET:
      execGetCommand(cmd); 
      break;
    case Command::SET:
      execSetCommand(cmd); 
      break; 
    case Command::REP:
      execRepCommand(cmd); 
      break;
    default:
      break; 
    }
  }


  void  Thread::outerRun() {
    hookSigSeg();
    debugMsg(getObjName() + " starting.\n");
    try {
      run(); 
    }
    catch (Radio::Exception exc) {
      std::cerr << getObjName() << " caught " << exc.toString() << std::endl;
    }
    catch (Radio::Exception * exc) {
      std::cerr << getObjName() << " caught " << exc->toString() << std::endl;
    }
    catch (const std::exception & e) {
      std::cerr << getObjName() << " caught exception here: " << e.what() << std::endl; 
    }
    catch (...) {
      std::cerr << getObjName() << " caught unknown exception" << std::endl;
    }
    debugMsg(getObjName() + " terminating.\n");
  }

  void  Thread::sigsegHandler(int sig)
  {
    std::cerr << "\n-----------"
	      << "\n-----------"
	      << "\n-----------"
	      << "\n-----------"
	      << " A SoDaThread caught a sig segv"
	      << "\n-----------"
	      << "\n-----------"
	      << "\n-----------"
	      << "\n-----------"
	      << std::endl;
    // resignal
    signal(sig, SIG_DFL);
    kill(getpid(), SIGSEGV);
  }

  void Thread::hookSigSeg() {
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler = Thread::sigsegHandler;
    act.sa_flags = 0;
    sigaction(SIGSEGV, &act, 0);
  }

}
