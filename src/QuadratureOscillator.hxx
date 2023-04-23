/*
  Copyright (c) 2012,2013,2014,2023 Matthew H. Reilly (kb1vc)
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
      last = std::complex<double>(1.0,0.0);
      ejw = std::complex<double>(1.0,0.0);

      // default IF freq is SampRate/1000;
      setPhaseIncr(2.0 * M_PI / 1000.0);
    }

    /**
     * @brief step the oscillator and produce a complex double result
     * @result cos(ang), -sin(ang)
     *
     * Note the use of sincos if we're building for linux.  This is
     * a bit faster than separate sin and cos calls.
     *
     * The original scheme also did some near-angle approximation
     * stuff but it really didn't help that much with computation
     * time, as the oscillators account for such a small part of the
     * total run time. 
     *
     * (notes from March, 2018) 
     * Well, no, the oscillators account for about 30% of the run time
     * when the compiler optimizations are turned on.  Changing from 
     * sincos to a complex multiply based scheme eliminates the 
     * NCO from the list of "expensive" functions. 
     * 
     * The code still retains the choice, if it is necessary for some 
     * reason to use the sincos scheme, but by default, the NCO will be
     * based on a complex multiply.
     */
    
    std::complex<double> stepOscCD() {
#ifdef USE_SINCOS_NCO
      return stepOscCD_sincos();
#else
      return stepOscCD_complex();      
#endif    
    }    

    std::complex<double> stepOscCD_sincos() {
      double s,c;
#  if __linux__
      sincos(ang, &s, &c);
#  else
      s = sin(ang); c = cos(ang); 
#  endif	
      ang = (ang > M_PI) ? (ang - (2.0 * M_PI)) : ang; 
      idx = 0; 

      idx++; 
      ang += phase_incr;
      s = -s; 
      std::complex<double> ret(c, s);
      return ret; 
    }

    std::complex<double> stepOscCD_complex() {
      std::complex<double> nval(0.0,0.0);
      idx++;
      nval = last * ejw;     
      if(idx > 512) {
	idx = 0;
	// keep the magnitude from growing above 1.
	// We should probably measure this, as it will manifest
	// itself as a kind of phase noise on the NCO
	nval = nval / abs(nval);
      }
      last = nval;
      return nval; 
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
      ejw = exp(std::complex<double>(0.0, -phase_incr));
    }
    
  private:
    double phase_incr;
    double ang; 
    std::complex<double> ejw, last; 
    int idx; 
  };
}

