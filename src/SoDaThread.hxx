/*
Copyright (c) 2012,2013,2014,2019 Matthew H. Reilly (kb1vc)
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

#ifndef SODA_THREAD_HDR
#define SODA_THREAD_HDR

#include "SoDaBase.hxx"
#include <string>
#include "Debug.hxx"
#include <thread>
#include <chrono>
#include <mutex>
#include <memory>
#include <condition_variable>

#include "version.h"
 /**
  * @file SoDaThread.hxx
  * The Baseclass for all SoDa thread objects.  These are used for
  * blocks that wait on messages from the message rings, and act on
  * them.  A SoDaThread subscribes to message rings, and exports 
  * a "run" method.
  *
  * @brief SoDa base and commonly used classes
  *
  * @author Matt Reilly (kb1vc)
  *
  */


namespace SoDa {
  
  class BaseMBox; 
  class Command;
  
  typedef std::map<std::string, BaseMBox *> MailBoxMap; 
  
  /**
   * The Thread baseclass for all SoDa thread objects.
   *
   * @class SoDa::Thread
   *
   * The SoDaThread baseclass simplifies interaction with threads.
   * Since SoDa thread objects aren't created and destroyed very often,
   * we don't need many bells and whistles here.  Just the ability to
   * start a thread a join a thread.
   *
   * All threads, however, must declare a "run" function, and handlers
   * for three types of SoDa::Command objects (GET, SET, and REPort)
   */
  class Thread : public Base, public Debug {
  public:
    
    /** 
     * @brief make the thread object.  Register it by name and
     * store a version string, so we can test it against the current
     * version of the SoDaServer.
     * @param oname The name of the thread.
     * @param version a string of the form "M.m.p" where 
     * M is the major version, m is minor, and p is patch. 
     * 
     * Invoking this thread should *always* leave the version parameter as 
     * the default.  This is a hack to get the actual version information from
     * the included file (used for the build) rather than from the shared 
     * library.
     */
    Thread(const std::string & oname, const std::string & version = std::string(SoDaRadio_VERSION));

    /**
     * @brief the creator of this thread may offer one or more mailboxes
     * to this object.  The thread may "subscribe" to the mailboxes as it
     * chooses.  
     * 
     * @param mbox_name The name of the mailbox.  This identifies the purpose
     * of the mailbox, and the thread should be able to infer the the "T" in 
     * the actual MultiMBox<T> type from this identifying string. 
     * @param mbox_p a pointer to a multimbox.  This should be cast to the 
     */
    virtual void subscribeToMailBox(const std::string & mbox_name, BaseMBox * mbox_p) {
    }

    /**
     * @brief the creator of this thread may offer one or more mailboxes
     * to this object.  The thread may "subscribe" to the mailboxes as it
     * chooses.  offerSubscriptionList is a convenience function that allows
     * a thread creator to present a list of mailboxes to which the thread
     * may subscribe. 
     * 
     * @param subscription_map The name/mailbox pairs. The name identifies the purpose
     * of the mailbox, and the thread should be able to infer the the "T" in 
     * the actual MultiMBox<T> type from this identifying string. 
     */
    void subscribeToMailBoxList(const MailBoxMap & subscription_map) {
      for(auto p : subscription_map) {
	subscribeToMailBox(p.first, p.second);
      }
    }

    void operator() () {
      // we woke up with a "start" or "join" call
      outerRun();
    }

    /**
     * Execute the thread's run loop. 
     */
    void start() {
      thread_ptr = std::unique_ptr<std::thread>(new std::thread(&SoDa::Thread::outerRun, this));
      //      thread_ptr = std::unique_ptr<std::thread>(new std::thread(*this));
    }

    /**
     * more properly "Wait for this thread to exit its run loop".
     * returns after the thread has received a STOP message.
     */
    void join() {
      thread_ptr->join(); 
    }

    /**
     * Each thread object must define its "run" loop.  This loop
     * exits only when the thread has received a STOP command on
     * one of its command mailboxes.
     */
    virtual void run() {
      std::cerr << "Thread " << getObjName() << " has no run method\n";
    }

    /**
     * Execute (dispatch) a message removed from the command stream to one
     * of the basic Command handler functions.
     *
     * This sequence appears in so many of the instances of Thread that it
     * was factored out. 
     * 
     * @param cmd the command message to be handled
     */
    void execCommand(Command * cmd);
    
    /**
     * optional method to handle "GET" commands -- commands that request a response
     */
    virtual void execGetCommand(Command * cmd) { (void) cmd; }

    /**
     * optional method to handle "SET" commands -- commands that set internal state in the object.
     */
    virtual void execSetCommand(Command * cmd) { (void) cmd; }

    /**
     * optional method that reports status or the result of some action. 
     */
    virtual void execRepCommand(Command * cmd) { (void) cmd; } 

    /**
     * optional method that performs cleanup -- may not delete. 
     */
    virtual void shutDown() {
      return; 
    }

    void sleep_ms(unsigned int milliseconds) {
      std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }
    
  private:
    /**
     * This is the actual thread object -- 
     */
    std::unique_ptr<std::thread> thread_ptr;
    
    /**
     * the run method that is called by the thread handler.
     * This method wraps the thread objects run loop in an exception
     * handler so that we can do something useful with it. 
     *
     * We call the subclass's "run" method inside a wrapper that can 
     * give us some limited information about what happened. 
     */
    void outerRun();
    
    static void sigsegHandler(int sig);
    
    void hookSigSeg();
  };

  /**
   * @brief if the pattern and key match, return true and set "mbox_ptr" to point
   * to the mailbox pointed to by "could_be_pointer".  Otherwise, just return false;
   * 
   * @param obj A SoDa object so we have someone to blame for an exception.
   * @param current_ptr if the mailbox has already been connected, then this is it, otherwise, we'll update the mailbox on a match.

   * @param pattern this is the mailbox name we're looking for
   * @param key this is the name of the  mailbox we're being offered
   * @param could_be_pointer this is the mailbox we're being offered. 
   * @return true if we find a match
   *
   */
  template<class T> bool connectMailBox(SoDa::Base * obj, 
					T * & current_ptr, 
					const std::string & pattern,
					const std::string & key, 
					SoDa::BaseMBox * could_be_pointer) {
    T * ret;
    if(pattern == key) {
      ret = dynamic_cast<T *>(could_be_pointer);
      if(ret == NULL) {
	throw SoDa::Exception(SoDa::Format("Bad mailbox pointer for mailbox named = [%0]\n") 
			      .addS(key), obj);
      }
      else {
	current_ptr = ret; 
	return true;
      }
    }
    return false;
  }
}


#endif
