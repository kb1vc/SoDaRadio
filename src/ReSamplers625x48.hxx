#pragma once
/*
  Copyright (c) 2012,2013,2014,2024 Matthew H. Reilly (kb1vc)
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

// OK... this is just for 48K to 625K interpolation. 
// fast, small, simple. 
#include "SoDaBase.hxx"

#include <complex>
#include "ReSampler.hxx"

namespace SoDa {
  class ReSample48to625;
  typedef std::shared_ptr<ReSample48to625> ReSample48to625Ptr;
  
  /**
   * Resampler for 48KHz to 625KHz data stream, built on rational ReSampler class
   */
  class ReSample48to625 : public SoDa::Base {
  public:
    /**
     * @brief Constructor
     * @param inbufsize the size of the input buffer, output buffer will be 625 * inbufsize / 48
     * @param gain multiply output by gain factor
     */
    ReSample48to625(unsigned int inbufsize, float gain = 1.0);
    ~ReSample48to625();

    static ReSample48to625Ptr make(unsigned int inbufsize, float gain = 1.0) {
      return std::make_shared<ReSample48to625>(inbufsize, gain); 
    }
    
    /**
     * @brief Perform the resampling on a complex float buffer
     * @param in input buffer
     * @param out output buffer 
     */
    void apply(std::complex<float> * in, std::complex<float> * out);
    /**
     * @brief Perform the resampling on a real float buffer
     * @param in input buffer
     * @param out output buffer 
     */
    void apply(float * in, float * out);

  private:
    unsigned int N, MN;

    // The resampler objects
    SoDa::ReSampler * rs54a, * rs54b, * rs53, * rs51;

    // the buffers
    std::complex<float> * inter1, *inter2; 
    float * finter1, *finter2; 
    
    float gain; ///< the gain to be applied to the final transform result. 
  }; 

  /**
   * Resampler for 625KHz to 48KHz data stream, built on rational ReSampler class
   */
  class ReSample625to48 : public SoDa::Base {
  public:
    /**
     * @brief Constructor
     * @param inbufsize the size of the input buffer, output buffer will be 48 * inbufsize / 625
     * @param _gain multiply output by gain factor
     */
    ReSample625to48(unsigned int inbufsize, float _gain = 1.0);
    ~ReSample625to48();

    /**
     * @brief Perform the resampling on a complex float buffer
     * @param in input buffer
     * @param out output buffer 
     */
    void apply(std::complex<float> * in, std::complex<float> * out);
    /**
     * @brief Perform the resampling on a real float buffer
     * @param in input buffer
     * @param out output buffer 
     */
    void apply(float * in, float * out);


  private:
    unsigned int N, MN;

    // The resampler objects
    SoDa::ReSampler * rs45a, * rs45b, * rs35, * rs15;

    // the buffers
    std::complex<float> * inter1, *inter2; 
    float * finter1, *finter2;

    float gain; ///< the gain to be applied to the final transform result. 
  }; 

}

