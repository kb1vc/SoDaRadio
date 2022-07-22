/*
Copyright (c) 2022 Matthew H. Reilly (kb1vc)
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
#include <vector>
#include <iostream>
#include <fftw3.h>
#include <SoDa/Format.hxx>
#include <SoDa/Options.hxx>
#include <random>
#include <chrono>
#include <string>
#include <cstring>


class FFT {
public:
  FFT(int size, unsigned int flags) : size(size) {
    fft_in = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * size);
    fft_out = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * size);

    f_plan = fftwf_plan_dft_1d(size, fft_in, fft_out, FFTW_FORWARD, flags);
    i_plan = fftwf_plan_dft_1d(size, fft_in, fft_out, FFTW_BACKWARD, flags);    
  }

  ~FFT() {
    fftwf_destroy_plan(f_plan);
    fftwf_destroy_plan(i_plan);
    fftwf_free(fft_in);
    fftwf_free(fft_out);
  }

  void forward(const std::vector<std::complex<float>> & in, 
	       std::vector<std::complex<float>> & out) {
    doit(f_plan, in, out);
  }

  void inverse(const std::vector<std::complex<float>> & in, 
	       std::vector<std::complex<float>> & out) {
    doit(i_plan, in, out);
  }
  
private:
  
  void doit(fftwf_plan plan, const std::vector<std::complex<float>> & in, std::vector<std::complex<float>> & out) {
    std::memcpy((void*)fft_in, (void*)in.data(), size * sizeof(std::complex<float>));
    fftwf_execute(plan);
    std::memcpy((void*)out.data(), (void*) fft_out, size * sizeof(std::complex<float>));
  }
  
  fftwf_complex * fft_in, * fft_out; 
  fftwf_plan f_plan, i_plan;
  int size; 
}; 

std::random_device dev;
std::mt19937 rng(dev());
std::uniform_real_distribution<float> distr(-1.0, 1.0);

void doTest(int size, unsigned int flags) {
  // build the test input vector.
  std::vector<std::complex<float>> test_in(size);
  std::vector<std::complex<float>> test_out(size);
  // write to the sink to avoid disappearing values/convergence problems.
  std::vector<std::complex<float>> test_sink(size);

  for(int i = 0; i < size; i++) {
    test_in[i] = std::complex<float>(distr(rng), distr(rng));
  }
  
  // how long does it take to create the FFT widget
  auto cr_st = std::chrono::steady_clock::now();
  auto fft = new FFT(size, flags);
  auto cr_end = std::chrono::steady_clock::now();
  
  double create_time = 1e-9 * (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(cr_end - cr_st).count());
  
  // First, find a good number of iterations to get us to about
  // one second. The way we'll do that is to measure 8 iterations and
  // get a good guess
  auto st_t1 = std::chrono::steady_clock::now();
  for(int i = 0; i < 8; i++) {
    fft->forward(test_in, test_out);
    fft->inverse(test_out, test_sink);
  }
  auto en_t1 = std::chrono::steady_clock::now();

  auto itdur = std::chrono::duration_cast<std::chrono::nanoseconds>(en_t1 - st_t1).count();
  // 8 iterations in tdur ns.
  // how many in 10^9 ns?
  double tdur = ((double) itdur);
  double dit = (8e9 / tdur);
  unsigned int iters = (unsigned int) floor(dit);

  std::cerr << "Doing " << iters << " at size " << size << "\n";
  
  // get the start time
  auto start = std::chrono::steady_clock::now();
  
  for(int i = 0; i < iters; i++) {
    fft->forward(test_in, test_out);
    fft->inverse(test_out, test_sink);
  }

  // get the end time
  auto end = std::chrono::steady_clock::now();

  auto idur = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();

  double dur = (double) idur; 
  
  // duration in ns dur

  // ns per point
  double ns_p_p = dur / ((double) (2 * iters * size));

  std::cout << SoDa::Format("%0 %1 %2 %3\n")
    .addI(size)
    .addF(dur * 1e-9, 'e', 4, 4)
    .addF(ns_p_p * 1e-9, 'e', 4, 4)
    .addF(create_time, 'e', 4, 4);
				       
  std::cout.flush();
}


int main(int argc, char * argv[])
{
  unsigned int fftw_flag = FFTW_ESTIMATE;
  
  if(argc >= 2) {
    switch(argv[1][0]) {
    case 'E':
      fftw_flag = FFTW_ESTIMATE;
      std::cout << "# testing FFTW_ESTIMATE\n";
      break; 
    case 'M':
      fftw_flag = FFTW_MEASURE;
      std::cout << "# testing FFTW_MEASURE\n";
      break; 
    case 'P':
      fftw_flag = FFTW_PATIENT;
      std::cout << "# testing FFTW_PATIENT\n";
      break; 
    case 'X':
      fftw_flag = FFTW_EXHAUSTIVE;
      std::cout << "# testing FFTW_EXHAUSTIVE\n";
      break; 
    }
  }
  // do powers of 2 from 8 to 18
  // and powers of 3 from 0 to 3
  // and powers of 5 from 0 to 3

  for(int p2 = 4; p2 <= (1 << 18); p2 = p2 * 2) {
    for(int p3 = 1 ; p3 <= 27; p3 = p3 * 3) {
      for(int p5 = 1; p5 <= 625; p5 = p5 * 5) {
	int s = p2 * p3 * p5;
	if(s > (1 << 18)) continue; 
	if(s < 1000) continue;
	std::cout << "2:" << p2 << " 3:" << p3 << " 5:" << p5 << " ";
	doTest(s, fftw_flag);
      }
    }
  }
}

