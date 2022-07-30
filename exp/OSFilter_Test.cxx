/*
Copyright (c) 2022, Matthew H. Reilly (kb1vc)
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
#include "OSFilter.hxx"
#include "Filter.hxx"
#include <time.h>
#include <fftw3.h>
#include <math.h>

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

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


int main(int argc, char * argv[])
{

  SoDa::FilterSpec fs().add().add().add().add();
  SoDa::OSFilter osf(std::unique_ptr<BlocOp>(new SoDa::Filter(fs)), 
		     taps - 1, 0);

  asdf
  srandom(0x13255);
  
  const int samp_len = 65536;
  int inbuflen = samp_len / 8; 
  float * specbuf[6];
  for(int iii = 0; iii < 6; iii++) {
    specbuf[iii] = (float*) malloc(sizeof(float) * samp_len); 
  }
  filter_map =  buildFilterMap(inbuflen);
  // three
  complex<float> in[samp_len];
  complex<float> out[6][samp_len];

  FILE * trace_outf = fopen("OSFilter_Test_Trace.dat", "w");
  FILE * spec_outf = fopen("OSFilter_Test_Spectrum.dat", "w");

  // four

  std::complex<float> sout[6][samp_len];
  fftwf_plan sp = fftwf_plan_dft_1d(samp_len, (fftwf_complex *) out[0], (fftwf_complex *) sout[0],
				    FFTW_FORWARD, FFTW_ESTIMATE);

  // five
  for(j = 0; j < 5; j++) {
    for(i = 0; i < samp_len; i++) specbuf[j][i] = 0.0;
  }
  // one

  int k; 
  for(k = 0; k < 1000; k++) {
    for(i = 0; i < samp_len; i++) {
      long irand = random();
      in[i] = ((float) (irand & 0xffff)); 
    }

    // not this

    for(i = 0; i < samp_len; i++) {
      out[0][i] = in[i]; 
    }
    // not this

    // now run the filter through its paces.
    for(i = 0; i < 5; i++) {
      int j;
      for(j = 0; j < samp_len; j += inbuflen) {
	filter_map[i]->apply(&(in[j]), &(out[i+1][j]));
      }
    }
    // not in here

    // print the results.
    if(k == 2) {
      for(i = 0; i < samp_len; i++) {
	fprintf(trace_outf, "%d ", i); 
	for(j = 0; j < 6; j++) {
	  fprintf(trace_outf, "%g %g ", out[j][i].real(), out[j][i].imag());
	}
	fprintf(trace_outf, "\n");
      }
    }

    for(j = 0; j < 6; j++) {
      fftwf_execute_dft(sp, (fftwf_complex*) out[j], (fftwf_complex*)sout[j]);
    }
    for(i = 0; i < samp_len; i++) {
      for(j = 0; j < 6; j++) {
	float re, im;
	re = sout[j][i].real();
	im = sout[j][i].imag();
	specbuf[j][i] += sqrt(re * re + im * im); 
      }
    }
  }

  fclose(trace_outf);
  float norm = 1.0 / ((float) k); 
  for(i = 0; i < samp_len / 2; i++) {
    float freq = ((float) i) * 48000.0 / 65536.0; 
    fprintf(spec_outf, "%d %f", i,  freq);
    for(j = 0; j < 6; j++) {
      fprintf(spec_outf, " %f %f", norm * specbuf[j][i], arg(sout[j][i])); 
    }
    fprintf(spec_outf, "\n"); 
  }
  fclose(spec_outf); 
  return 0; 
}
