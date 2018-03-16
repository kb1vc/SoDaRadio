/*
  Copyright (c) 2018 Matthew H. Reilly (kb1vc)
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

/**
 * @file TDReSamplers625x48.hxx
 * @brief Time domain resampler using polyphase filter technique from
 * Lyons "Understanding Digial Signal Processing" chapter 10. 
 *
 * @author Matt Reilly (kb1vc)
 */
#include "SoDaBase.hxx"

#ifndef TDRESAMPLERS_48625_HDR
#define TDRESAMPLERS_48625_HDR
#include <complex>
#include "ReSampler.hxx"

namespace SoDa {

  class TDRationalResampler;
  class TDInterpolator;
  class TDDecimator; 

  class TDFilter : public SoDaBase {
  public:
    TDFilter(const std::string & name) : SoDaBase(name) { }

    /**
     * @brief Perform decimation on a complex float buffer
     * @param in input buffer
     * @param out output buffer 
     * @param inlen number of samples in input buffer
     * @param max_outlen maximum number of samples in output buffer
     * @return number of samples in output buffer
     */
    virtual int apply(std::complex<float> * in, std::complex<float> * out, int inlen, int max_outlen) = 0;
  };


  class TDInterpolator : public TDFilter { 
  public:
    /** 
     * @brief Create a interpolator object
     * @param L interpolation rate.  Output vector will be inlen * L
     * @param proto_filter prototype low-pass anti-aliasing filter
     * @param filter_len length of prototype filter. Must be a multiple of L.
     */
    TDInterpolator(int L, std::complex<float> * proto_filter, int filter_len) :
      TDFilter("TDInterpolator")  { } 
    ~TDInterpolator() { }

    /**
     * @brief Perform interpolation on a complex float buffer
     * @param in input buffer
     * @param out output buffer 
     * @param inlen number of samples in input buffer
     * @param max_outlen maximum number of samples in output buffer
     * @return number of samples in output buffer
     */
    int apply(std::complex<float> * in, std::complex<float> * out, int inlen, int max_outlen) { } 

  private:
    std::complex<float> ** filter_bank; // segmented prototype filter
    std::complex<float> * buffer; // only needs to be filter_len / L
  };


  class TDDecimator : public TDFilter { 
  public:
    /** 
     * @brief Create a decimator object
     * @param M decimation rate.  Output vector will be inlen / M
     * @param LPF prototype low-pass anti-aliasing filter
     * @param filter_len length of prototype filter. Must be a multiple of M.
     */
    TDDecimator(int M, float * LPF, int filter_len); 
    ~TDDecimator() {
      delete[] leftovers; 
      delete[] filter; 
    }

    /**
     * @brief Perform decimation on a complex float buffer
     * @param in input buffer
     * @param out output buffer 
     * @param inlen number of samples in input buffer
     * @param max_outlen maximum number of samples in output buffer
     * @return number of samples in output buffer
     */
    int apply(std::complex<float> * in, std::complex<float> * out, int inlen, int max_outlen);

  private:
    int M; // downselect ratio
    float * filter; // low pass filter
    int filter_len; // length of partitioned subfilters
    std::complex<float> * leftovers; // leftover samples from last interation
    int lbuf_len; // maximum length of leftover buffer
    int leftover_count; // how many samples left over from last apply()
    float gain_correction; // restore amplitude 
  };

  class TDRationalResampler : public TDFilter { 
  public:
    /** 
     * @brief Create a rational resampler object
     * @param _M decimation rate.  
     * @param _L interpolation rate.  Output vector will be L * inlen / M
     * @param proto_filter prototype low-pass anti-aliasing filter
     * @param filter_len length of prototype filter. Must be a multiple of L.
     */
    TDRationalResampler(int _M, int _L, float * proto_filter, int filter_len);
    ~TDRationalResampler() {
      for(int i = 0; i < L; i++) {
	delete[] filter_bank[i]; 
      }
      delete[] filter_bank; 

      delete[] prefix_buf; 
    }

    /**
     * @brief Perform interpolation on a complex float buffer
     * @param in input buffer
     * @param out output buffer 
     * @param inlen number of samples in input buffer
     * @param max_outlen maximum number of samples in output buffer
     * @return number of samples in output buffer
     */
    int apply(std::complex<float> * in, std::complex<float> * out, int inlen, int max_outlen);

  private:

    void bumpCounters();
    
    float ** filter_bank;
    float * proto_filter; 
    std::complex<float> * prefix_buf;
    int M, L, taps; 
    int k;
    int n; 
    float gain_correction; 
  };


  class TDResampler625x48 : public TDFilter { 
  public:
    /** 
     * @brief Create a chain of rational resamplers to 
     * downsample from 625ks/Sec to 48ks/Sec
     */
    TDResampler625x48();
    ~TDResampler625x48() {
      delete rs51_p;
      delete rs53_p;
      delete rs54a_p;
      delete rs54b_p;

      if(lastoutlen != 0) {
	delete[] ibuf51;
	delete[] ibuf53;
	delete[] ibuf54a; 
      }
    }

    /**
     * @brief Perform interpolation on a complex float buffer
     * @param in input buffer
     * @param out output buffer 
     * @param inlen number of samples in input buffer
     * @param max_outlen maximum number of samples in output buffer
     * @return number of samples in output buffer
     */
    int apply(std::complex<float> * in, std::complex<float> * out, int inlen, int max_outlen);

  private:
    void allocateIBufs(int outlen);
    /// first stage resampler: 5 to 1
    TDFilter * rs51_p; 
    /// second stage resampler: 5 to 3
    TDFilter * rs53_p; 
    /// third stage resampler: 5 to 4
    TDFilter * rs54a_p; 
    /// fourth stage resampler: 5 to 4
    TDFilter * rs54b_p; 

    /// intermediate buffers 
    std::complex<float> *ibuf51, *ibuf53, *ibuf54a; 
    unsigned int len51, len53, len54a, lastoutlen;

    static float HCLPF35_5x1_125[];
    static float PMLPF30_5x3_75[];
    static float PMLPF32_5x4_60[];
    static float PMLPF32Sinc_5x4_60[];    
    static float PMLPF40_5x4_48[];
    static float PMLPF52_5x4_48[];         
  };
  
}
#endif
