/*
Copyright (c) 2012,2013,2014 Matthew H. Reilly (kb1vc)
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

#ifndef SODA_BASE_HDR
#define SODA_BASE_HDR

#include "Command.hxx"
#include "MultiMBox.hxx"
#include "Debug.hxx"
#include <complex>
#include <string>
#include <boost/thread.hpp>
#include <boost/format.hpp>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

 /**
  * @file SoDaBase.hxx
  * The Baseclass for all SoDa objects, and useful commonly used classes.
  *
  * @brief SoDa base and commonly used classes
  *
  * @author Matt Reilly (kb1vc)
  *
  */

namespace SoDa {

  /**
   * The Buffer Class
   *
   * @class SoDaBuf
   *
   * This is used to carry blocks of complex or real single precision
   * floating point samples on the message ring. A SoDaBuf can carry
   * either complex or real values so that buffers for either use
   * can be allocated from the same storage pool.  
   *
   */
  class SoDaBuf : public MBoxMessage {
  public:
    /**
     * constructor: Allocate a complex/real buffer of complex data values
     *
     * @param _size the maximum number of single presion complex values the buffer can hold. 
     */
    SoDaBuf(unsigned int _size) {
      maxlen = _size;
      len = maxlen;
      dat = new std::complex<float>[_size];
      // we also overlay a floating point buffer in the same space.
      fdat = (float *) dat; 
      maxflen = maxlen * 2;
      flen = len * 2; 
    }

    bool copy(SoDaBuf * src) {
      if(maxlen >= src->maxlen) {
	flen = src->flen;
	memcpy(fdat, src->fdat, sizeof(float) * flen);
	return true; 
      }
      else {
	return false; 
      }
    }
    
    //! Return the number of complex float values in this buffer
    unsigned int getComplexLen() { return len; }
    //! Return the maximum number of complex float values that this buffer can hold
    unsigned int getComplexMaxLen() { return maxlen; }
    
    //! Return the number of float values in this buffer
    unsigned int getFloatLen() { return flen; }
    //! Return the maximum number of float values that this buffer can hold
    unsigned int getFloatMaxLen() { return maxflen; }

    /**
     * set the length of the buffer (in number of complex floats.)
     * @param nl new length
     */
    bool setComplexLen(unsigned int nl) {
      if(nl > maxlen) return false; 
      len = nl;
      return true; 
    }
    
    /**
     * set the length of the buffer (in number of floats.)
     * @param nl new length
     */
    bool setFloatLen(unsigned int nl) {
      if(nl > maxflen) return false; 
      flen = nl;
      return true; 
    }

    /**
     * Return a pointer to the storage buffer of complex floats
     */
    std::complex<float> * getComplexBuf() { return dat; }
    /**
     * Return a pointer to the storage buffer of floats
     */
    float * getFloatBuf() { return fdat; }
    
  private:
    std::complex<float> * dat; ///< the storage array (complex version) Storage is common to both types
    float * fdat;              ///< the storage array (REAL version)  Storage is common to both types
    unsigned int maxflen;      ///< the maximum length in terms of real floats
    unsigned int flen;         ///< the current length of the buffer in REAL fp values
    unsigned int maxlen;       ///< the maximum length in terms of complex floats
    unsigned int len;          ///< the current length of the buffer in complex fp values
  };

  /**
   * Mailboxes that carry commands only are of type CmdMBox
   */
  typedef MultiMBox<Command> CmdMBox;
  /**
   * Mailboxes that carry float or complex data are of type DatMBox
   */ 
  typedef MultiMBox<SoDaBuf> DatMBox;


  /**
   * The SoDa Base class
   *
   * @class SoDaBase
   *
   * All persistent soda objects of any size are given a NAME so that
   * the SoDaException class can show who is complaining for a given
   * exception. 
   */
  class SoDaBase {
  public:
    /**
     * The constructor -- pass a name for the object.
     *
     * @param oname The name of the object. Uniqueness is helpful, but not necessary.
     *              The first object with a given name will be entered into the directory
     *              Objects can be retrieved from the directory by name with the findSoDaObject function.
     */
    SoDaBase(const std::string & oname);

    /**
     * get the name of this object
     * @return the name of this object.
     */
    std::string & getObjName() { return objname; }

