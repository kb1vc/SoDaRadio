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

extern "C" {
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
}

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
   * @class SoDa::Buf
   *
   * This is used to carry blocks of complex or real single precision
   * floating point samples on the message ring. A SoDa::Buf can carry
   * either complex or real values so that buffers for either use
   * can be allocated from the same storage pool.  
   *
   */
  class Buf : public MBoxMessage {
  public:
    /**
     * constructor: Allocate a complex/real buffer of complex data values
     *
     * @param _size the maximum number of single presion complex values the buffer can hold. 
     */
    Buf(unsigned int _size) {
      maxlen = _size;
      len = maxlen;
      dat = new std::complex<float>[_size];
      // we also overlay a floating point buffer in the same space.
      fdat = (float *) dat; 
      maxflen = maxlen * 2;
      flen = len * 2; 
    }

    bool copy(Buf * src) {
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
  typedef MultiMBox<Buf> DatMBox;


  /**
   * The SoDa Base class
   *
   * @class Base
   *
   * All persistent soda objects of any size are given a NAME so that
   * the SoDa::Exception class can show who is complaining for a given
   * exception. 
   */
  class Base {
  public:
    /**
     * The constructor -- pass a name for the object.
     *
     * @param oname The name of the object. Uniqueness is helpful, but not necessary.
     *              The first object with a given name will be entered into the directory
     *              Objects can be retrieved from the directory by name with the findSoDaObject function.
     */
    Base(const std::string & oname);

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
    Base * findSoDaObject(const std::string & oname); 

    /**
     * Get a time stamp in nS resolution that monotonically increases
     * and that is very inexpensive (typically < 100nS). 
     * 
     * @return a monotonically increasing timestamp in nS since an arbitrary time in the past.
     */
    double getTime(); 

  private:
    std::string objname; ///< the name of the object

    static bool first_time; ///< have we seen the first call to getTime? 
    static double base_first_time; ///< time of first call to getTime from anyone. 

    static std::map<std::string, Base * > ObjectDirectory; ///< a class member -- directory of all registered objects.
  };

  /**
   * The SoDa Exception class
   *
   * @class SoDa::Exception
   *
   * Wherever possible, objects reporting exceptions should signal a subclass of the
   * SoDa::Exception class. 
   */
  class Exception { 
  public:
    /**
     * The constructor
     *
     * @param _reason an informative string reporting the cause of the error
     * @param obj  a pointer to the SoDaBase object that triggered the exception (if any).
     */
    Exception(const std::string & _reason, Base * obj = NULL) 
    {
      thrower = obj;
      reason = _reason; 
    }
    /**
     * The constructor
     *
     * @param _reason an informative string reporting the cause of the error
     * @param obj  a pointer to the SoDa::Base object that triggered the exception (if any).
     */
    Exception(const char * _reason, Base * obj) {
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
    Base * thrower; ///< who caused the exception, if anyone? 
    std::string reason; ///< what was the cause of the exception? 
  };
}

#endif
