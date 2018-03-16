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
#include <complex>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "TDReSamplers625x48.hxx"
#include <time.h>
#include <fftw3.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#include <sys/time.h>

/* 
 So the filter design scheme here is: 

   5:1 - 5:3 - 5:4 - 5:4 
625   125    75    60    48

We need to filter based on the upsampled input.  
Our end goal is a passband of about -10kHz to 10kHz. 

5:1 Fc > 10kHz  Fs < 125 - 10 = 105 kHz
5:3 Fc > 10kHz  Fs < 75 - 10 = 65 kHz
5:4 Fc > 10kHz  Fs < 60 - 10 = 50 kHz
5:4 Fc > 10kHz  Fs < 48 - 10 = 38 kHz

Using TFilter 
5:1 20kHz - 100 kHz 19 taps 
*/ 
/*

FIR filter designed with
http://t-filter.appspot.com

sampling frequency: 625 Hz

* 0 Hz - 20 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = 0.808656068661126 dB

* 100 Hz - 312.5 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = -54.10102681327493 dB

*/

float RS_5x1_125_19[] = {
  -0.002925476960621446,
  -0.004670486683100547,
  -0.0038498114655096715,
  0.00399998787993632,
  0.022462282059162928,
  0.052311390082398865,
  0.08993455713740346,
  0.12755019133115594,
  0.155514499653266,
  0.16586212520410482,
  0.155514499653266,
  0.12755019133115594,
  0.08993455713740346,
  0.052311390082398865,
  0.022462282059162928,
  0.00399998787993632,
  -0.0038498114655096715,
  -0.004670486683100547,
  -0.002925476960621446
};

/*

FIR filter designed with
http://t-filter.appspot.com

sampling frequency: 375 Hz

* 0 Hz - 15 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = 0.8239376406453897 dB

* 65 Hz - 187.5 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = -53.93865605746788 dB

*/

float RS_5x3_75_18[] = {
  -0.003750587722558265,
  -0.007353525151250815,
  -0.008104985716413791,
  0.0005614872849629704,
  0.024205431718945007,
  0.06363491226559038,
  0.11232616261141509,
  0.15745688417767179,
  0.18472128538192176,
  0.18472128538192176,
  0.15745688417767179,
  0.11232616261141509,
  0.06363491226559038,
  0.024205431718945007,
  0.0005614872849629704,
  -0.008104985716413791,
  -0.007353525151250815,
  -0.003750587722558265
};

/*

FIR filter designed with
http://t-filter.appspot.com

sampling frequency: 240 Hz

* 0 Hz - 15 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = 1.1972781541559958 dB

* 38 Hz - 120 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = -50.69986968390401 dB

*/

float RS_5x4_60_20[] = {
  -0.005456684634506641,
  -0.011818383259299351,
  -0.018046703627969358,
  -0.017977275781750852,
  -0.005002344349502569,
  0.025021448776127653,
  0.07062610347772966,
  0.12355162773891792,
  0.170641410065091,
  0.19843000371378033,
  0.19843000371378033,
  0.170641410065091,
  0.12355162773891792,
  0.07062610347772966,
  0.025021448776127653,
  -0.005002344349502569,
  -0.017977275781750852,
  -0.018046703627969358,
  -0.011818383259299351,
  -0.005456684634506641
};

/*

FIR filter designed with
http://t-filter.appspot.com

sampling frequency: 300 Hz

* 0 Hz - 15 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = 0.6387135710766937 dB

* 50 Hz - 150 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = -56.147823014760846 dB

*/

float RS_5x4_48_20[] = {
  -0.0035824960509120707,
  -0.008815496897252863,
  -0.014349200035012244,
  -0.014864359772662813,
  -0.0038089152636523885,
  0.023552008695536183,
  0.06663547625356776,
  0.11782886002131132,
  0.1640924076535803,
  0.1916392158258477,
  0.1916392158258477,
  0.1640924076535803,
  0.11782886002131132,
  0.06663547625356776,
  0.023552008695536183,
  -0.0038089152636523885,
  -0.014864359772662813,
  -0.014349200035012244,
  -0.008815496897252863,
  -0.0035824960509120707
};



// These filters all came from the Iowa filter design program. 

  float LPF_5x1_125[] = {
    0.001500294898679822,
    0.006480544793236022,
    0.018497121952418410,
    0.039869820738689540,
    0.069496269387148854,
    0.101469526877781727,
    0.126511424870685230,
    0.136025700328422067,
    0.126511424870685230,
    0.101469526877781727,
    0.069496269387148854,
    0.039869820738689540,
    0.018497121952418410,
    0.006480544793236022,
    0.001500294898679822
  };


