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

  Filter::Filter(FilterSpec & filter_spec, 
		 unsigned int image_size) : image_size(image_size) {
    auto num_taps = filter_spec.getTaps();    
    // first create a frequency domain ideal filter:
    std::vector<std::complex<float>> Hproto(num_taps);
    std::vector<std::complex<float>> hproto(num_taps);    

    // crawl through the filter corners
    float cur_value = 0.0;

    auto spec = filter_spec.getSpec();

    auto sr = filter_spec.getSampleRate();
    for(float fr = -0.5 * sr; fr < 0.5 * sr; fr += 0.01 * sr) {
      std::cout << SoDa::Format("IDX %0 %1\n")
	.addF(fr,'e')
	.addI(filter_spec.indexHproto(fr));
    }
    
    int cur_idx = 0; 
    for(auto v : spec) {
      auto nidx = filter_spec.indexHproto(v.first);
      float incr = (v.second - cur_value) / ((float) (nidx - cur_idx));
      std::cout << SoDa::Format("HFILL %0 %1 %2\n").addI(nidx).addF(incr).addF(v.second);
      for(int i = cur_idx; i < nidx; i++) {
	Hproto[i].real(cur_value + ((float) (i - cur_idx)) * incr);
	Hproto[i].imag(0.0);
	std::cout << SoDa::Format("HLOOP %0 %1 %2 0.5\n")
	  .addI(i)
	  .addF(Hproto[i].real())
	  .addF(Hproto[i].imag());
      }
      Hproto[nidx] = cur_value; 
      std::cout << SoDa::Format("HLOOP %0 %1 %2 0.6\n")
	.addI(nidx)
	.addF(Hproto[nidx].real())
	.addF(Hproto[nidx].imag());
      cur_idx = nidx + 1; 
      cur_value = v.second; 
    }
    for(int i = cur_idx; i < num_taps; i++) {

      Hproto[i].real(cur_value);
      Hproto[i].imag(0.0);
      std::cout << SoDa::Format("HLOOP %0 %1 %2 0.7\n")
	.addI(i)
	.addF(Hproto[i].real())
	.addF(Hproto[i].imag());      
    }

    // now we've got a frequency domain prototype.
    // first shift it to fit the FFT picture
    FFT pfft(num_taps);
    pfft.shift(Hproto, Hproto);
    
    // transform it to the time domain
    pfft.ifft(Hproto, hproto);

    // shift that back to 0 in the middle
    pfft.ishift(hproto, hproto);

    for(int i = 0; i < num_taps; i++) {
      std::cout << SoDa::Format("Hproto %0 %1 %2 %3 %4\n")
	.addI(i)
	.addF(Hproto[i].real())
	.addF(Hproto[i].imag())
	.addF(hproto[i].real())
	.addF(hproto[i].imag());
    }
    
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
      h[i] = hproto[i] / ((float) num_taps); 
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
    for(int i = 0; i < M; i++) {
      w[i] = 0.54 - 0.46 * cos(anginc * (float(i)));
    }
  }

  unsigned int Filter::apply(std::vector<std::complex<float>> & in_buf, 
			     std::vector<std::complex<float>> & out_buf, 
			     float outgain,
			     InOutMode in_out_mode) {
    if((in_buf.size() != image_size) || (out_buf.size() != image_size)) {
      throw BadBufferSize("apply", in_buf.size(), out_buf.size(), image_size); 
    }
    if(temp_buf.size() != image_size) {
      temp_buf.resize(image_size); 
    }

    std::vector<std::complex<float>> * tbuf_ptr;
    if(in_out_mode.time_in) {
      // first do the transform
      fft->fft(in_buf, temp_buf);
      tbuf_ptr = & temp_buf; 
    }
    else {
      // we're already taking a transform as input. 
      tbuf_ptr = & in_buf; 
    }
    
    
    if(in_out_mode.time_out) {
      // now multiply 
      for(int i = 0; i < tbuf_ptr->size(); i++) {
	(*tbuf_ptr)[i] = (*tbuf_ptr)[i] * H[i] * outgain; 
      }
      // invert
      fft->ifft(*tbuf_ptr, out_buf); 
    }
    else {
      // they want frequency output
      // multiply directly into output buffer
      for(int i = 0; i < out_buf.size(); i++) {
	out_buf[i] = (*tbuf_ptr)[i] * H[i] * outgain; 
      }
    }
    return in_buf.size();
  }

  unsigned int Filter::apply(std::vector<float> & in_buf, 
			     std::vector<float> & out_buf, 
			     float out_gain,
			     InOutMode in_out_mode) {
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

    for(int i = 0; i < out_buf.size(); i++) {
      out_buf[i] = temp_out_buf[i].real();
    }
    return in_buf.size();    
  }
  
  void Filter::dump(std::ostream & os) {
    for(int i = 0; i < H.size(); i++) {
      os << SoDa::Format("FILTER %0 %1 %2 %3 %4\n")
	.addI(i)
	.addF(H[i].real())
	.addF(H[i].imag())
	.addF(h[i].real())
	.addF(h[i].imag());
    }
  }
}
