/*
  Copyright (c) 2018,2024 Matthew H. Reilly (kb1vc)
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
 * @file TDResamplers625x48.hxx
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
#include "TDResamplerTables625x48.hxx"

namespace SoDa {


  template<typename T> class TDFilter : public SoDa::Base {
  public:
    TDFilter(const std::string & name) : SoDa::Base(name) { }

    /**
     * @brief Perform decimation on a complex float buffer
     * @param in input buffer
     * @param out output buffer 
     * @param inlen number of samples in input buffer
     * @param max_outlen maximum number of samples in output buffer
     * @return number of samples in output buffer
     */
    virtual int apply(T * in, T * out, int inlen, int max_outlen) = 0;
  };

  template<typename T> class TDRationalResampler : public TDFilter<T> { 
  public:
    /** 
     * @brief Create a rational resampler object
     * @param _M decimation rate.  
     * @param _L interpolation rate.  Output vector will be L * inlen / M
     * @param proto_filter prototype low-pass anti-aliasing filter
     * @param filter_len length of prototype filter. Must be a multiple of L.
     * @param gain filter gain
     */
    TDRationalResampler(int _M, int _L, 
			float * proto_filter, int filter_len, 
			float gain = 1.0);
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
    int apply(T * in, T * out, int inlen, int max_outlen);

  private:

    void bumpCounters();
    
    float ** filter_bank;
    float * proto_filter; 
    T * prefix_buf;
    int M, L, taps; 
    int k;
    int n; 
    float gain_correction; 
  };


  template<typename T> class TDResampler625x48 : public TDFilter<T> { 
  public:
    /** 
     * @brief Create a chain of rational resamplers to 
     * downsample from 625ks/Sec to 48ks/Sec
     */
    TDResampler625x48(float gain = 1.0);
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

    static std::shared_ptr<TDResampler625x48<T>> make(float gain = 1.0) {
      return std::make_shared<TDResampler625x48<T>>(gain); 
    }
    
    /**
     * @brief Perform interpolation on a complex float buffer
     * @param in input buffer
     * @param out output buffer 
     * @param inlen number of samples in input buffer
     * @param max_outlen maximum number of samples in output buffer
     * @return number of samples in output buffer
     */
    int apply(T * in, T * out, int inlen, int max_outlen);

  private:
    void allocateIBufs(int outlen);
    /// first stage resampler: 5 to 1
    TDFilter<T> * rs51_p; 
    /// second stage resampler: 5 to 3
    TDFilter<T> * rs53_p; 
    /// third stage resampler: 5 to 4
    TDFilter<T> * rs54a_p; 
    /// fourth stage resampler: 5 to 4
    TDFilter<T> * rs54b_p; 

    /// intermediate buffers 
    T *ibuf51, *ibuf53, *ibuf54a; 
    unsigned int len51, len53, len54a, lastoutlen;

    TDResamplerTables625x48 tables; 
  };

  template <typename T> TDRationalResampler<T>::TDRationalResampler(int _M, int _L, float * _proto_filter, int filter_len, float gain) :
    TDFilter<T>(SoDa::Format("RationalResampler %0 to %1")
		.addI(M)
		.addI(L).str())
  {
    M = _M; 
    L = _L; 
    taps = filter_len / L;
    k = 0;   
    n = 0; 
    prefix_buf = new T[taps * 2];
    int i; 
    for(i = 0; i < taps * 2; i++) prefix_buf[i] = T(0);

    proto_filter = new float[filter_len]; 
    memcpy(proto_filter, _proto_filter, sizeof(float) * filter_len); 

    filter_bank = new float*[L]; 
    for(i = 0; i < L; i++) {
      filter_bank[i] = new float[taps]; 
      for(int j = 0; j < taps; j++) {
	filter_bank[i][j] = proto_filter[i + j * L]; 
      }
    }
    float fsum = 0.0; 
    for(i = 0; i < filter_len; i++) fsum += proto_filter[i]; 

    gain_correction = gain * ((float) L) / fsum;
  }

  template<typename T> int TDRationalResampler<T>::apply(T *in, T * out, 
							 int inlen, int max_outlen)
  {
    // Using the terminology from Lyons pp 541 -- note that we use the
    // recurrence sum in equation 10-20'' rather than the fancy diagram
    // with the separate shift registers.  
    T zero(0);
    T rsum; 

    T * x; 
    x = &prefix_buf[taps];
    // we need a prefix buffer for the "old" samples before in[n]
    memcpy(x, in, sizeof(T) * taps);

    int m; 
    // first consume the prefix buffer, then the input vector
    for(m = 0; (m < max_outlen) && (n < inlen); m++) {
      rsum = zero; 
      for(int i = 0; i < taps; i++) {
	// rsum += filter_bank[k][i] * x[n - i];
	rsum += proto_filter[i * L + k] * x[n - i];
      }
      out[m] = rsum * gain_correction; 
      bumpCounters();
      // once we've gotten through the prefix (leftover from last pass)
      // switch to the actual input buffer
      if((x != in) && (n > (taps - 2))) {
	x = in; 
      }
    }

    // now save the last of the input vector
    for(int i = 0; i < taps; i++) {
      prefix_buf[i] = in[i + (inlen - taps)]; 
    }
    n = n - inlen; 
    return m; 
  }

  template<typename T> void TDRationalResampler<T>::bumpCounters() 
  {
    // bump k and n    
    k += M; 
    while(k >= L) {
      k = k - L; 
      n++; 
    }
  }

  template<typename T> TDResampler625x48<T>::TDResampler625x48(float gain) :
    TDFilter<T>("TDResampler625x48")
  {
    lastoutlen = 0; 
    ibuf51 = ibuf53 = ibuf54a = NULL; 

    rs51_p = new TDRationalResampler<T>(5, 1, tables.HCLPF35_5x1_125, 35);
    rs53_p = new TDRationalResampler<T>(5, 3, tables.PMLPF30_5x3_75, 30);
    rs54a_p = new TDRationalResampler<T>(5, 4, tables.PMLPF32_5x4_60, 32);
    rs54b_p = new TDRationalResampler<T>(5, 4, tables.PMLPF40_5x4_48, 40, gain);
  }

  template<typename T> void TDResampler625x48<T>::allocateIBufs(int outlen)
  {
    if(outlen != lastoutlen) {
      if(ibuf51 != NULL) {
	delete[] ibuf51;
	delete[] ibuf53;
	delete[] ibuf54a;       
      }

      lastoutlen = outlen; 
      len54a = 3 + (outlen * 5) / 4;
      len53 = 3 + (len54a * 5) / 4;
      len51 = 2 + (len53 * 5) / 3;
      ibuf51 = new T[len51];
      ibuf53 = new T[len53];
      ibuf54a = new T[len54a];    
    }
  }

  template<typename T> int TDResampler625x48<T>::apply(T * in, 
						       T * out, 
						       int inlen, int max_outlen)
  {
    // do we need new intermediate buffers? 
    allocateIBufs(max_outlen); 

    // resample through the stages
    int len;
    len = rs51_p->apply(in, ibuf51, inlen, len51);
    len = rs53_p->apply(ibuf51, ibuf53, len, len53);
    len = rs54a_p->apply(ibuf53, ibuf54a, len, len54a);
    len = rs54b_p->apply(ibuf54a, out, len, max_outlen); 

    return len; 
  }
  
}
#endif
