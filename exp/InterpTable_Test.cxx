/*
Copyright (c) 2015, Matthew H. Reilly (kb1vc)
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

#include "InterpTable.hxx"

#include <math.h>
#define MAX(a, b) (((a)>(b)) ? (a) : (b))
#define MIN(a, b) (((a)<(b)) ? (a) : (b))

int main() {

  InterpTable<double> osin_map; 
  InterpTable<double> sin_map;

  double ang = -M_PI; 
  for(ang = -3.2; ang < 3.2; ang += 0.1) {
    osin_map.store(ang, sin(ang));
    sin_map.store(ang+0.01, cos(ang+0.01));     
  }
  
  std::cerr << "About to do lookup test." << std::endl; 

  sin_map = osin_map; 

  double errsum, err; 
  int count = 0; 
  errsum = 0.0; 
  for(ang = -3.2; ang < 3.2; ang += 0.01) {
    err = sin(ang) - sin_map.lookup(ang); 
    errsum += err * err; 
    count++; 
  }

  
  std::cerr << boost::format("RMS error %g over %d samples\n")
    % (errsum / ((double) count)) % count; 

}

