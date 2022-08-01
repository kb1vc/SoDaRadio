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

#include "FilterSpec.hxx"

namespace SoDa {
  FilterSpec::FilterSpec(float sample_rate, unsigned int taps, FType filter_type) :
    sorted(false), filter_type(filter_type), taps(taps), sample_rate(sample_rate)
  {
  }

  unsigned int FilterSpec::estimateTaps(unsigned int min, unsigned int max) {
    // find the narrowest transition region
    if(!sorted) sortSpec();

    // now crawl...
    bool looking = true;
    float min_interval = 10e9;
    float last_gain;
    float last_corner;
    last_gain = spec.front().gain;
    last_corner = spec.front().freq;
    for(const auto & v : spec) {
      if(v.gain != last_gain) {
	float inter = v.gain - last_gain;
	if(inter < min_interval) min_interval = inter;
	last_gain = v.gain;
	last_corner = v.freq;
      }
    }
    
    // now use fred harris's rule for the number of filter taps?
    // assume 40 dB stop band attenuation
    float ftaps = sample_rate * 40 / (min_interval * 22);
    float comp_taps = 2 * int(ftaps / 2) + 1; // make sure it is odd

    return comp_taps;
  }

  void FilterSpec::fillHproto(std::vector<std::complex<float>> & Hproto) {
    if(!sorted) sortSpec();
    
    std::list<std::pair<Corner,Corner>> edges;
    float start_freq = (filter_type == REAL) ? 0.0 : - (sample_rate / 2);
    float end_freq = sample_rate/2;
    Corner last(start_freq, -200.0);

    for(auto c : spec) {
      edges.push_back(std::pair<Corner,Corner>(last, c));
      last = c; 
    }
    edges.push_back(std::pair<Corner,Corner>(last, Corner(end_freq, -200.0)));

    // now go through the edges.
    for(auto e : edges) {
	
      auto start_idx = indexHproto(e.first.freq);
      auto end_idx = indexHproto(e.second.freq);
      // note the 0.05 multiplier -- we're dealing in filter amplitudes.... ;(
      auto start_amp = std::pow(10.0, 0.05 * e.first.gain);
      auto end_amp = std::pow(10.0, 0.05 * e.second.gain);
      if(start_idx == end_idx) {
	auto max_amp = (start_amp > end_amp) ? start_amp : end_amp; 
	Hproto[start_idx] = std::complex<float>(max_amp);
      }
      else {
	auto rise = end_amp / start_amp;
	auto run =  float(end_idx - start_idx);
	auto mult = pow(rise, 1.0 / run);
	float amp = start_amp; 
	for(int i = start_idx; i <= end_idx; i++) {
	  Hproto[i] = std::complex<float>(amp, 0.0);
	  amp *= mult;
	}
      }
    }
  }

  FilterSpec & FilterSpec::start(float amp) {
    sorted = false;
    float start_freq = (filter_type == REAL) ? 0.0 : - (sample_rate / 2);

    if(amp < -200) amp = -200;
    
    spec.push_back(Corner(start_freq, amp));
    
    return *this;
  }

  FilterSpec & FilterSpec::add(float freq, float amp) {
    sorted = false;
    if((filter_type == REAL) && (freq < 0)) {
      throw BadRealSpec("add", freq);
    }
    // no zero amplitude buckets. (We need to be able to divide and take logs.)
    if(amp < -200) amp = -200;
    
    spec.push_back(Corner(freq, amp));    
    
    return *this;
  }
      
  const std::list<FilterSpec::Corner> FilterSpec::getSpec() {
    if(!sorted) sortSpec();
    return spec; 
  }

  std::pair<float, float> FilterSpec::getFilterEdges() {
    std::pair<float, float> ret;
    ret.first = (filter_type == REAL) ? 0.0 : -(sample_rate / 2);
    ret.second = sample_rate / 2;
    bool looking_low;
    float last_freq; 
    for(const auto v : spec) {
      if(looking_low) {
	if(v.gain > (1 - 0.01)) {
	  ret.first = v.freq;
	  looking_low = false; 
	}
      }
      else {
	if(v.gain < 0.1) {
	  ret.second = last_freq;
	  break; 
	}
      }
      last_freq = v.freq; 
    }
    return ret; 
  }
  
  void FilterSpec::sortSpec() {
    spec.sort([](const Corner & a, const Corner & b) { return a.freq < b.freq; });
    sorted = true; 
  }

  unsigned int FilterSpec::indexHproto(float freq) {
    unsigned int ret;
    float hsamprate = sample_rate / 2;
    ret = int(((freq + hsamprate) / sample_rate) * taps + 0.50001);
    if(ret >= taps) ret = taps - 1;
    
    return ret;
  }
}
