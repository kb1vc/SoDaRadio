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
 *   @file RadioModel.hxx 

 *@brief Each implementation of the handler processes (control, rx, tx) for a 
 * model of a radio will create a subclass of RadioModel. RadioModel is a factory
 * that produces the specific handler objects for a specified model. 
 * 
 * So, for instance, the USRP RadioModel would be created with a key "USRP" and
 * it would produce a USRPCtrl, USRPRX, and USRPTX object on request. 
 * 
 *   @author M. H. Reilly (kb1vc)
 *   @date   June 2022
 */

#include "CtrlBase.hxx"
#include "RXBase.hxx"
#include "TXBase.hxx"

namespace SoDa {
  /**
   * @class RadioModel
   */
  template<typename CtrlImp, typename RXImp, typename TXImp>
  class RadioModel {
  public:
    RadioModel(const std::string & name) {
      auto reg = RadioRegistry::getRegistry(); 
      reg->registerModel(name, this);
    }
    
    void makeRadio(SoDa::Params & params, 
		   std::shared_ptr<CtrlImp> & ctrl, 
		   std::shared_ptr<RXImp> & rx, 
		   std::shared_ptr<TXImp> & tx) {
      auto * imp = new CtrlImp(params); 
      ctrl = std::shared_ptr<CtrlImp>(imp); 
      rx = std::shared_ptr<RXImp>(new RXImp(params, ctrl));
      tx = std::shared_ptr<TXImp>(new TXImp(params, ctrl));       
    }
  };
}
