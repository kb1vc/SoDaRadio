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
#include <fstream>
#include <stdlib.h>
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

 // test program to checkout the antialiasing capability of the
 // resamplers. 

using namespace std; 

double curtime()
{
  double ret;
  struct timeval tp; 
  gettimeofday(&tp, NULL); 
  ret = ((double) tp.tv_sec) + 1e-6*((double)tp.tv_usec);
  return ret; 
}

static void bindump(char * fn, std::complex<float> * buf, unsigned int num_elts)
{
  FILE * of;
  of = fopen(fn, "w");
  int i;
  for(i = 0; i < num_elts; i++) {
    fprintf(of, "%d %g %g\n", i, buf[i].real(), buf[i].imag()); 
  }
  fclose(of); 
}

void checkResult(FILE * ofile,
		 float anginc, unsigned int iM, unsigned int dN,
		 unsigned int inlen,
		 std::complex<float> * invec, std::complex<float> * outvec, int idx = 0)
{
  // sweep through the output.

  // how long is the output?
  unsigned int outlen = inlen * iM;
  outlen = outlen / dN;

  // what is the scale factor for the angle increment?
  float oanginc = anginc * ((float) dN) / ((float) iM);

  int i;
  float ang = 0.0;
  float err_sq_sum[2] = { 0.0, 0.0 }; 
  for(i = 0; i < outlen; i++) {
    // test the output.
    fprintf(ofile, "%d\t%g\t%g\t%g\t%g\t%g\t", i + idx*outlen, ang, outvec[i].real(), outvec[i].imag(), cos(ang), sin(ang)); 
    float err = outvec[i].real() - cos(ang); 
    err_sq_sum[0] += err * err;
    fprintf(ofile, "%g\t", err); 
    err = outvec[i].imag() - sin(ang);
    err_sq_sum[1] += err * err;  
    fprintf(ofile, "%g\n", err); 
    ang += oanginc;
    if(ang > M_PI) ang = ang - (2.0 * M_PI); 
  }

  fprintf(ofile, "%d\t%g\t%g\t%g\t%g\t%g\t%g\t%g\n", i + idx*outlen, ang+0.5*anginc, 5.0, 5.0, cos(ang), sin(ang), 1.0, 1.0); 
  float fn = (float) outlen; 
  printf("RMS error for M/N = %d / %d : %g  %g\n",
	 iM, dN, sqrt(err_sq_sum[0]/fn), sqrt(err_sq_sum[1]/fn)); 
}

float loadVector(float ang, float ang_inc, std::complex<float> * vec, int len)
{
  int i;
  for(i = 0; i < len; i++) {
    vec[i] = std::complex<float>(cos(ang), sin(ang));
    ang += ang_inc;
    int ii = i / 100; 
    if((ii == 0) || (ii == 5) || (ii == 7) || (ii == 10)) ang += ang_inc; 
    if(ang > M_PI) ang -= (2.0 * M_PI); 
  }
  return ang; 
}

void test625x48()
{
  SoDa::ReSample625to48 rsf(30000);
  SoDa::ReSample625to48 rsc(30000);
  float in[60000], out[2304];
  std::complex<float> cin[60000], cout[2304];
  int i;
  float ang = 0.0;

  for(i = 0; i < 60000; i++) {
    in[i] = sin(ang);
    cin[i] = std::complex<float>(sin(ang), cos(ang)); 
    ang += 0.01;
  }

  // now resample
  rsf.apply(in, out);
  rsf.apply(in + 30000, out);
  rsc.apply(cin, cout); 
  rsc.apply(cin + 30000, cout); 

  for(i = 0; i < 30000; i++) {
    int j = i + 30000; 
    std::cout << boost::format("%d %f %f %f ") % i % in[j] % cin[j].real() % cin[j].imag();
    if(i < 2304) {
      std::cout << boost::format("%f %f %f\n") % out[i] % cout[i].real() % cout[i].imag(); 
    }
    else {
      std::cout << std::endl; 
    }
  }
}

void test48x625()
{
}

