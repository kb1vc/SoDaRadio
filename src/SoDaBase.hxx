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

#include "Command.hxx"
#include "Buffer.hxx"
#include "MultiMBox.hxx"
#include "Debug.hxx"
#include <complex>
#include <string>
//#include <sys/types.h>
//#include <unistd.h>
//#include <sys/syscall.h>
#include <SoDa/Format.hxx>

//extern "C" {
//#include <signal.h>
//#include <stdio.h>
//#include <unistd.h>
//}

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

}

