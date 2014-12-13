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

int main(int argc, char * argv[])
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

