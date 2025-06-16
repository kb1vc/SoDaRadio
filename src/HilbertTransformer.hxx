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
#ifndef HILBERT_HDR
#define HILBERT_HDR

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
   * @class HilbertTransformer
   *
   * In several places we have a real valued signal x(t) that needs to be
   * converted to an analytic signal g(t) such that real(g(t)) == x(t)
   * and imag(g(t)) = shift_by_90degrees(x(t));
   *
   * HilbertTransformer provides the apply functions to convert x(t)
   * as a float or complex<float>.
   * Hilbert Transformer also provides a function (applyIQ) to convert a complex x(t)
   * into g(t) such that
   *   real(g(t)) = real(x(t + tau)) and
   *   imag(g(t)) = shift_by_90deg(imag(x(t + tau)))
   */
  class HilbertTransformer : public SoDa::Base {
  public:
    /**
     * constructor -- build a Hilbert Transformer
     * @param inout_buffer_length the length of the input and output buffers
     * @param filter_length the minimum length of the hilbert transform impulse response
     */
    HilbertTransformer(unsigned int inout_buffer_length, unsigned int filter_length = 256);

    /**
     * Perform a hilbert transform on the QUADRATURE signal in the input buffer.
     * Pass the Inphase signal through a delay filter that matches the hilbert transform
     *
     * @param inbuf complex input buffer of I (real) and Q (imag) samples
     * @param outbuf complex output buffer real is input.real delayed, and imag is Hilbert(input.imag)
     * @param gain factor to apply to output buffer.
     * @return M -- length of input buffer.
     */
    unsigned int applyIQ(std::vector<std::complex<float>> & inbuf, 
			 std::vector<std::complex<float>> & outbuf, 
			 float gain = 1.0);
    
    /**
     * Perform a hilbert transform on the INPHASE signal in the input buffer.
     * It is assumed that the QUADRATURE signal is zero, if not, the result is broken. 
     *
     * @param inbuf complex input buffer of I (real) and Q (imag) samples
     * @param outbuf complex output buffer real is input.real delayed, and imag is Hilbert(input.real)
     * @param pos_sided if true, swap I and Q outputs
     * @param gain factor to apply to output buffer.
     * @return M -- length of input buffer.
     */
    unsigned int apply(std::vector<std::complex<float>> & inbuf, 
		       std::vector<std::complex<float>> & outbuf, bool pos_sided = true, float gain = 1.0);

    /**
     * Perform a hilbert transform on the signal in the floating point input buffer.
     *
     * @param inbuf  input buffer of real (float) samples
     * @param outbuf complex output buffer real is input delayed, and imag is Hilbert(input)
     * @param pos_sided if true, swap I and Q outputs
     * @param gain factor to apply to output buffer.
     * @return M -- length of input buffer.
     */
    unsigned int apply(std::vector<float> & inbuf,
		       std::vector<std::complex<float>> & outbuf, 
		       bool pos_sided = true, float gain = 1.0);

    std::ostream & dump(std::ostream & os); 
  private:
    /**
     *these are the salient dimensions for this Overlap/Save
     * widget (for terminology, see Lyons pages 719ff
     */
    unsigned int M; ///< the input buffer length;
    unsigned int Q; ///< the filter length
    unsigned int N; ///< the total length of the transform N > (M + Q-1)
    
    // these are the intermediate buffers
    std::complex<float> * fft_I_input, * fft_Q_input;
    std::complex<float> * fft_I_output, * fft_Q_output;
    std::complex<float> * ifft_I_input, * ifft_Q_input;
    std::complex<float> * ifft_I_output, * ifft_Q_output;  
    
    // each filter needs two plans, a forward and backward
    // plan for the FFT and IFFT
    fftwf_plan forward_I_plan, forward_Q_plan, backward_I_plan, backward_Q_plan;

    std::complex<float> * HTu_filter; ///< The DFT image of the hilbert transform -- upper sideband
    std::complex<float> * HTl_filter; ///< The DFT image of the hilbert transform -- lower sideband
    std::complex<float> * Pass_U_filter; ///< The DFT image of a Q/2 delay transform -- used in USB.
    std::complex<float> * Pass_L_filter; ///< The DFT image of a Q/2 delay transform -- used in LSB.
    // we need to correct for "gain" in the fftw forward /backward transform pair.
    float passthrough_gain; ///< the gain of the direct passthrough path. 
    float H_transform_gain; ///< the gain of the Hilbert Transform path
  };
}

#endif
