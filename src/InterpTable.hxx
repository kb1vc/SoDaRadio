#ifndef __INTERPTABLE__
#define __INTERPTABLE__
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


#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <iostream>
#include <vector>
#include <algorithm>

#include <math.h>

/**
 * @brief InterpTable Interpolation Lookup Table. 
 *
 * This class stores a map of <T> keys onto <T> values. 
 * It is, however, accessed by supplying a value kv that may 
 * not be in the set of keys {K}.  Lookup of a key value kv
 * finds the nearest set of key values {Km} to kv.  The 
 * returned value is an interpolation from a degree 2, 3, or 4
 * polynomial fit to the available neighbor points. 
 *
 * @tparam T must be a type that has +, -, *, /, and < defined on it.
 */
template <typename T> 
class InterpTable {
public:
  /**
   * Constructor
   */
  InterpTable() {
    needs_sort = false; 
    num_elts = 0; 
  }

  /**
   * Copy Constructor
   */
  InterpTable(const InterpTable & src) {
    needs_sort = src.needs_sort;
    num_elts = src.num_elts;
    table = src.table;
  }

  

  /** 
   * @brief lookup a value in the table. 
   *
   * @param k a value that may not be in the set of stored keys
   *
   * @return an interpolated value based on the keys nearest to k
   */
  T lookup(const T & k) {
    sortTable();

    int idx = recursiveLookup(k, 0, num_elts-1); 
    
    return interpolate(k, idx); 
  }

  /**
   * @brief store a value in the table. 
   * 
   * @param k the key (x coordinate, for instance)
   * @param v the value (y coordinate, to belabor the point...)
   */
  void store(const T & k, const T & v) {
    table.push_back(std::pair<T, T>(k, v)); 
    needs_sort = true; 
    num_elts++; 
  }

  void clearTable() {
    table.erase(); 
  }

  
private:
  /**
   * The table is sorted when necessary. 
   */
  void sortTable() {
    if(!needs_sort) return; 
    std::sort(table.begin(), table.end(), tl_comp); 
  }

  class TLess {
  public:
    bool operator() (const std::pair<T, T> & a, const std::pair<T, T> & b) {
      return a.first < b.first; 
    }
  } tl_comp;

  unsigned int recursiveLookup(const T & k, unsigned int f, unsigned int l) 
  {
    unsigned int idx = (f + l) >> 1; 
    if((idx == f) || (idx == l) || (table[idx].first == k)) {
      // converged. 
      return idx; 
    }
    else if(table[idx].first < k)  {
      return recursiveLookup(k, idx, l);
    }
    else {
      return recursiveLookup(k, f, idx);
    }
  }

  /**
   * @brief Interpolate the dependent variable based on the value k
   * and the points found near by. 
   *
   * @param k the key value
   * @param idx the index to the largest value that is less than k;
   *
   * @return an interpolated value based on a lagrange polynomial fit to the neighborhood
   */
  T interpolate(const T & k, unsigned int idx) {
    T x0, x1, x2, x3;     
    T y0, y1, y2, y3; 
    int degree; 
    int sidx; 

    if(num_elts < 3) {
      // we only have two points... interpolate between them. 
      // YIPEEE!!!
      degree = 2; 
      sidx = 0; 
    }
    else if(idx == (num_elts - 1)) {    
      // this key is beyond the end of the list... 
      // interpolate using the two points
      degree = 2; 
      sidx = idx - 1;
    }
    else if(idx == (num_elts - 2)) {
      // this key is between the last two points on the list.
      // use the last three points
      degree = 3; 
      sidx = idx - 1; 
    }
    else if((idx == 0) && (k > table[0].first)) {
      // this key is between the first two points on the list
      // use the first three points
      degree = 3; 
      sidx = 0; 
    }
    else if(idx == 0) {
      // this key is below the list. 
      degree = 2; 
      sidx = 0; 
    }
    else {
      // this is the normal case...
      x0 = table[idx-1].first;
      y0 = table[idx-1].second;      
      x1 = table[idx].first;
      y1 = table[idx].second;      
      x2 = table[idx+1].first;
      y2 = table[idx+1].second;      
      x3 = table[idx+2].first;
      y3 = table[idx+2].second;      

      degree = 4; 
      sidx = idx - 1; 
    }

    x0 = table[sidx].first;
    y0 = table[sidx].second;

    if(degree == 4) {
      x1 = table[sidx + 1 ].first;
      y1 = table[sidx + 1].second;
      x2 = table[sidx + 2].first;
      y2 = table[sidx + 2].second;
      x3 = table[sidx + 3].first;
      y3 = table[sidx + 3].second;
      return 
	y0 * ((k - x1) * (k - x2) * (k - x3) / 
	      ((x0 - x1) * (x0 - x2) * (x0 - x3))) +
	y1 * ((k - x0) * (k - x2) * (k - x3) / 
	      ((x1 - x0) * (x1 - x2) * (x1 - x3))) + 	
	y2 * ((k - x0) * (k - x1) * (k - x3) / 
	      ((x2 - x0) * (x2 - x1) * (x2 - x3))) +
      	y3 * ((k - x0) * (k - x1) * (k - x2) / 
	      ((x3 - x0) * (x3 - x1) * (x3 - x2)));      
    }
    else if(degree == 3) {    
      x1 = table[sidx + 1 ].first;
      y1 = table[sidx + 1].second;
      x2 = table[sidx + 2].first;
      y2 = table[sidx + 2].second;
      return y0 * (k - x1) * (k - x2) / ((x0 - x1) * (x0 - x2)) +
	y1 * (k - x0) * (k - x2) / ((x1 - x0) * (x1 - x2)) + 	
	y2 * (k - x0) * (k - x1) / ((x2 - x0) * (x2 - x1));
    }
    else if(degree == 2) {
      x1 = table[sidx + 1 ].first;
      y1 = table[sidx + 1].second;
      return y0 * (k - x1) / (x0 - x1) +
	y1 * (k - x0) / (x1 - x0);
    }
    else {
      throw std::runtime_error("interpolate function is horribly confused.");
    }
  }
  
  std::vector<std::pair< T, T> > table; 
  bool needs_sort; 
  unsigned int num_elts; 
}; 

#endif // __INTERPTABLE__

