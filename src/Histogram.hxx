/*
Copyright (c) 2017 Matthew H. Reilly (kb1vc)
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

#ifndef HISTOGRAM_HDR
#define HISTOGRAM_HDR
#include <iostream>
#include <fstream>

namespace SoDa {
  /**
   * A generic histogram widget. 
   *
   */
  class Histogram {
  public:
    /**
     * @brief Constructor for the histogram. 
     *
     * @param _num_buckets the number of buckets in the histogram
     * @param _min  the value corresponding to the bottom of
     *                    bucket 0 in the histogram
     * @param _max  the value corresponding to the top of
     *                    bucket (num_buckets - 1) in the histogram
     *
     */
    Histogram(unsigned int _num_buckets, 
	      double _min, double _max) {
      num_buckets = _num_buckets; 
      min = _min; 
      max = _max; 
      table = new unsigned int [num_buckets]; 
      recip_bucket_interval = ((double) num_buckets) / (max - min); 
      for(unsigned int i = 0; i < num_buckets; i++) table[i] = 0; 
    }

    void updateTable(double v) {
      int idx = lround((v  - min) * recip_bucket_interval); 
      if(v > max) table[num_buckets - 1]++; 
      else if(v < min) table[0]++; 
      else table[idx]++; 
    }

    void writeTable(const std::string & fname) {
      std::ofstream of(fname.c_str()); 

      double bucket_interval = 1.0 / recip_bucket_interval; 
      double half_int = 0.5 * bucket_interval; 

      for(unsigned int i = 0; i < num_buckets; i++) {
	double mid = min + half_int + ((double) i) * bucket_interval; 
	if(table[i] != 0) of << SoDa::Format("%0 %1 %2\n")
			    .addI(i)
			    .addF(mid)
			    .addI(table[i]); 
      }

      of.close();
    }

  protected:
    unsigned int * table; 
    double recip_bucket_interval; 
    unsigned int num_buckets; 
    double min, max; 
  }; 

}


#endif
