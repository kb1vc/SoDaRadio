/*
  Copyright (c) 2014, Matthew H. Reilly (kb1vc)
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
#ifndef MEDIAN_FILTER_HDR
#define MEDIAN_FILTER_HDR


 ///
 ///  @file MedianFilter.hxx
 ///  @brief This is a simple median filter widget 
 ///
 ///  @author M. H. Reilly (kb1vc)
 ///  @date   February 2014
 ///

#include <fstream>
#include <complex>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fftw3.h>
namespace SoDa {
  /// 3 point Median Filter class -- templatized (!)
  template <typename T> class MedianFilter3 {
  public:
    

    /// constructor
    /// no parameters
    MedianFilter3() {
      a = b = (T) (0.0);
    }

    /**
     * @brief Apply the 3 sample median filter to an input vector
     *
     * @param inbuf the input buffer
     * @param outbuf the output buffer -- the two buffers may overlap, 
     *        though inbuf must be <= outbuf
     * @param len the length of the input and output buffers
     * @param outgain  boost the output, if necessary
     */
    unsigned int apply(T * inbuf, T * outbuf, unsigned int len, float outgain = 1.0)
    {
      unsigned int i;
      for(i = 0; i < len; i++) {
	outbuf[i] = outgain * findMedian(inbuf[i]);
      }
    }
      
  private:
    /**
     * @brief find the median of the supplied value vs. the previous two supplied values
     *
     * @param v the "new" value in the sequence
     * @return the median value of the last 3 values in a sequence
     */
    T findMedian(T v) {
      T ret;
      if(v > a) {
	if(v <  b) ret = v; 
	else ret = (a > b) ? a : b;
      }
      else {
	if(v < b) ret = (a > b) ? b : a;
	else ret = v; 
      }
      a = b;
      b = v;

      return ret;
    }
    
    T a; ///< storage for the value-before-last
    T b; ///< storage for the previous value
  };
}

#endif