void aliasingPlot()
{
  // create a downsampler from 30K samples to 2304
  SoDa::ReSample625to48 rsf(30000);
  std::complex<float> cin[30000], cout[2304];
  
  ofstream to("ReSampler625to48_Test.dat");

  // pretend that our input sample rate was 625KS/sec.
  float samp_freq = 625000.0;
  float test_freq;
  float angle = 0.0; 
  for(test_freq = -312500.0; test_freq < 312500.0; ) {
    int i;
    float phase_advance_per_tic = 2.0 * M_PI * test_freq / samp_freq;
    float energy = 0.0; 
    for(i = 0; i < 100; i++) {
      int j;
      for(j = 0; j < 30000; j++) {
	cin[j].real() = cos(angle);
	cin[j].imag() = sin(angle);
	angle += phase_advance_per_tic;
	if(angle > M_PI) angle -= (2.0 * M_PI);
	if(angle < -M_PI) angle += (2.0 * M_PI);
      }
      rsf.apply(cin, cout);
      if(i != 0) {
	for(j = 0; j < 2304; j++) {
	  float ip = cout[j].real();
	  float qu = cout[j].imag(); 
	  energy += ip * ip + qu * qu;
	}
      }
    }
    to << boost::format("%f %f\n") % test_freq % (10.0 * log10(energy));
    std::cout << boost::format("%f %f\n") % test_freq % (10.0 * log10(energy));
    if(test_freq < 0.0) {
      if(test_freq > -10.0) test_freq = 10.0;
      else test_freq *= (1.0 / 1.05); 
    }
    else {
      test_freq *= 1.05; 
    }
  }
  
  to.close();
}

