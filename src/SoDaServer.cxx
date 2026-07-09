/*

  Copyright (c) 2012,2013,2014,2015,2016,2017,2026 Matthew H. Reilly (kb1vc)
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
 * @file SoDaServer.cxx
 *
 * @brief The MAIN process that creates and supervises all the threads that make
 * up the SoDa SDR. 
 *
 * @author Matt Reilly (kb1vc)
 */

#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <fstream>
#include <SoDa/Format.hxx>

#include "SoDaBase.hxx"
#include "SoDaThread.hxx"
#include "SoDaThreadRegistry.hxx"

#include "RadioModels.hxx"
#include <SoDa/MailBox.hxx>

// Include functions to dynamically link any user supplied plugins
#include <dlfcn.h>

// the radio parts. 
#include "Params.hxx" 

#include "BaseBandRX.hxx"
#include "BaseBandTX.hxx"
#include "CWTX.hxx"
#include "UI.hxx"
#include "IFRecorder.hxx"
#include "Command.hxx"
#include "Debug.hxx"
#include "RadioModels.hxx"

#include "AudioQt.hxx"


int loadAccessories(const std::vector<std::string> & libs, SoDa::Debug & d) {
  // are there loadable modules we want to run?
  typedef bool (*initfunctype)();

  for(auto l : libs) {
    initfunctype initfunc; 
    auto handle = dlopen(l.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    d.debugMsg(SoDa::Format("Loaded plugin %0\n").addS(l));
    // now find the "init" method.
    initfunc = (initfunctype) dlsym(handle, "initPlugin");
    const char * dlsym_error = dlerror();
    if(dlsym_error != nullptr) {
      d.debugMsg(SoDa::Format("Shared plugin %0 does not provied an initialization function %1 -- error %2\n")
		 .addS(l)
		 .addS("initPlugin")
		 .addS(dlsym_error)
		 );
    }
    else {
      initfunc();
    }
  }
  return 1; 
}

/// do the work of creating the SoDa threads
/// @param params command line parameter parser object
int doWork(SoDa::ParamsPtr params)
{
  /// create the components of the radio
  SoDa::Debug d(params->getDebugLevel(), "SoDaServer");
  d.setDefaultLevel(params->getDebugLevel());
  
  loadAccessories(params->getLibs(), d);
  
  // These are the mailboxes that connect
  // the various widgets
  std::vector<SoDa::MailBoxBasePtr> mailboxes;
  mailboxes.push_back(SoDa::CDatMBox::make("RXstream"));
  mailboxes.push_back(SoDa::CDatMBox::make("TXstream"));
  mailboxes.push_back(SoDa::CDatMBox::make("IFstream"));
  mailboxes.push_back(SoDa::FDatMBox::make("CWstream"));

  mailboxes.push_back(SoDa::CmdMBox::make("CMDstream"));
  mailboxes.push_back(SoDa::CmdMBox::make("CWTXTstream"));  
  

  auto radio_p = SoDa::RadioModels::make(params->getRadioType(), params);

  if(radio_p == nullptr) {
    std::cerr << SoDa::Format("Radio type [%0] is not yet supported\nServer will terminate.\n")
      .addS(params->getRadioType());
    auto models = SoDa::RadioModels::getModels();
    std::cerr << "Must be one of:\n";
    std::string sp(" ");
    for(auto & m : models) {
      std::cerr << sp << m;
      sp = ", ";
    }
    std::cerr << "\n";
    exit(-1);
  }

  /// Create the audio server on the host machine.
  /// Audio is either via Qt for RX and ALSA for TX.
  /// If ALSA is not present, the server will be RX only.
  /// These are subclasses of the more generic SoDa::AudioIfc class
  //
  
  auto audio_ifc = SoDa::AudioQt::make(params->getAudioSampleRate(),
				       params->getAFBufferSize(),
				       params->getServerSocketBasename());
  /// Create the audio RX and audio TX unit threads
  /// These are also responsible for implementing IF tuning and modulation.
  /// @see SoDa::BaseBandRX @see SoDa::BaseBandTX
  auto bbrx = SoDa::BaseBandRX::make(params, audio_ifc);

  auto bbtx = SoDa::BaseBandTX::make(params, audio_ifc);

  /// Create the morse code (CW) tx handler thread @see SoDa::CWTX
  auto cwtx = SoDa::CWTX::make(params);

  /// Create the user interface (UI) thread @see SoDa::UI
  auto ui = SoDa::UI::make(params);

  /// Create an IF listener process that copies the IF stream to an output file
  /// when requested.
  auto ifrec = SoDa::IFRecorder::make(params);

  
  d.debugMsg("Created units.");

  // get the thing that knows which threads are part of the radio.
  // (Some may be loaded dynamically with the "--load" command line parameter.
  auto thread_registrar = SoDa::ThreadRegistry::getRegistrar();  

  // hook everyone up to the mailboxes.
  thread_registrar->subscribeThreads(mailboxes);

  // Initialize hardware: PlutoRadio::init() calls ctrl->init() (sets 2.5 MSPS),
  // then rx->init() and tx->init() (create IIO DMA buffers).  Must run after
  // subscribeThreads() (so cmd_stream is wired up) but before startThreads()
  // (so txbuf is non-null when PlutoTX::run() begins).
  radio_p->init();

  // Now start each of the activities -- they may or may not
  // implement the "start" method -- not all objects need to be threads.

  d.debugMsg("Starting Threads");

  // start all the threads
  thread_registrar->startThreads();

  thread_registrar->joinThreads();

  // once everyone has joined, we're due to stop
  thread_registrar->shutDownThreads();  
  
  // when we get here, we are done... (UI should not return until it gets an "exit/quit" command.)
  d.debugMsg("Exit");
  
  return 0; 
}

/// main entrypoint
///
/// Start the SoDa USRP radio server threads
/// @param argc number of command line arguments
/// @param argv command line arguments @see doWork
int main(int argc, char * argv[])
{
  /// the command line parameter list is used to
  /// create a param object that holds configuration
  /// information from the command line and from
  /// the stored configuration files.
  /// @see SoDa::Params
  SoDa::ParamsPtr params = SoDa::Params::make(argc, argv);

  try {
    doWork(params);
  }
  catch (SoDa::SDR::Exception exc) {
    std::cerr << "Exception caught at SoDa main: " << std::endl;
    std::cerr << "\t" << exc.toString() << std::endl;
  }
}
