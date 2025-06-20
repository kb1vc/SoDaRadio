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

#include <fstream>
#include <complex>
#include <fftw3.h>
#include "SoDaBase.hxx"

namespace SoDa {
  /**
   * Spectrogram generates magnitude buffers from input sample stream. 
   */

  class Spectrogram;
  typedef std::shared_ptr<Spectrogram> SpectrogramPtr;


  
  class Spectrogram : public Base {
  protected:
    /**
     * @brief Constructor
     *
     * @param fftlen how many frequency points in the spectrogram buffer
     */
    Spectrogram(unsigned int fftlen);

  public:
    static SpectrogramPtr make(unsigned int fftlen) {
      auto ret = std::shared_ptr<Spectrogram>(new Spectrogram(fftlen));
      ret->registerSelf(ret);
      return ret; 
    }

    /**
     * @brief Calculate the spectrogram from an input vector -- add it to
     * an accumulation buffer.
     *
     * @param invec the input sample buffer
     * @param outvec the result buffer
     * @param accumulation_gain (out = result + out * acc_gain)
     */
    void apply_acc(std::vector<std::complex<float>> & invec,
		   std::vector<float> & outvec, 
		   float accumulation_gain = 0.0); 
    
    /**
     * @brief Calculate the spectrogram from an input vector -- add it to
     * an accumulation buffer.
     *
     * @param invec the input sample buffer
     * @param outvec the result buffer (out = max(result, out))
     * @param first if true, ignore contents of outvec... 
     */
    void apply_max(std::vector<std::complex<float>> & invec,
		   std::vector<float> & outvec, bool first = true);

    
  private:

    /**
     * @brief all spectrograms are under a blackman harris window
     */
    float * initBlackmanHarris();

    /**
     * @brief this is the common spectrogram calculation (window + fft + mag)
     *
     * @param invec the buffer we're going to operate on
     */
    void apply_common(std::vector<std::complex<float>> & invec);
    
    fftwf_plan fftplan;
    float * window;
    std::complex<float> * win_samp;
    std::complex<float> * fft_out; 
    float * result; 
    unsigned int fft_len; 
  }; 
}

