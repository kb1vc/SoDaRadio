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

  // ---------------------------------------------------------------
  // Constructors
  // ---------------------------------------------------------------

  QuadratureOscillator::QuadratureOscillator(unsigned int buf_size) {
    idx = 0;
    ang = 0.0;
    last = std::complex<double>(1.0, 0.0);
    ejw  = std::complex<double>(1.0, 0.0);
    ejw_n = std::complex<double>(1.0, 0.0);

    // Set default frequency (sample_rate/1000) and build the phasor table.
    // setPhaseIncr skips the table rebuild when the table is still empty,
    // so we call buildTable explicitly here.
    setPhaseIncr(2.0 * M_PI / 1000.0);
    buildTable(buf_size);
  }

  QuadratureOscillator::QuadratureOscillator() {
    idx = 0;
    ang = 0.0;
    last = std::complex<double>(1.0, 0.0);
    ejw  = last;
    ejw_n = std::complex<double>(1.0, 0.0);

    // No buf_size supplied — table stays empty.  mulBuffer() will build it
    // on first call; per-sample stepOsc*() work without a table.
    setPhaseIncr(2.0 * M_PI / 1000.0);
  }

  // ---------------------------------------------------------------
  // Phasor table management
  // ---------------------------------------------------------------

  void QuadratureOscillator::buildTable(unsigned int n) {
    phasor_table.resize(n);

    // Walk a double-precision phasor from 1+0j, stepping by ejw each sample.
    // Store each position as float — the table is used in a float multiply
    // loop, so double precision here only helps the initial placement accuracy.
    std::complex<double> p(1.0, 0.0);
    for (unsigned int i = 0; i < n; i++) {
      phasor_table[i] = std::complex<float>((float)p.real(), (float)p.imag());
      p *= ejw;
    }

    // p has now advanced n steps: p = ejw^n.  Store it so mulBuffer() can
    // advance 'last' by one full buffer in a single multiply.
    ejw_n = p;
  }

  // ---------------------------------------------------------------
  // Frequency control
  // ---------------------------------------------------------------

  void QuadratureOscillator::setPhaseIncr(double _pi) {
    phase_incr = _pi;
    ejw = std::exp(std::complex<double>(0.0, -phase_incr));

    // Rebuild the table only if one has already been allocated.  On first
    // call from the constructor the table is empty and buildTable() is
    // invoked explicitly afterwards.
    if (!phasor_table.empty()) {
      buildTable((unsigned int)phasor_table.size());
    }
  }

  // ---------------------------------------------------------------
  // Buffer interface
  // ---------------------------------------------------------------

  void QuadratureOscillator::mulBuffer(std::vector<std::complex<float>> & buf) {
    // Silently rebuild if the caller passes a buffer of unexpected size.
    // This should only happen on the first call after a retune with a
    // differently-sized buffer (e.g. the resampler path in USRPRX).
    if (buf.size() != phasor_table.size()) {
      buildTable((unsigned int)buf.size());
    }

    // Capture the current phasor as float.  The compiler treats 'cur' as a
    // loop-invariant scalar and hoists it, leaving an inner loop of the form:
    //   buf[i] *= cur * phasor_table[i]
    // Every iteration is independent (no cross-iteration data dependency),
    // so the compiler can emit SSE/AVX vector instructions.
    std::complex<float> cur((float)last.real(), (float)last.imag());
    const size_t n = buf.size();
    for (size_t i = 0; i < n; i++) {
      buf[i] *= cur * phasor_table[i];
    }

    // Advance 'last' by N steps and renormalise to unit magnitude.
    // Renormalising once per buffer is equivalent to the every-512-step policy
    // in stepOscCD_complex(): magnitude drift per step is O(eps_mach), so
    // over 30 000 steps it is ~6e-12 — well within float precision.
    last *= ejw_n;
    last /= std::abs(last);
  }

  // ---------------------------------------------------------------
  // Per-sample interface
  // ---------------------------------------------------------------

  std::complex<double> QuadratureOscillator::stepOscCD() {
#ifdef USE_SINCOS_NCO
    return stepOscCD_sincos();
#else
    return stepOscCD_complex();
#endif
  }

  std::complex<double> QuadratureOscillator::stepOscCD_sincos() {
    double s, c;
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
    if (idx > 512) {
      idx = 0;
      nval = nval / std::abs(nval);
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

}
