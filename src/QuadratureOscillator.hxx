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
#pragma once

#include <complex>
#include <vector>
#include <math.h>


namespace SoDa {
  /**
   * @class QuadratureOscillator
   * @brief Complex phasor oscillator driven by recursive complex multiplication.
   *
   * Generates a quadrature (I/Q) tone by rotating a unit phasor one step per
   * sample: @f$ \hat{p}_{n+1} = \hat{p}_n \cdot e^{-j\omega} @f$.  This costs
   * one complex multiply per sample — far cheaper than calling sin/cos.
   *
   * ### Fast buffer path (preferred)
   *
   * When the oscillator is constructed with a buffer size, it pre-computes a
   * table of phasors @f$ e^{-j\omega k},\ k = 0 \ldots N-1 @f$.  The table is
   * built once at construction and rebuilt only on a frequency change (retune).
   * With the table in hand, mulBuffer() computes
   * @f$ \mathrm{buf}[i] \mathrel{*}= \hat{p}_0 \cdot e^{-j\omega i} @f$
   * for all @f$i@f$ in one pass.  Every sample is independent of its
   * neighbours, so the compiler can auto-vectorize the loop with SSE/AVX.
   *
   * ### Per-sample path (legacy / CW envelope use)
   *
   * stepOscCF() / stepOscCD() / stepOscD() remain available for callers that
   * multiply by a per-sample envelope (e.g. RadioTX::doCW).
   *
   * ### Phase noise
   *
   * Each complex multiply introduces ~2–3 × @f$\varepsilon_\mathrm{mach}@f$
   * radians of phase error (~2.2e-16 for double).  Errors accumulate as a
   * random walk so after @f$N@f$ steps the RMS phase error is
   * @f$\sqrt{N} \cdot 3\varepsilon@f$, which is negligible for all practical
   * sample counts.  mulBuffer() renormalizes |last| once per buffer; the
   * per-sample path renormalizes every 512 steps.  Neither resets phase error,
   * but the magnitude stays within floating-point rounding of 1.0.
   */
  class QuadratureOscillator {
  public:

    /**
     * @brief Construct and pre-build the phasor table for buffer-mode operation.
     *
     * Initialises the oscillator to a default frequency of
     * @f$ f_s / 1000 @f$ and immediately builds a @p buf_size -entry phasor
     * table so that mulBuffer() can be called without further setup.
     *
     * @param buf_size  Number of samples in one processing buffer
     *                  (typically params->getRFBufferSize()).
     */
    QuadratureOscillator(unsigned int buf_size);

    /**
     * @brief Default constructor — per-sample use only (no phasor table).
     *
     * Provided for members that are default-constructed before a buffer size
     * is known.  mulBuffer() will build the table on first call, but callers
     * that know the buffer size at construction should prefer the
     * single-argument form.
     */
    QuadratureOscillator();

    // ---------------------------------------------------------------
    // Per-sample interface (legacy; kept for envelope-modulated paths)
    // ---------------------------------------------------------------

    /**
     * @brief Advance the oscillator by one sample and return a complex<double>.
     *
     * Dispatches to either the sincos or complex-multiply implementation
     * depending on the USE_SINCOS_NCO compile flag.  The complex-multiply
     * path is the default and is significantly faster.
     *
     * @return  @f$ e^{-j\omega n} @f$ as a complex<double>.
     */
    std::complex<double> stepOscCD();

    /// sincos-based implementation of stepOscCD (slower; kept for reference).
    std::complex<double> stepOscCD_sincos();

    /// Complex-multiply implementation of stepOscCD (default fast path).
    std::complex<double> stepOscCD_complex();

    /**
     * @brief Advance one sample and return a complex<float>.
     *
     * Thin wrapper around stepOscCD() that narrows to float.  Used by
     * callers (e.g. the CW envelope loop) that multiply the oscillator
     * output by a per-sample scalar before accumulating into a buffer.
     *
     * @return  @f$ e^{-j\omega n} @f$ as a complex<float>.
     */
    std::complex<float> stepOscCF();

    /**
     * @brief Advance one sample and return only the real (cosine) part.
     * @return  @f$ \cos(\omega n) @f$.
     */
    double stepOscD();

    // ---------------------------------------------------------------
    // Buffer interface (fast path — use this in the RX mixing loops)
    // ---------------------------------------------------------------

    /**
     * @brief Multiply an IQ buffer by the oscillator in a single vectorisable pass.
     *
     * Implements @f$ \mathrm{buf}[i] \mathrel{*}= \hat{p} \cdot e^{-j\omega i} @f$
     * for every sample @f$i@f$ in @p buf, where @f$\hat{p}@f$ is the current
     * phasor at the start of the buffer.  Because every output sample depends
     * only on the (loop-invariant) initial phasor and the pre-computed table
     * entry, all iterations are independent and the compiler can emit SSE/AVX
     * vector instructions.
     *
     * After the loop, @p last is advanced by @f$N@f$ steps and renormalised to
     * unit magnitude, so successive calls produce a phase-continuous tone.
     *
     * If @p buf.size() differs from the table size (e.g. on the first call
     * after a retune with a different-sized buffer), the table is silently
     * rebuilt before proceeding.
     *
     * @param buf  IQ sample buffer to mix in-place; modified on return.
     */
    void mulBuffer(std::vector<std::complex<float>> & buf);

    // ---------------------------------------------------------------
    // Frequency control
    // ---------------------------------------------------------------

    /**
     * @brief Set the oscillator frequency via its phase increment per sample.
     *
     * @param _pi  Phase advance per sample in radians:
     *             @f$ \Delta\phi = 2\pi f / f_s @f$.
     *
     * Recomputes @f$e^{-j\Delta\phi}@f$ and, if a phasor table has already
     * been built, rebuilds it for the new frequency.  The rebuild is O(N) but
     * is called only on retune, not on every buffer.
     */
    void setPhaseIncr(double _pi);

  private:

    /**
     * @brief Build (or rebuild) the phasor table for a given buffer length.
     *
     * Fills phasor_table[i] = @f$ e^{-j\omega i} @f$ for i = 0 … n-1
     * (relative to the start of a buffer, i.e. table[0] = 1+0j always).
     * Also stores ejw_n = @f$ e^{-j\omega n} @f$ so that mulBuffer() can
     * advance @p last by exactly one buffer length in a single multiply.
     *
     * Called from the constructor and from setPhaseIncr() whenever the table
     * exists and the frequency changes.
     *
     * @param n  Number of entries to generate (= buffer size).
     */
    void buildTable(unsigned int n);

    double phase_incr;             ///< phase advance per sample (radians)
    double ang;                    ///< current angle (sincos path only)
    std::complex<double> ejw;      ///< unit step phasor: e^{-j*phase_incr}
    std::complex<double> ejw_n;    ///< N-step advance phasor: e^{-j*phase_incr*N}
    std::complex<double> last;     ///< current phasor state
    int idx;                       ///< step counter for per-sample renormalisation

    /// Pre-computed table: phasor_table[i] = e^{-j*phase_incr*i}, table[0] = 1+0j.
    std::vector<std::complex<float>> phasor_table;
  };
}
