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
#include "ReSampler.hxx"
#include "ReSamplers625x48.hxx"
#include "SoDaBase.hxx"
#include <iostream>
#include <stdlib.h>
#include <string.h>


// Implement an overlap-and-save interpolator with zero stuffing. 
SoDa::ReSample48to625::ReSample48to625(unsigned int inbufsize, float _gain) : SoDaBase("ReSamp 48 to 625") 
{
  // create the input and output buffers.
  // set N and MN;

  // N is the length of the input buffer.
  N = inbufsize;

  // N must be a multiple of 48, otherwise this doesn't work.
  if ((N % 48) != 0) {
    throw(new SoDa::SoDaException("48 to 625 resampler got an input buffer size that is not a multiple of 48", this)); 
  }

  // MN is the size of the output buffer. 
  MN = N * 625 / 48;

  // allocate the intermediate buffers
  inter1 = new std::complex<float>[MN];
  inter2 = new std::complex<float>[MN];
  finter1 = new float[MN];
  finter2 = new float[MN];

  // create the resamplers.
  int s1, s2, s3, s4;
  s1 = N;
  s2 = (s1 * 5) / 4;
  s3 = (s2 * 5) / 4;
  s4 = (s3 * 5) / 3;
  rs54a = new SoDa::ReSampler(5, 4, s1, 255); 
  rs54b = new SoDa::ReSampler(5, 4, s2, 255); 
  rs53 = new SoDa::ReSampler(5, 3, s3, 255); 
  rs51 = new SoDa::ReSampler(5, 1, s4, 255);

  gain = _gain; 
}

SoDa::ReSample48to625::~ReSample48to625()
{
  delete rs54a;
  delete rs54b;
  delete rs53;
  delete rs51;
}

// inlen and outlen are in "samples" -- that is pairs of doubles... 
void SoDa::ReSample48to625::apply(std::complex<float> * in, std::complex<float> * out)
{
  rs54a->apply(in, inter1);
  rs54b->apply(inter1, inter2); 
  rs53->apply(inter2, inter1); 
  rs51->apply(inter1 ,out, gain);
}

void SoDa::ReSample48to625::apply(float * in, float * out)
{
  rs54a->apply(in, finter1);
  rs54b->apply(finter1, finter2); 
  rs53->apply(finter2, finter1); 
  rs51->apply(finter1, out, gain);
}




SoDa::ReSample625to48::ReSample625to48(unsigned int inbufsize, float _gain) : SoDaBase("ReSamp 48 to 625") 
{
  // create the input and output buffers.
  // set N and MN;

  // N is the length of the input buffer.
  N = inbufsize;

  // N must be a multiple of 625, otherwise this doesn't work.
  if ((N % 625) != 0) {
    throw(new SoDa::SoDaException("625 to 48 resampler got an input buffer size that is not a multiple of 48", this)); 
  }

  // MN is the size of the output buffer. 
  MN = N * 48 / 625;

  // allocate the intermediate buffers
  inter1 = new std::complex<float>[N];
  inter2 = new std::complex<float>[N];
  finter1 = new float[N];
  finter2 = new float[N];

  // create the resamplers.
  int s1, s2, s3, s4;
  s1 = N;
  s2 = (s1 * 1) / 5;
  s3 = (s2 * 3) / 5;
  s4 = (s3 * 4) / 5;
  rs15 = new SoDa::ReSampler(1, 5, s1, 255);
  rs35 = new SoDa::ReSampler(3, 5, s2, 255); 
  rs45a = new SoDa::ReSampler(4, 5, s3, 255); 
  rs45b = new SoDa::ReSampler(4, 5, s4, 255); 

  gain = _gain; 
}

SoDa::ReSample625to48::~ReSample625to48()
{
  delete rs45a;
  delete rs45b;
  delete rs35;
  delete rs15;
}

// inlen and outlen are in "samples" -- that is pairs of doubles... 
void SoDa::ReSample625to48::apply(std::complex<float> * in, std::complex<float> * out)
{
  rs15->apply(in, inter1);
  rs35->apply(inter1, inter2); 
  rs45a->apply(inter2, inter1); 
  rs45b->apply(inter1 ,out, gain);
}

void SoDa::ReSample625to48::apply(float * in, float * out)
{
  rs15->apply(in, finter1);
  rs35->apply(finter1, finter2); 
  rs45a->apply(finter2, finter1); 
  rs45b->apply(finter1, out, gain);
}
