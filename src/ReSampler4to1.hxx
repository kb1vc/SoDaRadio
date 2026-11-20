/*
  Copyright (c) 2025 Matthew H. Reilly (kb1vc)
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
#include <cstdint>
#include <memory>
#include <vector>


namespace SoDa {

  class ReSampler4to1;
  typedef std::shared_ptr<ReSampler4to1> ReSampler4to1Ptr;

  /**
   * @class ReSampler4to1
   * @brief Fixed 4:1 complex-IQ decimator built as a cascade of two Kaiser-windowed
   *        halfband FIR filters.
   *
   * This is a drop-in replacement for the general SoDa::ReSampler when the ratio is
   * exactly 4:1 — notably the 2.5 MS/s → 625 kS/s path used by the PlutoSDR front end,
   * and (planned) the N3xx-series USRP front end whose minimum hardware rate is also
   * 2.5 MS/s.  For integer power-of-two ratios the halfband cascade avoids the FFTs
   * that dominate the general resampler's run time.
   *
   * ### Why halfbands?
   *
   * A halfband FIR of length @f$N = 4L+3@f$ has the property that every other tap
   * (except the center) is exactly zero.  When used in a 2:1 polyphase decimator
   * one polyphase branch collapses to a single scalar multiply by @f$0.5@f$, and
   * the other branch is a symmetric FIR of length @f$(N+1)/2@f$.  After folding
   * the symmetric pairs before the multiply, each output sample costs only
   * @f$L+2@f$ real-by-complex multiplies.
   *
   * ### Cascade structure
   *
   * Stage 1 (input rate 2.5 MHz → 1.25 MHz) has an enormous transition band
   * (passband 250 kHz, stopband 1.0 MHz) and needs only N = 19 taps for 80 dB
   * stopband.  Stage 2 (1.25 MHz → 625 kHz) has a much tighter transition
   * (passband 250 kHz, stopband 375 kHz) and requires N = 55 taps.
   * Stage 2 runs at half the sample rate of stage 1, so its higher per-sample
   * cost is offset by processing half as many samples.
   *
   * ### Filter design
   *
   * Coefficients are computed at construction via the window method with a
   * Kaiser window (@f$\beta \approx 7.857@f$ for 80 dB).  The ideal halfband
   * impulse response is @f$h_\mathrm{ideal}[n] = \frac{1}{2}\mathrm{sinc}(n/2)@f$;
   * the halfband zero structure is intrinsic to that sinc argument and is
   * preserved by any symmetric window.  No Remez / Parks-McClellan optimizer
   * is needed — the window method gives us 80 dB with only a few extra taps.
   *
   * ### Streaming
   *
   * The class maintains per-stage state so that successive calls to apply()
   * on back-to-back input buffers produce a seamless, phase-coherent output.
   * The first call has no history and uses zero-initialised state; the startup
   * transient is @f$O(N/2)@f$ samples (negligible for the 120 000-sample
   * buffers used in the radio path).
   *
   * @note The input buffer size passed to the constructor must be divisible by
   *       four.  Output buffer size is always input_buffer_size / 4.
   */
  class ReSampler4to1 {
  public:

    /**
     * @brief Factory — allocate a ReSampler4to1 and return a shared pointer.
     *
     * Matches the style of SoDa::ReSampler::make().  The underlying constructor
     * is public but calling make() keeps the shared_ptr idiom consistent across
     * the rest of the resampler code in the project.
     *
     * @param input_buffer_size  Number of complex IQ samples the caller will
     *                           pass to apply() on each call.  Must be a
     *                           multiple of 4.
     * @return  A ReSampler4to1Ptr owning the new resampler.
     */
    static ReSampler4to1Ptr make(uint32_t input_buffer_size);

    /**
     * @brief Construct the resampler and build its filter coefficients.
     *
     * Designs both halfband stages using the Kaiser window method (80 dB
     * stopband, passband edge 250 kHz referenced to the 2.5 MHz → 625 kHz
     * use case), allocates per-stage state and work buffers sized for
     * @p input_buffer_size, and primes the state buffers to zero.
     *
     * @param input_buffer_size  Samples per apply() call; must be a multiple
     *                           of 4 so that both decimation stages produce
     *                           integer-length output buffers.
     */
    explicit ReSampler4to1(uint32_t input_buffer_size);

    /**
     * @brief Apply the 4:1 decimation to a buffer of complex IQ samples.
     *
     * Executes two halfband 2:1 decimation passes back-to-back.  State is
     * preserved between calls, so the output stream is phase-continuous
     * across buffer boundaries.
     *
     * @param in   Input buffer of exactly getInputBufferSize() complex samples.
     *             Not modified, though declared non-const to match the
     *             SoDa::ReSampler interface convention.
     * @param out  Output buffer; will be resized to getOutputBufferSize() and
     *             filled with the decimated stream.
     * @return     The number of output samples produced
     *             (= getOutputBufferSize()).
     */
    uint32_t apply(std::vector<std::complex<float>> & in,
                   std::vector<std::complex<float>> & out);

    /**
     * @brief Required input buffer size for apply().
     */
    uint32_t getInputBufferSize() const { return in_size; }

    /**
     * @brief Output buffer size produced by apply() (always in_size / 4).
     */
    uint32_t getOutputBufferSize() const { return out_size; }

  private:

    /**
     * @brief Design one Kaiser-windowed halfband filter, stored in compact form.
     *
     * For a halfband of length @p N = 4L+3, only L+1 unique coefficients are
     * needed because:
     *   - 2L of the taps are exactly zero (halfband property)
     *   - The remaining (2L+3) non-zero taps include one center tap that is
     *     exactly 0.5 and (2L+2) symmetric non-center taps that fold into
     *     L+1 pairs.
     *
     * This routine populates @p sym with those L+1 pair coefficients.  The
     * center tap is understood to be 0.5 and is applied separately by the
     * convolution loop.
     *
     * @param N     Filter length; must satisfy N = 4L + 3 (so N mod 4 == 3).
     * @param beta  Kaiser window shape parameter
     *              (@f$\beta = 0.1102 (A - 8.7)@f$ for stopband A in dB, A > 50).
     * @param sym   Output: L+1 symmetric-pair coefficients (sym[0] is the
     *              outermost pair h[0]=h[N-1], sym[L] is the innermost pair
     *              h[(N-3)/2]=h[(N+1)/2]).
     */
    static void designHalfband(uint32_t N, double beta, std::vector<float> & sym);

    /**
     * @brief Kaiser window value at sample @p n of a length-@p N window.
     *
     * Uses the modified Bessel-I0 approximation below.  Called only at
     * construction, so performance is not critical.
     */
    static double kaiserWindow(uint32_t n, uint32_t N, double beta);

    /**
     * @brief Polynomial approximation to the modified Bessel function I0(x).
     *
     * Abramowitz & Stegun formulas 9.8.1 and 9.8.2, good to ~1e-7.
     * Sufficient for coefficient design; not used in the hot path.
     */
    static double besselI0(double x);

    /**
     * @brief Run one 2:1 halfband decimation pass over one input buffer.
     *
     * Implements the symmetric-pair-folded polyphase form
     * @f$ y[n] = 0.5 \cdot x[2n - (N{-}1)/2] + \sum_{j=0}^{L} \mathrm{sym}[j]
     *                 \cdot (x[2n - 2j] + x[2n - (N{-}1) + 2j]) @f$.
     *
     * The @p state and @p work vectors handle inter-call continuity:
     *   - On entry, @p state holds the last (N-1) samples of the previous
     *     block (zero-initialised on the very first call).
     *   - @p work is a scratch buffer of size (N-1) + in.size() that is
     *     filled with [state | in] before the inner loop, then the last
     *     (N-1) samples are copied back into @p state for the next call.
     *
     * @param in     Input block (in.size() must be even).
     * @param out    Output block, resized to in.size() / 2.
     * @param sym    Symmetric-pair coefficients from designHalfband().
     * @param N      Halfband filter length (4L+3).
     * @param state  Per-stage history buffer, size N-1.
     * @param work   Per-stage scratch buffer, size (N-1) + in.size().
     */
    static void halfbandStep(const std::vector<std::complex<float>> & in,
                             std::vector<std::complex<float>> & out,
                             const std::vector<float> & sym,
                             uint32_t N,
                             std::vector<std::complex<float>> & state,
                             std::vector<std::complex<float>> & work);

    // ---------------------------------------------------------------
    // Fixed design parameters for the 2.5 MHz → 625 kHz path
    // ---------------------------------------------------------------

    /// Stage 1 halfband length: 19 taps gives ~85 dB stopband at Δω = 0.6π.
    static constexpr uint32_t STAGE1_N = 19;

    /// Stage 2 halfband length: 55 taps gives ~85 dB stopband at Δω = 0.2π.
    /// (51 would meet 80 dB on the nose; 55 gives ~5 dB margin for ~7% more work.)
    static constexpr uint32_t STAGE2_N = 55;

    /// Kaiser β for 80 dB attenuation: β = 0.1102 × (80 − 8.7).
    static constexpr double KAISER_BETA = 7.857;

    // ---------------------------------------------------------------
    // Buffer sizes
    // ---------------------------------------------------------------

    uint32_t in_size;     ///< input samples per apply() call
    uint32_t mid_size;    ///< samples after stage 1 (= in_size / 2)
    uint32_t out_size;    ///< samples after stage 2 (= in_size / 4)

    // ---------------------------------------------------------------
    // Per-stage filter state and buffers
    // ---------------------------------------------------------------

    std::vector<float> stage1_sym;                       ///< 5 pair-coeffs for N=19
    std::vector<std::complex<float>> stage1_state;       ///< history: N-1 = 18 samples
    std::vector<std::complex<float>> stage1_work;        ///< scratch: state + input
    std::vector<std::complex<float>> mid_buffer;         ///< stage-1 output / stage-2 input

    std::vector<float> stage2_sym;                       ///< 14 pair-coeffs for N=55
    std::vector<std::complex<float>> stage2_state;       ///< history: N-1 = 54 samples
    std::vector<std::complex<float>> stage2_work;        ///< scratch: state + mid
  };
}
