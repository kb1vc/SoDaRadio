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
#include "TDResamplers625x48.hxx"
#include "ReSampler.hxx"
#include "ReSamplers625x48.hxx"

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


int main(int argc, char * argv[])
{
  int itercount = 5000;
  std::complex<float> cin[60000], cout[2304];
  int i; 
  double ang; 
  for(i = 0; i < 60000; i++) {
    cin[i] = std::complex<float>(sin(ang), cos(ang)); 
    ang += 0.01;
  }
 
  SoDa::TDResampler625x48<std::complex<float> >  rs625x48; 
  SoDa::ReSample625to48 rsc(30000);

  if((argc < 2) || (argv[0][0] == 't')) {
    for(i = 0; i < itercount; i++) {
      rs625x48.apply(cin, cout, 30000, 2304);
    }
  }
  else {
    for(i = 0; i < itercount; i++) {
      rsc.apply(cin, cout);
    }
  }
}