// spec: input freq is 125kS -- upsample by 3, so
//  proto filter is at 375kS corner at 18
// output rate is 75kHz, our ultimate passband is 10kHz, so put
// the stop band at 75 - 10 or 65kHz
// Iowa Hills sf 375 fc 0.121 kaiser b 0.8 tw 0.295 taps = 24 PM
//
float LPF24_5x3_75[] = {
  -806.2327165814780300E-6,
-0.002531644043415368,
-0.005229529199137091,
-0.007762166253497533,
-0.007691042156497170,
-0.001662621909547398,
 0.013397738638423955,
 0.038647512070262985,
 0.071986665523786644,
 0.107821765551824905,
 0.138329769163476224,
 0.155918349896981806,
 0.155918349896981806,
 0.138329769163476224,
 0.107821765551824905,
 0.071986665523786644,
 0.038647512070262985,
 0.013397738638423955,
-0.001662621909547398,
-0.007691042156497170,
-0.007762166253497533,
-0.005229529199137091,
-0.002531644043415368,
-806.2327165814780300E-6

}; 
  // With Iowa Hills FIR designer v 7.0,
  // sample freq: 625e3 Fc 0.06 (18.75kHz Sinc exp 0.9 window: sinc trans width 0.035
  // taps 15
  float badLPF15_5x3_75[] = {
 0.008907500394959741,
 0.014407599593438793,
 0.028478192985622614,
 0.046167585466025216,
 0.065148229289235332,
 0.082275042414834465,
 0.094170987885127497,
 0.098514146747789994,
 0.094170987885127497,
 0.082275042414834465,
 0.065148229289235332,
 0.046167585466025216,
 0.028478192985622614,
 0.014407599593438793,
 0.008907500394959741
  };

