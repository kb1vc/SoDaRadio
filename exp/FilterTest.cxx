#include "../src/Filter.hxx"
#include <SoDa/Format.hxx>
#include <iostream>
#include <fstream>

void testShift() {
  std::vector<std::complex<float>> in_e(8), sout_e(8), isout_e(8);
  std::vector<std::complex<float>> in_o(9), sout_o(9), isout_o(9);  
  
  for(int i = 0; i < in_e.size(); i++) {
    in_e[i] = std::complex<float>(i, 0);
  }
  for(int i = 0; i < in_o.size(); i++) {
    in_o[i] = std::complex<float>(i, 0);
  }

  SoDa::FFT fft_e(8);
  SoDa::FFT fft_o(9);

  std::ofstream even("even.dat");
  std::ofstream odd("odd.dat");
  
  fft_e.shift(in_e, sout_e);
  fft_e.ishift(in_e, isout_e);
  fft_o.shift(in_o, sout_o);
  fft_o.ishift(in_o, isout_o);

  
  for(int i = 0; i < isout_o.size(); i++) {
    odd << SoDa::Format("%0 %1 %2 %3\n")
      .addI(i)
      .addF(in_o[i].real())
      .addF(sout_o[i].real())
      .addF(isout_o[i].real());
  }
  for(int i = 0; i < isout_e.size(); i++) {
    even << SoDa::Format("%0 %1 %2 %3\n")
      .addI(i)
      .addF(in_e[i].real())
      .addF(sout_e[i].real())
      .addF(isout_e[i].real());
  }
  odd.close();
  even.close();
}

float testFreq(SoDa::Filter & filt, double freq, double sample_rate, int l) {
  std::vector<std::complex<float>> in(l), out(l);

  double incr = 2.0 * M_PI * freq / sample_rate;
  double angle = -M_PI;

  for(int i = 0; i < l; i++) {
    in[i] = std::complex<float>(cos(angle), sin(angle));
    angle = angle + incr; 
  }
  
  // apply the filter
  filt.apply(in, out); 

  // calculate the magnitude at some point
  float mag = std::norm(out[l/2]);

  return mag; 
}

int main() {
  const float sample_rate = 1000.0;
  const int taps = 73; // 53;
  const int image_size = 2048;
  SoDa::FilterSpec spec(sample_rate, taps);
  spec.add(-60.0, 0)
    .add(-59, 1)
    .add(80, 1)
    .add(81,0);

  testShift();
  auto edge = spec.getFilterEdges();
  
  std::cout << SoDa::Format("### spec taps = %0 edges %1 %2\n")
    .addI(spec.getTaps())
    .addF(edge.first)
    .addF(edge.second);

  auto shape = spec.getSpec();

  for(auto c : shape) {
    std::cout << SoDa::Format("SHAPE %0 %1\n")
      .addF(c.first)
      .addF(c.second);
  }
  
  // now create the filter.
  SoDa::Filter filt(spec, image_size);
  filt.dump(std::cout);

  // and sweep
  double bump = 0.1 * sample_rate / image_size;
  for(double fr = -0.5 * sample_rate; fr < 0.5 * sample_rate; fr += bump) {
    float v = testFreq(filt, fr, sample_rate, image_size);
    std::cout << SoDa::Format("RESP %0 %1\n").addF(fr).addF(v,'e');
  }
}

