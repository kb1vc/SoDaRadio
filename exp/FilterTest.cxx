#include "../src/Filter.hxx"
#include <SoDa/Format.hxx>
#include <iostream>
#include <fstream>
#include <chrono>


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

struct Response {
  Response(float m, float p) : mag(m), phase(p) {}
  float mag;
  float phase;
}; 

Response testFreq(SoDa::Filter & filt, double freq, double sample_rate, 
	       std::vector<std::complex<float>> in,
	       std::vector<std::complex<float>> out, 
	       double & tdur) {	       

  int l = in.size();
  double incr = 2.0 * M_PI * freq / sample_rate;
  double angle = -M_PI;

  for(int i = 0; i < l; i++) {
    in[i] = std::complex<float>(cos(angle), sin(angle));
    angle = angle + incr; 
  }

  auto cr_st = std::chrono::high_resolution_clock::now();
  // apply the filter
  filt.apply(in, out); 
  auto cr_end = std::chrono::high_resolution_clock::now();

  tdur = tdur + double(std::chrono::duration_cast<std::chrono::nanoseconds>(cr_end - cr_st).count());
  // calculate the magnitude at some point
  float mag = std::norm(out[l/2]);

  float phase = std::arg(out[l/2]) - std::arg(in[l/2]);
  while(phase > M_PI) {
    phase -= 2.0 * M_PI; 
  }
  while(phase < -M_PI) {
    phase += 2.0 * M_PI; 
  }
  return Response(mag, phase); 
}

int main() {
  const float sample_rate = 1000.0;
  const int taps = 133; // 53;
  const int image_size = 4096;
  SoDa::FilterSpec spec(sample_rate, taps);
  spec.add(-100.0, -50)
    .add(-60, 0)
    .add(-40, 0)
    .add(-10, -3)
    .add(20, -3)
    .add(60, 0)
    .add(80, 0)
    .add(100, -30)
    .add(320, -30)
    ;


  // testShift();
  auto edge = spec.getFilterEdges();
  
  std::cout << SoDa::Format("### spec taps = %0 edges %1 %2\n")
    .addI(spec.getTaps())
    .addF(edge.first)
    .addF(edge.second);

  auto shape = spec.getSpec();

  for(auto c : shape) {
    std::cout << SoDa::Format("SHAPE %0 %1\n")
      .addF(c.freq)
      .addF(c.gain);
  }
  
  // now create the filter.
  SoDa::Filter filt(spec, image_size);

  // and sweep
  unsigned int itercount = 0; 
  double bump = 1.0 * sample_rate / image_size;
  std::vector<std::complex<float>> in(image_size), out(image_size); 

  double tdur; 
  
  for(double fr = -0.5 * sample_rate; fr < 0.5 * sample_rate; fr += bump) {
    Response v = testFreq(filt, fr, sample_rate, in, out, tdur);
    std::cout << SoDa::Format("RESP %0 %1 %2\n").addF(fr).addF(v.mag,'e').addF(v.phase,'e');
    itercount++; 
  }

  double diter = double(itercount);
  double pts = diter * double(image_size);

  std::cerr << SoDa::Format("Time per iteration (ns) %0 per point %1\n")
    .addF(tdur / diter, 'e')
    .addF(tdur / pts, 'e');
}

