/*
  Copyright (c) 2014 Matthew H. Reilly (kb1vc)
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
#ifndef DCBLOCK_HDR
#define DCBLOCK_HDR

#include <fstream>
#include <complex>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fftw3.h>
#include "SoDaBase.hxx"
namespace SoDa {
  /**
   * @class DCBlock
   *
   * This unit implements an IIR filter to remove the DC bias from
   * a quadrature or scalar signal. (It is templated, and the
   * identified type needs only to support multiplication by a float
   * and addition. 
   *
   * The algorithm follows a scheme described by Richard G. Lyons in his
   * post at
   * http://www.embedded.com/design/configurable-systems/4007653/DSP-Tricks-DC-Removal
   *
   * it operates on an input buffer, and fills an output buffer supplied as
   * arguments to the apply() method.
   *
   * It implements H(z) = (1 - z^{-1}) / (1 - alpha * z^{-1})
   * or
   *     y(n) = d(n) - d(n - 1)
   *     d(n) = x(n) + alpha * d(n - 1)
   */
  template <typename Tv, typename Ta> class DCBlock {
  public:
    /**
     * constructor -- initialize the filter <alpha> factor and the output
     *
     * @param _alpha As _alpha approaches 1.0, the width of the filter becomes narrower. 
     */
    DCBlock(Ta _alpha) { 
      alpha = _alpha;
      Dm1 = Tv(0.0);
    }

    /**
     * Perform a DC block
     *
     * Use form (d) from the Lyons paper
     *
     * @param inbuf input buffer of type T
     * @param outbuf output buffer of type T -- may be the same as inbuf
     * @param len length of the buffer.
     */
    unsigned int apply(Tv * inbuf, Tv * outbuf, int len) {
      Tv D0;
      int i;
      std::cerr << Dm1 << std::endl;
      for(i = 0; i < len; i++) {
	D0 = inbuf[i] + alpha * Dm1;
	outbuf[i] = D0 - Dm1;
	Dm1 = D0; 
      }
    }

  public:
    Tv Dm1; ///< the delayed input
    Ta alpha; 
  };
}

#endif
