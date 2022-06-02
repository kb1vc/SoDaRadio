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
#include "MultiMBox.hxx"
#include <complex>
#include <vector>

 /**
  * @file Buffer.hxx
  * Simple buffer types for complex and real floats
  * mailboxes. 
  *
  * @brief SoDa buffer class
  *
  * @author Matt Reilly (kb1vc)
  *
  */

namespace SoDa {

  /**
   * The Buffer Class
   *
   * @class SoDa::Buffer
   *
   * This is used to carry blocks of complex or real single precision
   * floating point samples on the message ring. A SoDa::Buf can carry
   * either complex or real values so that buffers for either use
   * can be allocated from the same storage pool.  
   *
   */
  template<typename T> class Buffer : public MBoxMessage {
  public:
    /**
     * constructor: Allocate a complex/real buffer of complex data values
     *
     * @param _size the maximum number of single presion complex values the buffer can hold. 
     */
    Buffer(unsigned int _size) {
      vec_p = new std::vector<T>(_size); 
      len = _size; 
    }

    std::vector<T> & getVec() { return *vec_p; }
  private:
    std::vector<T> * vec_p; 
    unsigned int len;          ///< the current length of the buffer 
  };

  /**
   * float, and complex float buffers
   */
  typedef Buffer<std::complex<float>> CFBuffer;
  typedef Buffer<float> FBuffer;  
  /**
   * Mailboxes that carry commands only are of type CmdMBox
   */
  typedef MultiMBox<Command> CmdMBox;
  /**
   * Mailboxes that carry float or complex data are of type DatMBox
   */ 
  typedef MultiMBox<CFBuffer> CFMBox;
  typedef MultiMBox<FBuffer> FMBox;  

}
