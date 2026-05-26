/*
Copyright (c) 2019, 2025 Matthew H. Reilly (kb1vc)
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
#include "Debug.hxx"
#include <functional>

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
  class ThreadRegistry;
  typedef std::shared_ptr<ThreadRegistry> ThreadRegistryPtr;
  
  class ThreadRegistry  {
  public:

    static ThreadRegistryPtr getRegistrar();

    /**
     * @brief register a thread so that it can be connected and started
     * 
     * @param thread a thread object
     * @param version the SoDaRadio version the thread object was built with
     * (This must match the version the registry was built with.)
     *
     */
    static void addThread(SoDa::ThreadPtr thread, const std::string & version);
    
    static void apply(std::function<bool(SoDa::ThreadPtr)> f);

    static void subscribeThreads(const std::vector<SoDa::MailBoxBasePtr> & mailboxes); 
    static void startThreads();
    static void joinThreads();
    static void shutDownThreads();
    
  private:
    /**
     * @brief register a thread so that it can be connected and started
     * 
     * @param thread a thread object
     * @param version the SoDaRadio version the thread object was built with
     * (This must match the version the registry was built with.)
     *
     */
    void priv_addThread(SoDa::ThreadPtr thread, const std::string & version);
    
    void priv_apply(std::function<bool(SoDa::ThreadPtr)> f);

    void priv_subscribeThreads(const std::vector<SoDa::MailBoxBasePtr> & mailboxes); 
    void priv_startThreads();
    void priv_joinThreads();
    void priv_shutDownThreads();

  private:
    ThreadRegistry() { }    

    std::vector<SoDa::ThreadPtr> thread_list;
    static ThreadRegistryPtr registrar; 
  };
}


