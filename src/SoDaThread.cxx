#include "SoDaThread.hxx"
#include "SoDaThreadRegistry.hxx"
#include "Command.hxx"
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


SoDa::Thread::Thread(const std::string & oname, const std::string & version) : SoDa::Base(oname), Debug(oname) {
  
  SoDa::ThreadRegistry::getRegistrar()->addThread(this, version);
  thread_ptr = nullptr;
  
  // subscribe to whatever mailbox streams we need
  subscribe();
}

void SoDa::Thread::execCommand(std::shared_ptr<Command>  cmd) 
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


void  SoDa::Thread::outerRun() {
  hookSigSeg();
  debugMsg(getObjName() + " starting.\n");
  try {
    run(); 
  }
  catch (SoDa::Radio::Exception exc) {
    std::cerr << getObjName() << " caught " << exc.toString() << std::endl;
  }
  catch (SoDa::Radio::Exception * exc) {
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

void  SoDa::Thread::sigsegHandler(int sig)
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

void SoDa::Thread::hookSigSeg() {
  struct sigaction act;
  sigemptyset(&act.sa_mask);
  act.sa_handler = SoDa::Thread::sigsegHandler;
  act.sa_flags = 0;
  sigaction(SIGSEGV, &act, 0);
}