float HCLPF35_5x1_125[] = { // fs 625 fc 0.04 sinc exp 1.4 hcos 0.65 taps 35
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

float PMLPF18_5x1_125[] = { // PM s 625k fc 0.04 taps 18 kb 1.2 tw 0.09
 0.017364937709126067,
 0.024036302675903978,
 0.034533541716883762,
 0.052023020690899570,
 0.066409367532996880,
 0.084215367352765899,
 0.096969678275655225,
 0.108316763602503849,
 0.113425191168277276,
 0.113425191168277276,
 0.108316763602503849,
 0.096969678275655225,
 0.084215367352765899,
 0.066409367532996880,
 0.052023020690899570,
 0.034533541716883762,
 0.024036302675903978,
 0.017364937709126067
};

  float badPMLPF24_5x3_75[] = { // 625e3 Fc 0.04 (12.5k) KBeta 7 kaiser tw 0.02 24 taps 
 0.001084233711426047,
 0.001449150039364076,
 0.003463928384692594,
 0.006999438506155273,
 0.012483955746201919,
 0.020101551176672703,
 0.029707852615402996,
 0.040683838154194560,
 0.051989503508422276,
 0.062188069118076485,
 0.070014963532862212,
 0.074246082051230677,
 0.074246082051230677,
 0.070014963532862212,
 0.062188069118076485,
 0.051989503508422276,
 0.040683838154194560,
 0.029707852615402996,
 0.020101551176672703,
 0.012483955746201919,
 0.006999438506155273,
 0.003463928384692594,
 0.001449150039364076,
 0.001084233711426047
  };

// set the prototype filter designing for its <input> rate.
// the filter null is at the input rate / decimation  - passband.
// Design the filter at Fs * L (upsampled) rate
// Design for cutoff Fc / L  where Fc is Fnew / 2
// So Fnew is 75kHz, cutoff 37.5/3 or about 12
// so the null here is at 125K / 5 -> 25K - 9k or about 16KHz.
  float PMLPF30_5x3_75[] = { // 375e3 Fc 0.093 (17.k) kaiser 1.8 tw 0.070
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

  float PMLPF32SINC_5x4_60[] { // S 300 fc 0.111 sinc exp 0.9 taps 32 tw 0.02
    // flat in passband, but heavy rolloff up to about 50kHz
-0.003548844450242179,
-0.004450677677432949,
-0.007863251218326867,
-0.011255146458244091,
-0.013498047054977948,
-0.013227037878095999,
-0.009052252850271537,
 136.8924257660404460E-6,
 0.014979865636593918,
 0.035201630321134436,
 0.059658825491670960,
 0.086453227243491401,
 0.112889761257868468,
 0.136137088984847943,
 0.153424343986816425,
 0.162670716255962938,
 0.162670716255962938,
 0.153424343986816425,
 0.136137088984847943,
 0.112889761257868468,
 0.086453227243491401,
 0.059658825491670960,
 0.035201630321134436,
 0.014979865636593918,
 136.8924257660404460E-6,
-0.009052252850271537,
-0.013227037878095999,
-0.013498047054977948,
-0.011255146458244091,
-0.007863251218326867,
-0.004450677677432949,
-0.003548844450242179
  }; 
  float PMLPF32_5x4_60[] { // S 300 fc 0.111 kb 1.2 taps 32 tw 0.06
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
  float PMLPF20_5x4_60[] { // s 300 fc 9kHz kb 1.8 taps 20 tw 0.02
 0.003923853884379387,
 0.039517038982476979,
 0.033456711656097787,
 0.051763408958330807,
 0.069100260860716203,
 0.087436661165049362,
 0.104882700727168834,
 0.119820454403670715,
 0.130763954891474576,
 0.136570210218566523,
 0.136570210218566523,
 0.130763954891474576,
 0.119820454403670715,
 0.104882700727168834,
 0.087436661165049362,
 0.069100260860716203,
 0.051763408958330807,
 0.033456711656097787,
 0.039517038982476979,
 0.003923853884379387
  };

float PMLPF40_5x4_48[] = { // fs 240 fc 0.1 kb 5.6 nt 40 tw 0.02
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

float PMLPF32_5x4_48[] = { // fs 240 fc 0.1 kb 2.2 tw 0.045
-0.010158168048843043,
-0.007206685245003190,
-0.009186769494655517,
-0.010194345409888296,
-0.009432850355636433,
-0.006069257254338015,
 596.5849076808574410E-6,
 0.010982005539983169,
 0.025043845486313464,
 0.042324181142268769,
 0.061811711631029012,
 0.082029700921414622,
 0.101285320436520596,
 0.117730785348387129,
 0.129747553992407116,
 0.136091567629862104,
 0.136091567629862104,
 0.129747553992407116,
 0.117730785348387129,
 0.101285320436520596,
 0.082029700921414622,
 0.061811711631029012,
 0.042324181142268769,
 0.025043845486313464,
 0.010982005539983169,
 596.5849076808574410E-6,
-0.006069257254338015,
-0.009432850355636433,
-0.010194345409888296,
-0.009186769494655517,
-0.007206685245003190,
-0.010158168048843043
}; 
  
 // test program to checkout the antialiasing capability of the
 // resamplers. 

using namespace std; 

void doFreqTest(SoDa::TDFilter & filt, const std::string & fbase_name, 
		float anginc, 
		int inlen, int outlen) 
{
  std::complex<float> invec[inlen];
  std::complex<float> outvec[outlen];

  std::string ifname = fbase_name + "_in.dat";
  std::string ofname = fbase_name + "_out.dat";
  ofstream inf(ifname.c_str());  
  ofstream outf(ofname.c_str());

  
  float ang = 0.0; 
  for(int i = 0; i < inlen; i++) {
    invec[i] = std::exp(std::complex<float>(0.0, ang));
    ang += anginc; 
    if(ang > M_PI) ang -= 2.0*M_PI; 
    inf << boost::format("%d %g %g %g\n")
      % i % invec[i].real() % invec[i].imag() % abs(invec[i]);
  }
    
  int len;
  int first = 3 * inlen / 7; 
  int second = inlen - first;
  len = filt.apply(invec, outvec, first, outlen);
  len += filt.apply(&invec[first], &outvec[len], second, outlen - len);  

  std::cerr << boost::format("%s: inlen = %d output length = %d\n") % fbase_name % inlen % len; 
  for(int i = 0; i < len; i++) {
    outf << boost::format("%d %g %g %g\n")
      % i % outvec[i].real() % outvec[i].imag() % abs(outvec[i]);
  }

  outf.close();
  inf.close();
}

void doSweepTest(SoDa::TDFilter & filt, const std::string & fname, int inlen, int outlen) 
{
  std::complex<float> invec[inlen];
  std::complex<float> outvec[outlen];

  ofstream outf(fname.c_str());

  
  for(float anginc = -M_PI; anginc < M_PI; anginc += 0.01) {
    // load the invec
    float ang = 0.0; 
    for(int i = 0; i < inlen; i++) {
      invec[i] = std::exp(std::complex<float>(0.0, ang));
      ang += anginc; 
      if(ang > M_PI) ang -= 2.0*M_PI;
      if(ang < -M_PI) ang += 2.0*M_PI;       
    }
    
    int len;
    len = filt.apply(invec, outvec, inlen, outlen);
    float mina, maxa; 
    maxa = 0.0; mina = 1e9;
    for(int i = len / 2; i < len; i++) {
      float mag = abs(outvec[i]);
      mina = (mina > mag) ? mag : mina; 
      maxa = (maxa < mag) ? mag : maxa;
    }

    outf << boost::format("%g %g %g\n") % anginc % mina % maxa; 
  }

  outf.close();
}

void doSweepCascade(SoDa::TDFilter & filt1, SoDa::TDFilter & filt2, const std::string & fname, int inlen, int outlen) 
{
  std::complex<float> invec[inlen];
  std::complex<float> s2vec[inlen];  
  std::complex<float> outvec[outlen];

  ofstream outf(fname.c_str());

  
  for(float anginc = -M_PI; anginc < M_PI; anginc += 0.01) {
    // load the invec
    float ang = 0.0; 
    for(int i = 0; i < inlen; i++) {
      invec[i] = std::exp(std::complex<float>(0.0, ang));
      ang += anginc; 
      if(ang > M_PI) ang -= 2.0*M_PI;
      if(ang < -M_PI) ang += 2.0*M_PI;       
    }
    
    int len;
    len = filt1.apply(invec, s2vec, inlen, outlen);
    len = filt2.apply(s2vec, outvec, len, outlen);    
    float mina, maxa; 
    maxa = 0.0; mina = 1e9;
    for(int i = len / 2; i < len; i++) {
      float mag = abs(outvec[i]);
      mina = (mina > mag) ? mag : mina; 
      maxa = (maxa < mag) ? mag : maxa;
    }

    outf << boost::format("%g %g %g\n") % anginc % mina % maxa; 
  }

  outf.close();
}

void doSweepChain(SoDa::TDFilter ** filters, int num_filters, const std::string & fname, int inlen, int outlen) 
{
  std::complex<float> invec[inlen];
  std::complex<float> between[inlen];  
  std::complex<float> outvec[outlen];

  ofstream outf(fname.c_str());

  
  for(float anginc = -M_PI; anginc < M_PI; anginc += 0.01) {
    // load the invec
    float ang = 0.0; 
    for(int i = 0; i < inlen; i++) {
      invec[i] = std::exp(std::complex<float>(0.0, ang));
      ang += anginc; 
      if(ang > M_PI) ang -= 2.0*M_PI;
      if(ang < -M_PI) ang += 2.0*M_PI;       
    }
    
    int len = inlen;
    for(int i = 0; i < num_filters; i++) {
      len = filters[i]->apply(invec, between, len, outlen);
      memcpy(invec, between, sizeof(std::complex<float>) * len); 
    }
    memcpy(outvec, between, sizeof(std::complex<float>) * len);
    
    float mina, maxa; 
    maxa = 0.0; mina = 1e9;
    for(int i = len / 2; i < len; i++) {
      float mag = abs(outvec[i]);
      mina = (mina > mag) ? mag : mina; 
      maxa = (maxa < mag) ? mag : maxa;
    }

    outf << boost::format("%g %g %g\n") % anginc % mina % maxa; 
  }

  outf.close();
}


int main(int argc, char * argv[])
{
  (void) argc; (void) argv; 


  SoDa::TDDecimator decimate5(5, LPF_5x1_125, 15);
  // SoDa::TDRationalResampler rs53_75(5, 3, LPF24_5x3_75, 24); // not good
  // SoDa::TDRationalResampler rs53_75(5, 3, RCLPF24_5x3_75, 24);
  SoDa::TDRationalResampler rs53_75(5, 3, PMLPF30_5x3_75, 30);  
  
  //  SoDa::TDRationalResampler rs51_125(5, 1, LPF_5x1_125, 15);
  SoDa::TDRationalResampler rs51_125(5, 1, HCLPF35_5x1_125, 35);  
  //  SoDa::TDRationalResampler rs51_125(5, 1, PMLPF18_5x1_125, 18);
  SoDa::TDRationalResampler rs54_60(5, 4, PMLPF32_5x4_60, 32);
  // SoDa::TDRationalResampler rs54_60(5, 4, PMLPF32SINC_5x4_60, 32);  
  //SoDa::TDRationalResampler rs54_48(5, 4, PMLPF32_5x4_48, 32);
  SoDa::TDRationalResampler rs54_48(5, 4, PMLPF40_5x4_48, 40);  
 
  SoDa::TDResampler625x48 rs625x48; 

  // doSweepTest(decimate5, "decimate5_resp.dat", 500, 100);
  // doFreqTest(decimate5, "decimate5_freq", 0.01, 500, 100);

  doSweepTest(rs625x48, "rs625x48.dat", 62500, 4800); 
  doSweepTest(rs53_75, "rs53_75_resp.dat", 5000, 3000);
  doSweepTest(rs54_60, "rs54_60_resp.dat", 5000, 4000);
  doSweepTest(rs54_48, "rs54_48_resp.dat", 5000, 4000);  
  // doFreqTest(rs53_75, "rs53_75_2r2_freq", 2.2, 500, 300);  
  // doFreqTest(rs53_75, "rs53_75_1r1_freq", 1.1, 500, 300);
  // doFreqTest(rs53_75, "rs53_75_0r1_freq", 0.1, 500, 300);
  // doFreqTest(rs53_75, "rs53_75_0r01_freq", 0.01, 500, 300);    

  doSweepTest(rs51_125, "rs51_125_resp.dat", 5000, 1000);
  // doFreqTest(rs51_125, "rs51_125_1r1_freq", 1.1, 500, 300);
  // doFreqTest(rs51_125, "rs51_125_0r1_freq", 0.1, 500, 300);
  // doFreqTest(rs51_125, "rs51_125_0r01_freq", 0.01, 500, 300);

  SoDa::TDFilter * resamp_chain[4];
  resamp_chain[0] = &rs51_125; 
  resamp_chain[1] = &rs53_75;
  resamp_chain[2] = &rs54_60;
  resamp_chain[3] = &rs54_48;
  
  doSweepChain(resamp_chain, 1, "rsChain_resp1.dat", 5000, 3000);
  doSweepChain(resamp_chain, 2, "rsChain_resp2.dat", 5000, 3000);
  doSweepChain(resamp_chain, 3, "rsChain_resp3.dat", 5000, 3000);
  doSweepChain(resamp_chain, 4, "rsChain_resp.dat", 5000, 3000);  
  

  // SoDa::TDFilter * tfilt_chain[4];
    
  // tfilt_chain[0] = new SoDa::TDDecimator(5, RS_5x1_125_19, 19);
  // tfilt_chain[1] = new SoDa::TDRationalResampler(5, 3, RS_5x3_75_18, 18);
  // tfilt_chain[2] = new SoDa::TDRationalResampler(5, 4, RS_5x4_60_20, 20);
  // tfilt_chain[3] = new SoDa::TDRationalResampler(5, 4, RS_5x4_48_20, 20);

  // doSweepChain(tfilt_chain, 4, "TFChain_resp.dat", 50000, 30000);
  // doSweepChain(tfilt_chain, 3, "TFChain_resp3.dat", 50000, 30000);
  // doSweepChain(tfilt_chain, 2, "TFChain_resp2.dat", 50000, 30000);
  // doSweepChain(tfilt_chain, 1, "TFChain_resp1.dat", 50000, 30000);

  // SoDa::TDFilter * hybrid_chain[4];  
  // hybrid_chain[0] = new SoDa::TDDecimator(5, RS_5x1_125_19, 19);
  // hybrid_chain[1] = new SoDa::TDRationalResampler(5, 3, RS_5x3_75_18, 18);
  // hybrid_chain[2] = new SoDa::TDRationalResampler(5, 4, PMLPF15_5x4_60, 15);
  // hybrid_chain[3] = new SoDa::TDRationalResampler(5, 4, PMLPF15_5x4_60, 15);

  // doSweepChain(hybrid_chain, 4, "HBChain_resp.dat", 50000, 30000);
  // doSweepChain(hybrid_chain, 3, "HBChain_resp3.dat", 50000, 30000);
  // doSweepChain(hybrid_chain, 2, "HBChain_resp2.dat", 50000, 30000);
  // doSweepChain(hybrid_chain, 1, "HBChain_resp1.dat", 50000, 30000);
  
}

