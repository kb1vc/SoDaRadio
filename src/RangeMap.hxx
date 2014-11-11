#ifndef RANGE_MAP_HDR
#define RANGE_MAP_HDR
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

///
///  @file RangeMap.hxx
///  @brief Specialization of the STL map container to support
///  range-based lookups. 
///
///
///  @author M. H. Reilly (kb1vc)
///  @date   October, 2014
///
#include <map>

namespace SoDa {
  ///  @class Range -- a template class for mapping ranges to thingies
  ///  This represents an open interval -- thanks to suggestions from
  ///  http://stackoverflow.com/questions/1089192/c-stl-range-container
  template <typename Tk> class Range {
  public:
    Range(Tk _min, Tk _max) {
      min = _min;
      max = _max; 
    }

    Range(Tk _c) {
      min = _c;
      max = _c;
    }

    Tk getMin() const { return min; }
    Tk getMax() const { return max; }


      
  private:
    Tk min, max; 
  };

  template <typename Tk> struct leftOfRange : public std::binary_function< Range<Tk>, Range<Tk>, bool>
  {
    bool operator() (Range<Tk> const & l,
		     Range<Tk> const & r) const {
      return ((l.getMin() < r.getMin()) && (l.getMax() <= r.getMin()));
    }
  };

  template <typename Tk, typename Tv> class RangeMap : public std::map<Range<Tk>, Tv, leftOfRange<Tk> > {
  public:
    RangeMap() : std::map<Range<Tk>, Tv, leftOfRange<Tk> >() {
    }
    
  protected:
  }; 
}


#endif