    /**
     * find a SoDa Object by name.
     *
     * @param oname a string that names the object
     * @return a pointer to the SoDaBase object (NULL if the name isn't found)
     */
    SoDaBase * findSoDaObject(const std::string & oname); 
  private:
    std::string objname; ///< the name of the object
    
    static std::map<std::string, SoDaBase * > ObjectDirectory; ///< a class member -- directory of all registered objects.
  };

  /**
   * The SoDa Exception class
   *
   * @class SoDaException
   *
   * Wherever possible, objects reporting exceptions should signal a subclass of the
   * SoDaException class. 
   */
  class SoDaException { 
  public:
    /**
     * The constructor
     *
     * @param _reason an informative string reporting the cause of the error
     * @param obj  a pointer to the SoDaBase object that triggered the exception (if any).
     */
    SoDaException(const std::string & _reason, SoDaBase * obj = NULL) {
      thrower = obj;
      reason = _reason; 
    }
    /**
     * The constructor
     *
     * @param _reason an informative string reporting the cause of the error
     * @param obj  a pointer to the SoDaBase object that triggered the exception (if any).
     */
    SoDaException(const char * _reason, SoDaBase * obj) {
      thrower = obj;
      reason = std::string(_reason); 
    }


    /**
     * Create a string that explains this exception.
     * @return the exception string
     */
    std::string toString() {
      std::string ret; 
      if(thrower != NULL) {
	ret = (boost::format("SoDa Object [%s] threw exception [%s]\n") % thrower->getObjName() % reason).str();
      }
      else {
	ret = (boost::format("Unknown SoDa Object threw exception [%s]\n") % reason).str();
      }

      return ret;
    }

    /**
     * Create a string that explains this exception.
     * @return a pointer to a c_str buffer (suitable for generic exception handling.)
     */
    const char * what() {
      return toString().c_str();
    }
  private:
    SoDaBase * thrower; ///< who caused the exception, if anyone? 
    std::string reason; ///< what was the cause of the exception? 
  };


  /**
   * The Thread baseclass for all SoDa thread objects.
   *
   * @class SoDaThread
   *
   * The SoDaThread baseclass simplifies interaction with threads.
   * Since SoDa thread objects aren't created and destroyed very often,
   * we don't need many bells and whistles here.  Just the ability to
   * start a thread a join a thread.
   *
   * All threads, however, must declare a "run" function, and handlers
   * for three types of SoDa::Command objects (GET, SET, and REPort)
   */
  class SoDaThread : public SoDaBase, public Debug {
  public:
    SoDaThread(const std::string & oname) : SoDaBase(oname), Debug(oname) {
      th = NULL; 
    }
    /**
     * Execute the threads run loop. 
     */
    void start() {
      if(th != NULL) return;
      th = new boost::thread(&SoDaThread::outerRun, this);
    }

    /**
     * more properly "Wait for this thread to exit its run loop".
     * returns after the thread has received a STOP message.
     */
    void join() {
      th->join(); 
    }

    /**
     * Each thread object must define its "run" loop.  This loop
     * exits only when the thread has received a STOP command on
     * one of its command mailboxes.
     */
    virtual void run() = 0;

    /**
     * Execute (dispatch) a message removed from the command stream to one
     * of the basic Command handler functions.
     *
     * This sequence appears in so many of the instances of SoDaThread that it
     * was factored out. 
     * 
     * @param cmd the command message to be handled
     */
    void execCommand(Command * cmd) 
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

    /**
     * optional method to handle "GET" commands -- commands that request a response
     */
    virtual void execGetCommand(Command * cmd) { }

    /**
     * optional method to handle "SET" commands -- commands that set internal state in the object.
     */
    virtual void execSetCommand(Command * cmd) { }

    /**
     * optional method that reports status or the result of some action. 
     */
    virtual void execRepCommand(Command * cmd) { }; 


  private:
    boost::thread * th;


    /**
     * the run method that is called by the boost thread handler.
     * This method wraps the thread objects run loop in an exception
     * handler so that we can do something useful with it. 
     */
    void outerRun() {
      pid_t tid;
      tid = syscall(SYS_gettid);
      try {
	run(); 
      }
      catch (SoDaException * exc) {
	std::cerr << exc->toString() << std::endl;
      }
    }
  }; 
}

#endif
