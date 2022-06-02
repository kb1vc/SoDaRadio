/*
Copyright (c) 2012,2013,2014,2022 Matthew H. Reilly (kb1vc)
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

#include <string>
#include <SoDa/Format.hxx>
#include "SoDaBase.hxx"

 /**
  * @file Exception.hxx
  * The Baseclass for all SoDa exceptions
  *
  * @brief SoDa exception class
  *
  * @author Matt Reilly (kb1vc)
  *
  */

namespace SoDa {
  /**
   * The SoDa Exception class
   *
   * @class SoDa::Exception
   *
   * Wherever possible, objects reporting exceptions should signal a subclass of the
   * SoDa::Exception class. 
   */
  class Exception : public std::runtime_error { 
  public:
    /**
     * The constructor
     *
     * @param _reason an informative string reporting the cause of the error
     * @param obj  a pointer to the SoDaBase object that triggered the exception (if any).
     */
    Exception(const std::string & _reason, Base * obj = NULL) 
      : std::runtime_error(SoDa::Format("SoDa Object [%0] threw exception [%1]\n")
			   .addS((obj == NULL) ? obj->getObjName()
				 : "Unknown")
			   .addS(_reason)
			   .str())
    {
    }
    
    /**
     * The constructor
     *
     * @param _reason an informative string reporting the cause of the error
     * @param obj  a pointer to the SoDa::Base object that triggered the exception (if any).
     */
    Exception(const char * _reason, Base * obj) 
      : std::runtime_error(SoDa::Format("SoDa Object [%0] threw exception [%1]\n")
			   .addS((obj == NULL) ? obj->getObjName()
				 : "Unknown")
			   .addS(_reason)
			   .str())
    {
    }

    /**
     * The constructor
     *
     * @param _reason a SoDa::Format object with an explanation of the error
     * @param obj  a pointer to the SoDa::Base object that triggered the exception (if any).
     */
    Exception(const SoDa::Format & _reason, Base * obj)
      : std::runtime_error(SoDa::Format("SoDa Object [%0] threw exception [%1]\n")
			   .addS((obj == NULL) ? obj->getObjName()
				 : "Unknown")
			   .addS(_reason.str())
			   .str())
    {      
    }
  };
}

