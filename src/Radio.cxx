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

#include "CtrlBase.hxx"
#include "RXBase.hxx"
#include "TXBase.hxx"
#include "RadioRegistry.hxx"

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
     * @param params a parameter object that supplies any unit with initial 
     * settings. 
     */
    Radio(const std::string & RadioModel, SoDa::Params & params) {
      auto model_p = RadioRegistry::getRegistry()->getModel(RadioModel); 
      
      ctrl = model_p->makeCtrl(params);
      rx = model_p->makeRX(params);
      tx = model_p->makeTX(params);
    }

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
       * The constructor
       *
       * @param _reason a SoDa::Format object with an explanation of the error
       * @param obj  a pointer to the SoDa::Base object that triggered the exception (if any).
       */
      Exception(const SoDa::Format & _reason, Base * obj) {
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
      Base * thrower; ///< who caused the exception, if anyone? 
      std::string reason; ///< what was the cause of the exception?

      std::string message; ///< the reason together with the owner. 
    };

    
  protected:
    std::shared_ptr<CtrlImp> ctrl; 
    std::shared_ptr<RXImp> rx; 
    std::shared_ptr<TXImp> tx; 
  };
}
