#pragma once
/*
Copyright (c) 2022, Matthew H. Reilly (kb1vc)
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
 *   @file Radio.hxx @brief Base class describing a radio. 
 * 
 * 
 *   @author M. H. Reilly (kb1vc)
 *   @date   June 2022
 */

#include <string>
#include <stdexcept>
#include <SoDa/Format.hxx>
#include "SoDaBase.hxx"

namespace SoDa {
  /**
   * @class Radio
   * @brief Not much to see here.  This class creates the control, tx, and rx
   * objects/threads. 
   * 
   * The three classes correspond to subclasses of CtrlBase, RXBase, and TXBase, and
   * methods to make them are supplied from the RadioRegistry. 
   * 
   * For radios that do not provide a transmitter, the TXImp class should be specified
   * as TXNull.
   */
  class Radio {
  public:
    /**
     * @brief We often need to specify whether we're coming or going. 
     */
    enum TXRX { RX, TX }; 

    /**
     * @brief the constructor -- builds each widget
     *  
     * @param name a descriptive name for this radio
     * settings. 
     */
    Radio(const std::string & name) : name(name) { }

    virtual void init() = 0;
    
    /**
     * @brief EVERY radio must supply a few parameters to all the
     * other units. Most important are the TX and RX sample rates
     */
    virtual float getRXSampleRate() = 0;
    virtual float getTXSampleRate() = 0; 
    virtual void cleanUp() {}; 
    
    const std::string & getName() const { return name; }
  private:
    std::string name; 

  public:
    /**
     * The SoDa Exception class
     *
     * @class SoDa::Radio::Exception
     *
     * We put this under the Radio class definition because we needed it to be 
     * in a namespace below "SoDa" and this looked like the best place to put it. 
     * sigh. 
     *
     * Wherever possible, objects reporting exceptions should signal a subclass of the
     * SoDa::Exception class. 
     */
    class Exception : public std::runtime_error { 
    public:
      /**
       * The constructor
       *
       * @param reason an informative string reporting the cause of the error
       * @param obj  a pointer to the SoDaBase object that triggered the exception (if any).
       */
      Exception(const std::string & reason, Base * obj = nullptr) :
	std::runtime_error(SoDa::Format("SoDaRadio %0 exception: %1")
			   .addS((obj == nullptr) ? std::string("") : obj->getObjName())
			   .addS(reason)
			   .str()) {}
    };
  };
}