int main(int argc, char * argv[])
{
  // create various ReSampler ratios, especially the ones I care
  // about. 
  unsigned int resampler_ratio[][2] = { { 5, 1 },
					{ 5, 4 },
					{ 5, 3 },
					{ 5, 2 },
					{ 0, 0} };
  // create the input and output buffers.
  const unsigned int min_mult_size = 5 * 4 * 3;
  // max size of outvec/invec = 5:1.  
  std::complex<float> invec[30000]; 
  std::complex<float> outvec[30000]; 
  std::complex<float> out2vec[30000]; 

  aliasingPlot(); 
  
  exit(0);

  test625x48();
  test48x625();

  int i, j;
  // fill the input vector
  float ang = 0.0;
  float ang_inc = 2.0 * M_PI * 315.26 / 6000.0;
  ang = loadVector(ang, ang_inc, invec, 6000); 

  
  SoDa::ReSampler * rs;
  for(j = 0; j < 2; j++) {
    for(i = 0; resampler_ratio[i][0] > 0; i++) {
      unsigned int iM, dN;
      iM = resampler_ratio[i][j]; 
      dN = resampler_ratio[i][1-j];
      
      char buf[1024]; 
      sprintf(buf, "resample_%d_%d.dat", iM, dN);
      FILE * ofile = fopen(buf, "w"); 
      
      // create a ReSampler.
      rs = new SoDa::ReSampler(iM, dN, 6000, 255);
      ang = loadVector(ang, ang_inc, invec, 6000); 
      rs->apply(invec, outvec);
      ang = loadVector(ang, ang_inc, invec, 6000); 
      double startt = curtime(); 
      rs->apply(invec, outvec);
      double endt = curtime(); 
      // now check the vector
      checkResult(ofile, ang_inc, iM, dN, 6000, invec, outvec, 0);
      ang = loadVector(ang, ang_inc, invec, 6000); 
      rs->apply(invec, outvec);
      checkResult(ofile, ang_inc, iM, dN, 6000, invec, outvec, 1);
      ang = loadVector(ang, ang_inc, invec, 6000); 
      rs->apply(invec, outvec);
      checkResult(ofile, ang_inc, iM, dN, 6000, invec, outvec, 2);
      ang = loadVector(ang, ang_inc, invec, 6000); 
      rs->apply(invec, outvec);
      checkResult(ofile, ang_inc, iM, dN, 6000, invec, outvec, 3);
      std::cout << "TIM: Resample took " << (endt - startt) << " seconds" << std::endl; 
      delete rs;
      fclose(ofile); 
    }
  }

  // now a time test.
  // create a 5/4 resampler and a 4/5 resampler.
  SoDa::ReSampler * up, * down;
  up = new SoDa::ReSampler(5, 4, 6000, 255);
  down = new SoDa::ReSampler(4, 5, 7500, 255);
  double ttstart = curtime();
  for(i = 0; i < 4000; i++) {
    up->apply(invec, outvec);
    down->apply(outvec, invec); 
  }
  double ttend = curtime(); 

  std::cout << "TIM: average time per up/down resampling pair: "
	    << (ttend - ttstart) / ((double) i) << std::endl; 

  // Now try a 2304 buffer upsampled by 625/48
  SoDa::ReSampler rs54a(5, 4, 2304, 255); 
  SoDa::ReSampler rs54b(5, 4, 2880, 255); 
  SoDa::ReSampler rs53(5, 3, 3600, 255); 
  SoDa::ReSampler rs51(5, 1, 6000, 255);

  ang = 0.0;
  ang_inc = 2.0 * M_PI / 53.7;
  float ang2 = 0.0;
  float ang2_inc = 2.0 * M_PI * 3.0 / 4.0; 
  for(i = 0; i < 30000; i++) {
    //    invec[i] = std::complex<float>(0.5 * cos(ang) + cos(ang2), 0.5 * sin(ang) + sin(ang2));
    invec[i] = std::complex<float>(cos(ang), sin(ang));
    ang += ang_inc;
    if(ang > M_PI) ang -= 2.0 * M_PI; 
    ang2 += ang2_inc;
    if(ang2 > M_PI) ang2 -= 2.0 * M_PI; 
  }

  int uf = creat("up_1khz_625k.dat", 0666);
  
  ttstart = curtime();
  rs54a.apply(invec, outvec);
  rs54b.apply(outvec, out2vec); 
  rs53.apply(out2vec, outvec); 
  rs51.apply(outvec, out2vec);
  ttend = curtime(); 
  std::cout << "Upsampled from 2304 to 30000 in 5/4 5/4 5/3 5/1 in "
	    << (ttend - ttstart) << " seconds" << std::endl; 
  int stat = write(uf, out2vec, sizeof(std::complex<float>) * 30000); 
  rs54a.apply(invec, outvec);
  rs54b.apply(outvec, out2vec); 
  rs53.apply(out2vec, outvec); 
  rs51.apply(outvec, out2vec);
  stat = write(uf, out2vec, sizeof(std::complex<float>) * 30000); 
  rs54a.apply(invec, outvec);
  rs54b.apply(outvec, out2vec); 
  rs53.apply(out2vec, outvec); 
  rs51.apply(outvec, out2vec);
  stat = write(uf, out2vec, sizeof(std::complex<float>) * 30000); 
  rs54a.apply(invec, outvec);
  rs54b.apply(outvec, out2vec); 
  rs53.apply(out2vec, outvec); 
  rs51.apply(outvec, out2vec);
  stat = write(uf, out2vec, sizeof(std::complex<float>) * 30000); 

  close(uf); 
  
  SoDa::ReSampler rs53b(5, 3, 2880, 255); 
  SoDa::ReSampler rs52a(5, 2, 4800, 255);
  SoDa::ReSampler rs52b(5, 2, 12000, 255);
  ttstart = curtime();
  rs54a.apply(invec, outvec);
  rs53b.apply(invec, outvec); 
  rs52a.apply(outvec, invec); 
  rs52b.apply(outvec, invec); 
  ttend = curtime(); 
  std::cout << "Upsampled from 2304 to 30000 in 5/4 5/3 5/2 5/2 in "
	    << (ttend - ttstart) << " seconds" << std::endl; 



  // Now try a 2304 buffer downsampled by 48/625
  SoDa::ReSampler rs15(1, 5, 30000, 255);
  SoDa::ReSampler rs35(3, 5, 6000, 255); 
  SoDa::ReSampler rs45a(4, 5, 4800, 255); 
  SoDa::ReSampler rs45b(4, 5, 2880, 255); 


  ang = 0.0;
  ang_inc = 2.0 * M_PI / 50.0; 
  for(i = 0; i < 30000; i++) {
    invec[i] = std::complex<float>(cos(ang), sin(ang));
    ang += ang_inc;
    if(ang > M_PI) ang -= 2.0 * M_PI; 
  }
  ttstart = curtime();
  rs15.apply(invec, outvec);
  rs35.apply(outvec, out2vec); 
  rs45a.apply(out2vec, outvec);
  rs45b.apply(outvec, out2vec); 
  ttend = curtime(); 
  std::cout << "Downsampled from 30000 to 2304 in 1/5 3/5 4/5 4/5 in "
	    << (ttend - ttstart) << " seconds" << std::endl; 

  int df = creat("down_1khz_625k.dat", 0666);
  stat = write(df, out2vec, sizeof(std::complex<float>) * 2304); 
  rs15.apply(invec, outvec);
  rs35.apply(outvec, out2vec); 
  rs45a.apply(out2vec, outvec);
  rs45b.apply(outvec, out2vec); 
  stat = write(df, out2vec, sizeof(std::complex<float>) * 2304);
  
  rs15.apply(invec, outvec);
  rs35.apply(outvec, out2vec); 
  rs45a.apply(out2vec, outvec);
  rs45b.apply(outvec, out2vec); 
  stat = write(df, out2vec, sizeof(std::complex<float>) * 2304);

  close(df); 

  SoDa::ReSampler rs25a(2, 5, 30000, 255);
  SoDa::ReSampler rs25b(2, 5, 12000, 255);
  SoDa::ReSampler rs35b(3, 5, 4800, 255); 
  ttstart = curtime();
  rs25b.apply(outvec, invec);
  rs25a.apply(outvec, invec); 
  rs35b.apply(invec, outvec);
  rs45b.apply(invec, outvec);

  
  ttend = curtime(); 
  std::cout << "Downsampled from 30000 to 2304 in 2/5 2/5 3/5 1/5 in "
	    << (ttend - ttstart) << " seconds" << std::endl; 

    
}
