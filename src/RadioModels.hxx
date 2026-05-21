#pragma once
/*
Copyright (c) 2026 Matthew H. Reilly (kb1vc)
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
 *  @file RadioModels.hxx
 *
 * @brief This class provides a mechanism for starting the threads for a hardware
 * model using only the name of the model.  Each new description (of type SoDa::Radio)
 * registers itself in the RadioModels.cxx file. 
 *
 * The class provides a single static method that returns a pointer to a Radio
 * 
 *  @author M. H. Reilly (kb1vc)
 *  @date   May 2026
 */

#include "Radio.hxx"
#include <string>
#include <vector>
#include <map>

namespace SoDa {
  class RadioModels {
  private:
    /**
     * @brief an actual RadioModels object should never be created. 
     */ 
    RadioModels();

  public:
    /**
     * @brief create a Radio unit.
     *
     * @param hardware_name the name of the radio type (e.g. USRP, Pluto, RTLSDR)
     * @param params initialization parameters
     * @return a pointer to a Radio model, or nullptr if there is no model
     * registered with @c hardware_name.
     */
    static RadioPtr make(const std::string & hardware_name,
			 Params * params);

    static std::vector<std::string> getModels();

  private:
    /**
     * @brief this initializes the model map. When a new model is written, add the
     * model name and a pointer to the model's static RadioPtr make method to the
     * model map. 
     */ 
    static void init(); 
    
    static bool init_complete;
    
    static std::map<std::string, std::function<RadioPtr(Params * params)>> model_map; 
  };
}



