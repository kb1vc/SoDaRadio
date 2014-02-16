/*
  Copyright (c) 2012,2013,2014 Matthew H. Reilly (kb1vc)
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

#ifndef QUADOSC_HDR
#define QUADOSC_HDR
#include <complex>
#include <math.h>


namespace SoDa {
  /**
   * @brief sin/cos oscillator to drive TX signal chain
   */
  class QuadratureOscillator {
  public:
    /**
     * Constructor
     */
    QuadratureOscillator() {
      idx = 0;
      ang = 0.0;
      // default IF freq is SampRate/1000;
      setPhaseIncr(2.0 * M_PI / 1000.0);
    }

    /**
     * @brief step the oscillator and produce a complex double result
     * @result cos(ang), -sin(ang)
     *
     * Note the use of sincos if we're building for a platform that
     * supports it.  This is a bit faster than separate sin and cos calls.
     *
     *
     * The original scheme also did some near-angle approximation
     * stuff but it really didn't help that much with computation
     * time, as the oscillators account for such a small part of the
     * total run time. 
     */
    std::complex<double> stepOscCD() {
      double s,c;
#if HAVE_SINCOS
      sincos(ang, &s, &c);
#else
      s = sin(ang); c = cos(ang); 
#endif	
      ang = (ang > M_PI) ? (ang - (2.0 * M_PI)) : ang; 
      idx = 0; 

      idx++; 
      ang += phase_incr;
      s = -s; 
      std::complex<double> ret(c, s);
      return ret; 
    }
    
    /**
     * @brief step the oscillator and produce a complex float result
     * @result cos(ang), -sin(ang)
     *
     * This is a wrapper for stepOscCD
     */
    std::complex<float> stepOscCF() {
      std::complex<double> dv = stepOscCD();
      std::complex<float> fv((float)dv.real(), (float)dv.imag());
      return fv; 
    }

    /**
     * @brief step the oscillator and produce a real double result
     * @result cos(ang)
     *
     * This is a wrapper for stepOscCD
     */
    double stepOscD() {
      std::complex<double> dv = stepOscCD(); 
      return dv.real(); 
    }

    /**
     * @brief set the phase increment per step for the oscillator (1/freq)
     * @param _pi the phase increment
     */
    void setPhaseIncr(double _pi) {
      phase_incr = _pi;
    }
    
  private:
    double phase_incr;
    double ang; 
    int idx; 
  };
}

#endif
