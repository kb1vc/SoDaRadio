/*
  Copyright (c) 2018, Matthew H. Reilly (kb1vc)
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
#include "TDReSamplers625x48.hxx"
#include "SoDaBase.hxx"
#include <iostream>
#include <stdlib.h>
#include <string.h>


SoDa::TDDecimator::TDDecimator(int _M, float * LPF, int _filter_len) :
  TDFilter("TDDecimator")
{
  leftover_count = 0; 
  M = _M; 
  filter_len = _filter_len; 
  
  filter = new float[filter_len];
  lbuf_len = filter_len + M;
  leftovers = new std::complex<float>[lbuf_len];
  filter = new float[filter_len]; 

  memcpy(filter, LPF, sizeof(float) * filter_len); 

  // calculate the gain correction
  float sum = 0.0; 
  for(int i = 0; i < filter_len; i++) {
    sum += LPF[i]; 
  }

  gain_correction = 1.0 / sum; 
}


int SoDa::TDDecimator::apply(std::complex<float> * in, std::complex<float> * out, int inlen, int max_outlen)
{
  std::complex<float> rsum(0.0, 0.0); 
  int j = 0; 
  int i = 0; 
  // use up any elements in the leftover buffer
  if(leftover_count != 0) {
    int fill_count = lbuf_len - leftover_count;
    memcpy(leftovers + leftover_count, in, sizeof(std::complex<float>) * fill_count); 
    // now do the filter loop 
    for(int p = 0; p < (lbuf_len - filter_len); p += M) {
      for(int k = 0; k < filter_len; k++) {
        rsum += leftovers[k + p] * filter[k]; 
      }
      out[j++] = rsum * gain_correction; 
      rsum = std::complex<float>(0.0,0.0);
    }
    i += fill_count;     
  }
  // run through the input vector, starting with the end of
  // the "leftover" buffer. 
  for(; i < (inlen - filter_len); i += M) {
    for(int k = 0; k < filter_len; k++) {
      rsum += in[k + i] * filter[k]; 
    }
    out[j++] = rsum * gain_correction; 
    rsum = std::complex<float>(0.0,0.0);
    if(j >= max_outlen) return -1; 
  }

  // copy leftovers to leftover buffer
  leftover_count = 0; 
  for(; i < inlen; i++) {
    leftovers[leftover_count++] = in[i]; 
  }

  return j; 
}

SoDa::TDRationalResampler::TDRationalResampler(int _M, int _L, float * _proto_filter, int filter_len) :
      TDFilter((boost::format("RationalResampler %d to %d") % M % L).str())
{
  M = _M; 
  L = _L; 
  taps = filter_len / L;
  k = 0;   
  n = 0; 
  prefix_buf = new std::complex<float>[taps * 2];
  int i; 
  for(i = 0; i < taps * 2; i++) prefix_buf[i] = std::complex<float>(0.0,0.0);

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

  gain_correction = ((float) L) / fsum;
}

int SoDa::TDRationalResampler::apply(std::complex<float> *in, std::complex<float> * out, 
			       int inlen, int max_outlen)
{
  // Using the terminology from Lyons pp 541 -- note that we use the
  // recurrence sum in equation 10-20'' rather than the fancy diagram
  // with the separate shift registers.  
  std::complex<float> czero(0.0,0.0);
  std::complex<float> rsum; 

  std::complex<float> * x; 
  x = &prefix_buf[taps];
  // we need a prefix buffer for the "old" samples before in[n]
  memcpy(x, in, sizeof(std::complex<float>) * taps);

  int m; 
  // first consume the prefix buffer, then the input vector
  for(m = 0; (m < max_outlen) && (n < inlen); m++) {
    rsum = czero; 
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

void SoDa::TDRationalResampler::bumpCounters() 
{
    // bump k and n    
    k += M; 
    while(k >= L) {
      k = k - L; 
      n++; 
    }
}

float SoDa::TDResampler625x48::HCLPF35_5x1_125[] = { // fs 625 fc 0.04 sinc exp 1.4 hcos 0.65 taps 35
-79.95470811197196780E-6,
-258.4136429563059210E-6,
-479.6063191113369730E-6,
-609.6952500588733980E-6,
-421.1386179694352450E-6,
 398.5839337199342940E-6,
 0.002213476082222032,
 0.005382171324155426,
 0.010185043627898772,
 0.016747971431142213,
 0.024978635896736039,
 0.034531107827123438,
 0.044810626816378672,
 0.055023402579374200,
 0.064267331931922689,
 0.071650663452773628,
 0.076418918521976187,
 0.078067467979207481,
 0.076418918521976187,
 0.071650663452773628,
 0.064267331931922689,
 0.055023402579374200,
 0.044810626816378672,
 0.034531107827123438,
 0.024978635896736039,
 0.016747971431142213,
 0.010185043627898772,
 0.005382171324155426,
 0.002213476082222032,
 398.5839337199342940E-6,
-421.1386179694352450E-6,
-609.6952500588733980E-6,
-479.6063191113369730E-6,
-258.4136429563059210E-6,
-79.95470811197196780E-6
};

float SoDa::TDResampler625x48::PMLPF30_5x3_75[] = { // 375e3 Fc 0.093 (17.k) kaiser 1.8 tw 0.070
-0.008550141894823119,
-0.005518626810191463,
-0.006290314138607590,
-0.005662734314818437,
-0.002907061686776321,
 0.002621715205463082,
 0.011352114647169447,
 0.023355831510871054,
 0.038241364278054447,
 0.055159164808336623,
 0.072885624666934365,
 0.089821114007866271,
 0.104382980638500197,
 0.115048590524805136,
 0.120688416287224584,
 0.120688416287224584,
 0.115048590524805136,
 0.104382980638500197,
 0.089821114007866271,
 0.072885624666934365,
 0.055159164808336623,
 0.038241364278054447,
 0.023355831510871054,
 0.011352114647169447,
 0.002621715205463082,
-0.002907061686776321,
-0.005662734314818437,
-0.006290314138607590,
-0.005518626810191463,
-0.008550141894823119
  };

float SoDa::TDResampler625x48::PMLPF32Sinc_5x4_60[] = { // S 300 fc 0.111 sinc exp 0.75 taps 32 tw 0.03
 -0.004286305539756133,
-0.005257963294732116,
-0.008875421842123881,
-0.012353668952351420,
-0.014593963939664516,
-0.014282670144634196,
-0.010127222739246551,
-0.001166393079653640,
 0.013081928891899864,
 0.032345636972765296,
 0.055453251327708938,
 0.080592635832372764,
 0.105289233533623655,
 0.126907844112932738,
 0.142942877519359357,
 0.151478957433987149,
 0.151478957433987149,
 0.142942877519359357,
 0.126907844112932738,
 0.105289233533623655,
 0.080592635832372764,
 0.055453251327708938,
 0.032345636972765296,
 0.013081928891899864,
-0.001166393079653640,
-0.010127222739246551,
-0.014282670144634196,
-0.014593963939664516,
-0.012353668952351420,
-0.008875421842123881,
-0.005257963294732116,
-0.004286305539756133
};

float SoDa::TDResampler625x48::PMLPF32_5x4_60[] = { // S 300 fc 0.111 kb 1.2 taps 32 tw 0.06
    // just a little bit of a boost around 10 kHz to compensate for
    // attenuation in earlier filters
-0.012256520683827698,
-0.012270991152492642,
-0.016861141721530613,
-0.020713509940997944,
-0.022738328919073420,
-0.021811753486358797,
-0.016958021067226721,
-0.007496267845164849,
 0.006646319039374624,
 0.024997089422824491,
 0.046391470734625823,
 0.069109441489325713,
 0.091052551141374533,
 0.110005509976700250,
 0.123936990548916146,
 0.131321189961315948,
 0.131321189961315948,
 0.123936990548916146,
 0.110005509976700250,
 0.091052551141374533,
 0.069109441489325713,
 0.046391470734625823,
 0.024997089422824491,
 0.006646319039374624,
-0.007496267845164849,
-0.016958021067226721,
-0.021811753486358797,
-0.022738328919073420,
-0.020713509940997944,
-0.016861141721530613,
-0.012270991152492642,
-0.012256520683827698
  };

float SoDa::TDResampler625x48::PMLPF52_5x4_48[] = { // fs 240 fc 0.075 kb 7 nt 52 tw 0.02  not good enough
  // fs 24 fc 0.075 sinc exp 2.15 taps 52 tw 0.02
  5.565630211855049580E-6,
-110.0953615890899900E-6,
-197.9533679901807370E-6,
-423.8330314256159000E-6,
-843.7651288417345090E-6,
-0.001499440985286912,
-0.002408357217938762,
-0.003547156665944404,
-0.004831751266908320,
-0.006108555300740413,
-0.007146306709394228,
-0.007651700334659528,
-0.007281438189406613,
-0.005691851651280670,
-0.002571204285239696,
 0.002303652864315378,
 0.009067543190077293,
 0.017615850592697727,
 0.027683457128813635,
 0.038863856027995990,
 0.050510295968668052,
 0.061922221155652386,
 0.072284940655570701,
 0.080859732332517725,
 0.086981186848117037,
 0.090170207059810839,
 0.090170207059810839,
 0.086981186848117037,
 0.080859732332517725,
 0.072284940655570701,
 0.061922221155652386,
 0.050510295968668052,
 0.038863856027995990,
 0.027683457128813635,
 0.017615850592697727,
 0.009067543190077293,
 0.002303652864315378,
-0.002571204285239696,
-0.005691851651280670,
-0.007281438189406613,
-0.007651700334659528,
-0.007146306709394228,
-0.006108555300740413,
-0.004831751266908320,
-0.003547156665944404,
-0.002408357217938762,
-0.001499440985286912,
-843.7651288417345090E-6,
-423.8330314256159000E-6,
-197.9533679901807370E-6,
-110.0953615890899900E-6,
 5.565630211855049580E-6
}; 
float SoDa::TDResampler625x48::PMLPF40_5x4_48[] = { // fs 240 fc 0.1 kb 5.6 nt 40 tw 0.02
-469.8570631597345940E-6,
-593.2038044152884600E-6,
-0.001258374877009809,
-0.002296416989879194,
-0.003716163497883330,
-0.005395110355155924,
-0.007039202801442299,
-0.008163396266852229,
-0.008132577862202204,
-0.006170230224354074,
-0.001540560650014034,
 0.006291724912209862,
 0.017604041713808118,
 0.032107941575157444,
 0.049080882768190400,
 0.067210088430114195,
 0.084860972044559460,
 0.100205322035463104,
 0.111573942386506003,
 0.117613463329419854,
 0.117613463329419854,
 0.111573942386506003,
 0.100205322035463104,
 0.084860972044559460,
 0.067210088430114195,
 0.049080882768190400,
 0.032107941575157444,
 0.017604041713808118,
 0.006291724912209862,
-0.001540560650014034,
-0.006170230224354074,
-0.008132577862202204,
-0.008163396266852229,
-0.007039202801442299,
-0.005395110355155924,
-0.003716163497883330,
-0.002296416989879194,
-0.001258374877009809,
-593.2038044152884600E-6,
-469.8570631597345940E-6
};

SoDa::TDResampler625x48::TDResampler625x48() :
    TDFilter("TDResampler625x48")
{
  lastoutlen = 0; 
  ibuf51 = ibuf53 = ibuf54a = NULL; 

  rs51_p = new SoDa::TDRationalResampler(5, 1, HCLPF35_5x1_125, 35);
  rs53_p = new SoDa::TDRationalResampler(5, 3, PMLPF30_5x3_75, 30);
  // rs54a_p = new SoDa::TDRationalResampler(5, 4, PMLPF32Sinc_5x4_60, 32);  
  rs54a_p = new SoDa::TDRationalResampler(5, 4, PMLPF32_5x4_60, 32);
  rs54b_p = new SoDa::TDRationalResampler(5, 4, PMLPF40_5x4_48, 40);
  // rs54b_p = new SoDa::TDRationalResampler(5, 4, PMLPF52_5x4_48, 52);        
}

void SoDa::TDResampler625x48::allocateIBufs(int outlen)
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
    ibuf51 = new std::complex<float>[len51];
    ibuf53 = new std::complex<float>[len53];
    ibuf54a = new std::complex<float>[len54a];    
  }
}

int SoDa::TDResampler625x48::apply(std::complex<float> * in, 
				   std::complex<float> * out, 
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
