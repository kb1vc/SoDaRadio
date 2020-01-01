/*
Copyright (c) 2019 Matthew H. Reilly (kb1vc)
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

#ifndef SODA_THREAD_REGISTRY_HDR
#define SODA_THREAD_REGISTRY_HDR

#include "SoDaBase.hxx"
#include "SoDaThread.hxx"
#include "Debug.hxx"

 /**
  * @file SoDaThreadRegistry.hxx
  * 
  * A singleton object that records instances of SoDa Thread objects. 
  * 
  * This allows control objects to iterate through threads for things
  * like subscriptions, start/stop, join, etc. 
  *
  * @author Matt Reilly (kb1vc)
  *
  */

#include <list>

namespace SoDa { 
  
  class ThreadRegistry : public std::list<SoDa::Thread *> {
  public:

    static ThreadRegistry * getRegistrar();

    /**
     * @brief register a thread so that it can be connected and started
     * 
     * @param thread a thread object
     * @param version the SoDaRadio version the thread object was built with
     * (This must match the version the registry was built with.)
     *
     */
    void addThread(SoDa::Thread * thread, const std::string & version);
    
    void apply(std::function<bool(SoDa::Thread *)> f);

    void subscribeThreads(const SoDa::MailBoxMap & mailbox_map);
    void startThreads();
    void joinThreads();
    void shutDownThreads();

  private:
    ThreadRegistry() { }    

    static ThreadRegistry * registrar; 
  };
}


#endif
