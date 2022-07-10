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
 *   @file RadioRegistry.hxx
 *   @brief key-value table that holds functions that produce radios.
 * 
 * 
 *   @author M. H. Reilly (kb1vc)
 *   @date   July 2022
 */


#include <map>
#include <functional>
#include <string>
#include "Radio.hxx"
#include "Params.hxx"

namespace SoDa {

  /**
   *   @class RadioRegistry
   *  
   * This registry contains functions that produce SoDa::Radio objects. 
   * 
   */
   class RadioRegistry {
   public:
     RadioRegistry(); 

     typedef std::function<Radio*(Params &)> RadioBuilderFunc;
     
     void add(const std::string & name, RadioBuilderFunc builder);

     RadioBuilderFunc & get(const std::string & name);

     Radio * make(const std::string & name, Params & parms);
     
     std::string supportedRadios(); 

     class ModelNotFound : public Radio::Exception {
     public:
       ModelNotFound(const std::string & model_name, const std::string & should_be) : 
	 Radio::Exception(SoDa::Format("Radio model name %0 not found. Supported radios are\n\t%1\n")
			  .addS(model_name).addS(should_be).str()) {}
     };
     
   protected:
     std::map<std::string, RadioBuilderFunc> builders; 
   };
}
