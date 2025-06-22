/*
Copyright (c) 2012,2013,2014,2025 Matthew H. Reilly (kb1vc)
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

#include "Command.hxx"
#include "Debug.hxx"
#include <complex>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

#include <SoDa/Format.hxx>
#include <SoDa/MailBox.hxx>

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
  class Buf;
  typedef std::shared_ptr<Buf> BufPtr;

  class FBuf;
  typedef std::shared_ptr<FBuf> FBufPtr; 
  
  class CBuf;
  typedef std::shared_ptr<CBuf> CBufPtr; 
  
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
  class Buf {
  protected:
    /**
     * constructor: Allocate a complex/real buffer of complex data values
     *
     * @param size the maximum number of single presion complex values the buffer can hold. 
     */
    Buf(unsigned int size);

  public:

    unsigned int size();
      
    void copy(BufPtr src);
    
    /**
     * set the length of the buffer (in number of complex floats.)
     * @param nl new length
     */
    virtual bool setComplexLen(unsigned int nl);
    
    /**
     * set the length of the buffer (in number of floats.)
     * @param nl new length
     */
    virtual bool setFloatLen(unsigned int nl);


  protected:
    std::vector<std::complex<float>> cdat; 
    std::vector<float> fdat;

    unsigned int r_size; 
  };

  /**
   * @class FBuf
   * @brief Buf specialized buffer for floats only.
   */ 
  class FBuf : public Buf {
  public:
    FBuf(unsigned int size);

    static FBufPtr make(unsigned int _size);    
    
    bool setComplexLen(unsigned int nl);

    std::vector<std::complex<float>> & getComplexBuf();

    std::vector<float> & getBuf() { return fdat; }    

    float & operator[](size_t index);
  }; 


  /**
   * @class CBuf 
   * @brief Buf specialized buffer for floats only.   
   */ 
  class CBuf : public Buf {
  public:
    CBuf(unsigned int size);

    static CBufPtr make(unsigned int _size);        

    bool setFloatLen(unsigned int nl);

    std::vector<float> & getFloatBuf();
    
    std::vector<std::complex<float>> & getBuf() { return cdat; }

    std::complex<float> & operator[](size_t index);
  }; 
  
  /**
   * Mailboxes that carry commands only are of type CmdMBox
   */
  typedef SoDa::MailBox<CommandPtr> CmdMBox;
  typedef std::shared_ptr<CmdMBox> CmdMBoxPtr;
  /**
   * Mailboxes that carry float or complex data are of type DatMBox
   */ 
  typedef SoDa::MailBox<FBufPtr> FDatMBox;
  typedef std::shared_ptr<FDatMBox> FDatMBoxPtr;

  /**
   * Mailboxes that carry float or complex data are of type DatMBox
   */
  typedef SoDa::MailBox<CBufPtr> CDatMBox;
  typedef std::shared_ptr<CDatMBox> CDatMBoxPtr;
  
  /**
   * The SoDa Base class
   *
   * @class Base
   *
   * All persistent soda objects of any size are given a NAME so that
   * the SoDa::Exception class can show who is complaining for a given
   * exception. 
   */
  class Base;
  typedef std::shared_ptr<Base> BasePtr;
  
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

    void registerSelf(BasePtr ptr);
    
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
    BasePtr findSoDaObject(const std::string & oname); 

    /**
     * Get a time stamp in nS resolution that monotonically increases
     * and that is very inexpensive (typically < 100nS). 
     * 
     * @return a monotonically increasing timestamp in nS since an arbitrary time in the past.
     */
    double getTime(); 

    /**
     * @brief get a pointer to myself. 
     *
     */
    BasePtr getSelfPtr() { return self.lock(); }
    
  private:
    std::string objname; ///< the name of the object
    std::weak_ptr<Base> self;
    
    static bool first_time; ///< have we seen the first call to getTime? 
    static double base_first_time; ///< time of first call to getTime from anyone. 

    static std::map<std::string, BasePtr > object_directory; ///< a class member -- directory of all registered objects.
  };

  namespace Radio {
    /**
     * The SoDa Exception class
     *
     * @class SoDa::Radio::Exception
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
      Exception(const std::string & _reason, BasePtr obj = NULL) 
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
      Exception(const char * _reason, BasePtr obj) {
	thrower = obj;
	reason = std::string(_reason); 
      }

      /**
       * The constructor
       *
       * @param _reason a SoDa::Format object with an explanation of the error
       * @param obj  a pointer to the SoDa::Base object that triggered the exception (if any).
       */
      Exception(const SoDa::Format & _reason, BasePtr obj) {
	thrower = obj;
	reason = _reason.str(); 
      }

      /**
       * Create a string that explains this exception.
       * @return the exception string
       */
      const std::string & toString() {
	if(thrower != NULL) {
	  message = SoDa::Format("SoDa Object [%0] threw exception [%1]\n")
	    .addS(thrower->getObjName())
	    .addS(reason)
	    .str();
	}
	else {
	  message = SoDa::Format("Unknown SoDa Object threw exception [%0]\n")
	    .addS(reason)
	    .str();
	}

	return message;
      }

      /**
       * Create a string that explains this exception.
       * @return a pointer to a c_str buffer (suitable for generic exception handling.)
       */
      const char * what() {
	toString();
	return message.c_str();
      }
    private:
      BasePtr thrower; ///< who caused the exception, if anyone? 
      std::string reason; ///< what was the cause of the exception?

      std::string message; ///< the reason together with the owner. 
    };
    
    
  }
}
