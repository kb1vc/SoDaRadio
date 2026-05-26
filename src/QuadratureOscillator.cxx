/*
  Copyright (c) 2012,2013,2014,2025 Matthew H. Reilly (kb1vc)
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

#include "QuadratureOscillator.hxx"

#include <complex>
#include <math.h>


namespace SoDa {
  QuadratureOscillator::QuadratureOscillator() {
    idx = 0;
    ang = 0.0;
    last = std::complex<double>(1.0,0.0);
    ejw = last;

    // default IF freq is SampRate/1000;
    setPhaseIncr(2.0 * M_PI / 1000.0);
  }

  std::complex<double> QuadratureOscillator::stepOscCD() {
#ifdef USE_SINCOS_NCO
    return stepOscCD_sincos();
#else
    return stepOscCD_complex();      
#endif    
  }    

  std::complex<double> QuadratureOscillator::stepOscCD_sincos() {
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

  std::complex<double> QuadratureOscillator::stepOscCD_complex() {
    std::complex<double> nval;
    idx++;
    nval = last * ejw;     
    if(idx > 512) {
      idx = 0; 
      nval = nval / abs(nval);
    }
    last = nval;
    return nval; 
  }

  std::complex<float> QuadratureOscillator::stepOscCF() {
    std::complex<double> dv = stepOscCD();
    std::complex<float> fv((float)dv.real(), (float)dv.imag());
    return fv; 
  }

  double QuadratureOscillator::stepOscD() {
    std::complex<double> dv = stepOscCD(); 
    return dv.real(); 
  }

  void QuadratureOscillator::setPhaseIncr(double _pi) {
    phase_incr = _pi;
    ejw = exp(std::complex<double>(0.0, -phase_incr));
  }
    
}
