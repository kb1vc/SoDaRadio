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

#include "ReSampler4to1.hxx"

#include <algorithm>
#include <cmath>
#include <stdexcept>


namespace SoDa {

  // ---------------------------------------------------------------
  // Factory
  // ---------------------------------------------------------------

  ReSampler4to1Ptr ReSampler4to1::make(uint32_t input_buffer_size) {
    return std::make_shared<ReSampler4to1>(input_buffer_size);
  }

  // ---------------------------------------------------------------
  // Construction
  // ---------------------------------------------------------------

  ReSampler4to1::ReSampler4to1(uint32_t input_buffer_size)
    : in_size(input_buffer_size),
      mid_size(input_buffer_size / 2),
      out_size(input_buffer_size / 4)
  {
    // Buffer size must be divisible by 4 so that each 2:1 stage produces an
    // integer number of outputs.  Catching this at construction is much
    // friendlier than a half-sample drift at runtime.
    if ((input_buffer_size % 4) != 0) {
      throw std::invalid_argument(
        "ReSampler4to1: input_buffer_size must be a multiple of 4");
    }

    // Build the two halfband filters.  Both use the same Kaiser β (the shape
    // parameter is a function of stopband attenuation only, not transition
    // width), so they share KAISER_BETA.
    designHalfband(STAGE1_N, KAISER_BETA, stage1_sym);
    designHalfband(STAGE2_N, KAISER_BETA, stage2_sym);

    // Pre-size and zero the history buffers.  First-call outputs will carry
    // an N/2-sample startup transient; harmless for the 120 000-sample radio
    // buffers but worth noting.
    stage1_state.assign(STAGE1_N - 1, std::complex<float>(0.0f, 0.0f));
    stage2_state.assign(STAGE2_N - 1, std::complex<float>(0.0f, 0.0f));

    // Pre-reserve the scratch work buffers so apply() does no allocation on
    // the hot path.
    stage1_work.reserve((STAGE1_N - 1) + in_size);
    stage2_work.reserve((STAGE2_N - 1) + mid_size);
    mid_buffer.reserve(mid_size);
  }

  // ---------------------------------------------------------------
  // Hot path: run both halfband stages back-to-back
  // ---------------------------------------------------------------

  uint32_t ReSampler4to1::apply(std::vector<std::complex<float>> & in,
                                std::vector<std::complex<float>> & out)
  {
    // Guard against caller size mismatch — the design uses a fixed buffer
    // length, and a mismatch would produce silently wrong output.
    if (in.size() != in_size) {
      throw std::invalid_argument(
        "ReSampler4to1::apply: input size does not match constructor size");
    }

    // Stage 1: 2.5 MHz → 1.25 MHz
    halfbandStep(in, mid_buffer, stage1_sym, STAGE1_N,
                 stage1_state, stage1_work);

    // Stage 2: 1.25 MHz → 625 kHz
    halfbandStep(mid_buffer, out, stage2_sym, STAGE2_N,
                 stage2_state, stage2_work);

    return out_size;
  }

  // ---------------------------------------------------------------
  // Core halfband polyphase decimator (shared by both stages)
  // ---------------------------------------------------------------

  void ReSampler4to1::halfbandStep(const std::vector<std::complex<float>> & in,
                                   std::vector<std::complex<float>> & out,
                                   const std::vector<float> & sym,
                                   uint32_t N,
                                   std::vector<std::complex<float>> & state,
                                   std::vector<std::complex<float>> & work)
  {
    const uint32_t M = static_cast<uint32_t>(in.size());  // input length
    const uint32_t state_size = N - 1;                    // history length
    const uint32_t L = (N - 3) / 4;                       // 0..L indexes sym pairs
    const uint32_t center = (N - 1) / 2;                  // h[(N-1)/2] = 0.5

    // Compose work = [state | in].  Using a contiguous buffer lets the inner
    // loop index linearly, which is what the compiler needs to auto-vectorize.
    work.resize(state_size + M);
    std::copy(state.begin(), state.end(), work.begin());
    std::copy(in.begin(), in.end(), work.begin() + state_size);

    out.resize(M / 2);

    // Convolution:
    //   y[n] = sum_k h[k] * x[2n - k]
    //
    // After folding the halfband zero structure and the symmetry h[k]=h[N-1-k]:
    //   y[n] = 0.5 * x[2n - center]
    //        + sum_{j=0}^{L} sym[j] * ( x[2n - 2j] + x[2n - (N-1) + 2j] )
    //
    // In work coordinates (work[state_size + i] holds x[i]):
    //   x[2n - 2j]         → work[state_size + 2n - 2j]
    //   x[2n - (N-1) + 2j] → work[2n + 2j]
    //   x[2n - center]     → work[state_size + 2n - center]
    //
    // The L+1 pair iterations are independent of each other and of the loop
    // counter n, so the inner j-loop vectorizes cleanly with -O3.
    const float * sym_data = sym.data();
    const std::complex<float> * work_data = work.data();
    std::complex<float> * out_data = out.data();

    for (uint32_t n = 0; n < M / 2; n++) {
      const uint32_t base_hi = state_size + 2 * n;   // index of x[2n]
      const uint32_t base_lo = 2 * n;                // index of x[2n - (N-1)]

      // Center tap (always 0.5 for a halfband).
      std::complex<float> acc = 0.5f * work_data[state_size + 2 * n - center];

      // Symmetric pair accumulation.
      for (uint32_t j = 0; j <= L; j++) {
        std::complex<float> pair_sum =
          work_data[base_hi - 2 * j] + work_data[base_lo + 2 * j];
        acc += sym_data[j] * pair_sum;
      }

      out_data[n] = acc;
    }

    // Carry the tail of the input forward as state for the next call.
    // These are the N-1 samples that y[M/2] (first output of the next block)
    // will reach back into.
    std::copy(work.end() - state_size, work.end(), state.begin());
  }

