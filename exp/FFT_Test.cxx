/*
Copyright (c) 2012, Matthew H. Reilly (kb1vc)
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
#define SAMPLE_RATE 48000
#include <complex>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <fftw3.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;
#include <sys/time.h>


// test program to compare the cost of various FFT vector sizes. 

using namespace std; 

double curtime()
{
  double ret;
  struct timeval tp; 
  gettimeofday(&tp, NULL); 
  ret = ((double) tp.tv_sec) + 1e-6*((double)tp.tv_usec);
  return ret; 
}

complex<float> doWork(complex<float> * in, complex<float> * out, unsigned int vsize)
{
  complex<float> ret(0.0,0.0);
  // create a plan.
  fftwf_plan plan = fftwf_plan_dft_1d(vsize,  (fftwf_complex*) in, (fftwf_complex*) out,
				      FFTW_FORWARD, FFTW_ESTIMATE);

  // timestamp
  double startt = curtime(); 
    
  // do 100 FFTs
  for(int i = 0; i < 100; i++) {
    fftwf_execute(plan);
    ret += out[3]; 
  }
  // timestamp
  double endt = curtime();
  
  //report
  double ttime = (endt - startt)/100;
  double time_per_point = ttime / ((double) vsize); 
  printf("%d %g %g\n", vsize, ttime, time_per_point);

  // destroy plan
  fftwf_destroy_plan(plan);

  return ret; 
}

int main(int argc, char * argv[])
{
  (void) argc; (void) argv; 
  // try various sizes of FFT inputs from powers of two between 8 and 32K
  // and for multiples of 2^N * 3 from 48 through 49152  (48 * 1024)

  static int maxbuf = 1000000;
  complex<float> * invec; // really big buffers
  complex<float> * outvec;

  invec = (complex<float>*) fftwf_malloc(maxbuf * sizeof(complex<float>));
  outvec = (complex<float>*) fftwf_malloc(maxbuf * sizeof(complex<float>));

  // fill in the input vector;
  float anginc = M_PI * 20.0;
  float ang = 0.0; 
  int i;
  for(i = 0; i < maxbuf; i++) {
    invec[i] = complex<float>(cos(ang), sin(ang));
    ang += anginc;
    if(ang > M_PI) ang -= (2.0 * M_PI); 
  }

  // now create and destroy plans.  Start with powers of two.
  unsigned int vecsize;
  complex<float> rsum(0.0,0.0);
  for(vecsize = 32; vecsize <= 50000; vecsize *= 2) {
    rsum += doWork(invec, outvec, vecsize);
  }
  for(vecsize = 48; vecsize <= 50000; vecsize *= 2) {
    rsum += doWork(invec, outvec, vecsize);
  }
  unsigned int ivs[] = { 625, 1250, 2500, 3125, 5000, 15625, 78125, 390625, 0 };
  for(i = 0; ivs[i] > 0; i++) {
    rsum += doWork(invec, outvec, ivs[i]);
  }
  rsum += doWork(invec, outvec, 30000 + 1250);
  rsum += doWork(invec, outvec, 30000);
  rsum += doWork(invec, outvec, 32768);
  rsum += doWork(invec, outvec, 48 * 48);

  // now for buffer copy cost...
  double startt = curtime();
  for(i = 0; i < 100; i++) {
    memcpy(invec, outvec, 30000 * sizeof(complex<float>));
    rsum += invec[i];
    invec[i*2] *= rsum; 
    memcpy(outvec, invec, 30000 * sizeof(complex<float>));    
    rsum += outvec[i+3];
    outvec[i*2+3] *= rsum; 
  }
  double endt = curtime();
  double timet = endt - startt;
  
  printf("memcpy bandwidth %g sec for a 30K long vector, %g sec per sample.\n",
	 timet / 200.0, timet / (200.0 * 30000.0));
	 
  printf("dummy print = %f %f\n", rsum.real(), rsum.imag()); 
}
