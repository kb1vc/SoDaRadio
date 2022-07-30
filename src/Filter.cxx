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

#include "Filter.hxx"

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
    float last_amp;
    float last_corner;
    last_amp = spec.front().second;
    last_corner = spec.front().first;
    for(const auto & v : spec) {
      if(v.second != last_amp) {
	float inter = v.second - last_amp;
	if(inter < min_interval) min_interval = inter;
	last_amp = v.second;
	last_coener = v.first;
      }
    }
    
    // now use fred harris's rule for the number of filter taps?
    // assume 40 dB stop band attenuation
    float ftaps = sample_rate * 40 / (min_interval * 22);
    float comp_taps = 2 * int(ftaps / 2) + 1; // make sure it is odd

    return comp_taps;
  }

  FilterSpec & FilterSpec::start(float amp) {
    sorted = false;
    float start_freq = (filter_type == REAL) ? 0.0 : - (sample_rate / 2);
    spec.push_back(Corner(start_freq, amp));
  }

  FilterSpec & FilterSpec::add(float freq, float amp) {
    sorted = false;
    if((filter_type == REAL) && (freq < 0)) {
      throw BadRealSpec("add", freq);
    }
    spec.push_back(Corner(start_freq, amp));    
  }
      
  const std::list<Corner> FilterSpec::getSpec() {
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
	if(v.second > (1 - 0.01)) {
	  ret.first = v.first;
	  looking_low = false; 
	}
      }
      else {
	if(v.second < 0.1) {
	  ret.second = last_freq;
	  break; 
	}
      }
      last_freq = v.first; 
    }
    return ret; 
  }

  void FilterSpec::sortSpec() {
    spec.sort([](const Corner & a, const Corner & b) { return a.first > b.first; });
    sorted = true; 
  }

  unsigned int FilterSpec::indexHproto(float freq) {
    unsigned int ret;
    float hsamprate = sample_rate / 2;
    ret = int(((freq + hsamprate) / sample_rate) * taps + 0.50001);
    return ret;
  }

  Filter::Filter(FilterSpec & filter_spec, 
		 unsigned int image_size) {
    auto num_taps = filter_spec.getTaps();    
    // first create a frequency domain ideal filter:
    std::vector<std::complex<float>> Hproto(num_taps);

    // crawl through the filter corners
    float cur_value;

    auto spec = filter_spec.getSpec();
    
    int cur_idx = 0; 
    for(auto v : spec) {
      auto nidx = filter_spec.indexHproto(v.first);
      float incr = (v.second - cur_value) / ((float) (nidx - cur_idx));
      for(int i = cur_idx; i < nidx; i++) {
	Hproto[i] = cur_value + ((float) (i + 1 - cur_idx)) * incr;
      }
      Hproto[nidx] = cur_value; 
      cur_idx = nidx; 
      cur_value = v.second; 
    }
    for(int i = cur_idx; i < num_taps; i++) {
      Hproto[i] = cur_value; 
    }

    // now we've got a frequency domain prototype.
    // first shift it to fit the FFT picture
    FFT pfft(num_taps);
    pfft.shift(Hproto, Hproto);
    
    // transform it to the time domain
    pfft.ifft(Hproto, hproto);

    // shift that back to 0 in the middle
    pfft.ishift(hproto, hproto);

    // now apply a window
    std::vector<float> window(num_taps);
    hammingWindow(window);
    for(int i = 0; i < num_taps; i++) {
      hproto[i] = hproto[i] * window[i];
    }

    // so now we have the time domain prototype.

    // embed it in the impulse response of the appropriate length
    h.resize(image_size);
    for(int i = 0; i < num_taps; i++) {
      h[i] = hproto[i]; 
    }
    // zero the rest
    for(int i = num_taps; i < image_size; i++) {
      h[i] = std::complex<float>(0.0, 0.0);
    }
    
    // now make the frequency domain filter
    
    fft = std::unique_ptr<FFT>(new FFT(image_size));
    H.resize(image_size);
    
    fft->fft(h, H);

    // and now we have it. But we need to rescale by image_size
    float scale = 1/((float) image_size);
    for(auto & v : H) {
      v = v * scale; 
    }
  }

  void Filter::hammingWindow(std::vector<float> & w) {
    int M = w.size();
    
    float anginc = 2 * M_PI / ((float) (M - 1));
    for(i = 0; i < M; i++) {
      w[i] = 0.54 - 0.46 * cos(anginc * (float(i)));
    }
  }

  unsigned int Filter::apply(std::vector<std::complex<float>> & in_buf, 
			     std::vector<std::complex<float>> & out_buf, 
			     float outgain) {
    if((in_buf.size() != image_size) || (out_buf.size() != image_size)) {
      throw BadBufferSize("apply", in_buf.size(), out_buf.size(), image_size); 
    }
    if(temp_buf.size() != image_size) {
      temp_buf.resize(image_size); 
    }

    // first do the transform
    fft->fft(in_buf, temp_buf);
    
    // now multiply
    for(int i = 0; i < temp_buf.size(); i++) {
      temp_buf[i] = temp_buf[i] * H[i] * outgain; 
    }
    
    // and invert
    fft->ifft(temp_buf, out_buf); 
  }

  unsigned int Filter::apply(std::vector<float> & in_buf, 
			     std::vector<float> & out_buf, 
			     float out_gain) { 
    if((in_buf.size() != image_size) || (out_buf.size() != image_size)) {
      throw BadBufferSize("apply", in_buf.size(), out_buf.size(), image_size); 
    }

    if(temp_in_buf.size() != image_size) {
      temp_in_buf.resize(image_size); 
    }
    if(temp_out_buf.size() != image_size) {
      temp_out_buf.resize(image_size); 
    }
    // first fill a complex vector
    for(int i = 0; i < in_buf.size(); i++) {
      temp_in_buf[i] = std::complex<float>(in_buf[i], 0.0); 
    }
    // now apply 
    apply(temp_in_buf, temp_out_buf, out_gain);

    for(int i = 0; i < out_buf_size(); i++) {
      out_buf[i] = temp_out_buf[i].real();
    }
  }
}