  // ---------------------------------------------------------------
  // Kaiser-windowed halfband design
  // ---------------------------------------------------------------

  void ReSampler4to1::designHalfband(uint32_t N,
                                     double beta,
                                     std::vector<float> & sym)
  {
    // Halfband length constraint: N must be of the form 4L+3 so that
    //   - the center tap lives at an odd position ((N-1)/2 = 2L+1)
    //   - all even-position taps are non-zero (these form the sym[] pairs)
    //   - all other odd-position taps are zero
    if ((N % 4) != 3) {
      throw std::invalid_argument(
        "designHalfband: N must satisfy N mod 4 == 3");
    }

    const uint32_t L = (N - 3) / 4;    // L+1 symmetric pairs
    const double center_pos = 0.5 * (N - 1);

    sym.resize(L + 1);

    // sym[j] corresponds to filter positions 2j (and N-1-2j by symmetry).
    // The centered index of position 2j is m = 2j - center_pos, which is
    // always odd (2j is even, center_pos is a half-integer that resolves
    // to odd + half when (N-1)/2 is odd — here m is always a non-zero
    // integer at odd distance from center).
    //
    // Ideal halfband impulse response (centered form):
    //   h_ideal[m] = (1/2) * sinc(m/2)   where sinc(x) = sin(πx)/(πx)
    // For odd m this is nonzero; for even m ≠ 0 it is zero.  The sym[] array
    // only stores the odd-m entries.
    const double denom = besselI0(beta);

    for (uint32_t j = 0; j <= L; j++) {
      const double m = static_cast<double>(2 * j) - center_pos;   // odd integer
      const double x = 0.5 * m;                                   // half-integer
      const double sinc_val = std::sin(M_PI * x) / (M_PI * x);

      // Kaiser window value at this tap position (position 2j within the
      // length-N window).  The window is symmetric so evaluating at 2j is
      // equivalent to evaluating at N-1-2j.
      const double r = (static_cast<double>(2 * j) - center_pos) / center_pos;
      const double w = besselI0(beta * std::sqrt(1.0 - r * r)) / denom;

      sym[j] = static_cast<float>(0.5 * sinc_val * w);
    }
  }

  double ReSampler4to1::kaiserWindow(uint32_t n, uint32_t N, double beta) {
    // Standard symmetric Kaiser:
    //   w[n] = I0(β·sqrt(1 - ((n - M/2)/(M/2))²)) / I0(β),  M = N - 1
    //
    // Kept as a utility for callers that want the window directly; designHalfband
    // inlines the same math for efficiency since it only needs the even-index
    // samples.
    const double M = static_cast<double>(N - 1);
    const double r = (static_cast<double>(n) - 0.5 * M) / (0.5 * M);
    return besselI0(beta * std::sqrt(1.0 - r * r)) / besselI0(beta);
  }

  double ReSampler4to1::besselI0(double x) {
    // Abramowitz & Stegun 9.8.1 / 9.8.2 polynomial approximations.
    // Accurate to a few parts in 1e7, which is well below the precision
    // needed for filter coefficient design in float.
    const double ax = std::fabs(x);
    if (ax < 3.75) {
      const double y = (x / 3.75) * (x / 3.75);
      return 1.0 + y * (3.5156229 +
             y * (3.0899424 +
             y * (1.2067492 +
             y * (0.2659732 +
             y * (0.0360768 +
             y * 0.0045813)))));
    } else {
      const double y = 3.75 / ax;
      return (std::exp(ax) / std::sqrt(ax)) *
             (0.39894228 +
              y * ( 0.01328592 +
              y * (-0.00157565 +
              y * ( 0.00916281 +
              y * (-0.02057706 +
              y * ( 0.02635537 +
              y * (-0.01647633 +
              y *  0.00392377)))))));
    }
  }
}
