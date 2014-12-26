/*
Copyright (c) 2014, Matthew H. Reilly (kb1vc)
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

#define SAMPLE_RATE 625000
#include <complex>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <boost/format.hpp>

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "DCBlock.hxx"

using namespace std;
#include <sys/time.h>


double curtime()
{
  double ret;
  struct timeval tp; 
  gettimeofday(&tp, NULL); 
  ret = ((double) tp.tv_sec) + 1e-6*((double)tp.tv_usec);

  return ret; 
}

#define NUMSEGS 16
#define SEGLEN 1024
#define VECLEN (NUMSEGS * SEGLEN)

template<typename Tv, typename Ta> void doTest(Tv * ref_in, Tv * in, Tv * out, const Tv & off, int len, SoDa::DCBlock<Tv, Ta> * block, char ID)
{
  std::cerr << boost::format("block alpha = %f\n") % block->alpha;
  std::cerr << block->Dm1 << std::endl; 
  block->apply(in, out, len);
}

int doTest1(int argc, char * argv[])
{
  // Test out the DCBlock (DC offset removal) module
  int i;
  float ftest_in[VECLEN], ftest_offset_in[VECLEN], ftest_out[VECLEN];
  std::complex<float> ctest_in[VECLEN], ctest_offset_in[VECLEN], ctest_out[VECLEN];
  float foffset = 0.2;
  std::complex<float> coffset(0.2, 0.3);
  
  float ang = 0.0;
  float anginc = M_PI / 6.0;
  
  // fill in the input vectors  
  for(i = 0; i < VECLEN; i++) {
    ftest_in[i] = sin(ang);
    ctest_in[i] = std::complex<float>(cos(ang), sin(ang));
    ftest_offset_in[i] = ftest_in[i] + foffset;
    ctest_offset_in[i] = ctest_in[i] + coffset;
    ang += anginc;
    if(ang > M_PI) ang = ang - 2.0 * M_PI; 
  }

  SoDa::DCBlock<float, float> fblock(0.99); 
  SoDa::DCBlock<std::complex<float>, float> cblock(0.99);
  
  for(i = 0; i < VECLEN; i += SEGLEN) {
    doTest(ftest_in + i, ftest_offset_in + i, ftest_out + i, foffset, SEGLEN, &fblock, 'F');
    doTest(ctest_in + i, ctest_offset_in + i, ctest_out + i, coffset, SEGLEN, &cblock, 'C');
  }

  ofstream ofi("DCBlock_Test.out");
  int idxoff = 3; 
  for(i = (idxoff+1); i < VECLEN; i++) {
    float fo = (ftest_out[i-idxoff] + ftest_out[(i-idxoff) - 1]) * 0.5;
    std::complex<float> cfo = (ctest_out[i-idxoff] + ctest_out[(i-idxoff) - 1]) * std::complex<float>(0.5, 0.0);
    ofi << i << " " << ftest_in[i] << " " << ftest_offset_in[i] << " " << fo
	<< " " << ctest_in[i].real() << " " << ctest_in[i].imag()
	<< " " << ctest_offset_in[i].real() << " " << ctest_offset_in[i].imag()
	<< " " << cfo.real() << " " << cfo.imag()
	<< std::endl; 
  }
  
  ofi.close(); 

}

int doTest2(int argc, char * argv[])
{
  // drive the filter with varying frequencies from 1Hz to 1MHz...
  // measure the phase and amplitude response.
  //
  float alpha; 
  if(argc < 2) {
    alpha = 0.99; 
  }
  else {
    alpha = atof(argv[1]);
  }
  std::cout << boost::format("# DC block test alpha = %f\n") % alpha;
  std::cout << "# freq (Hz)  gain (dB)   phase shift (rad)\n"; 
  int i;
  std::complex<float> ctest_in[VECLEN], ctest_offset_in[VECLEN], ctest_out[VECLEN];
  SoDa::DCBlock<std::complex<float>, float> cblock(alpha);
  
  // assume a 1.0 MHz sample rate. 
  double mults[] = {1.0, 2.0, 3.0, 5.0, 7.0, 8.0, 9.0, 0.0}; 
  for(double basefreq = 1.0e-2; basefreq < 0.99e6; basefreq *= 10.0) {
    int fi; 
    for(fi = 0; mults[fi] != 0.0; fi++) {
      double freq = basefreq * mults[fi];
      
      float ang = 0.0;
      float anginc = 2.0 * M_PI * freq / 1.0e6 ;
  
      // fill in the input vectors  
      for(i = 0; i < VECLEN; i++) {
	ctest_in[i] = std::complex<float>(cos(ang), sin(ang));
	ang += anginc;
	if(ang > M_PI) ang = ang - 2.0 * M_PI; 
      }

     
      cblock.apply(ctest_in, ctest_out, VECLEN); 

      // now plot the phase and amplitude for this frequency.
      float ampsq_sum = 0.0;
      float phase_sum = 0.0;
      float divisor = ((float) (VECLEN / 2)); 
      for(i = VECLEN / 2; i < VECLEN; i++) {
	ampsq_sum += ctest_out[i].real() * ctest_out[i].real() + ctest_out[i].imag() * ctest_out[i].imag();
	float p1 = atan2(ctest_in[i].imag(), ctest_in[i].real());
	float p2 = atan2(ctest_out[i].imag(), ctest_out[i].real());
	float phdiff = p1 - p2;
	if(phdiff > M_PI) phdiff - 2.0 * M_PI; 
	if(phdiff <  -M_PI) phdiff + 2.0 * M_PI; 
	phase_sum = phdiff; 
      }

      float phase = phase_sum / divisor;
      float logamp = 10.0 * log10(ampsq_sum / divisor); 
      std::cout << boost::format("%12.9g %g %5.3f\n") % freq % logamp % phase; 

    }
  }
}


int main(int argc, char * argv[])
{
  doTest2(argc, argv); 
}
