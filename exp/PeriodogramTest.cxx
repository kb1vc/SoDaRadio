#include "../src/Periodogram.hxx"
#include <cmath>
#include <iostream>
#include <fstream>
#include <complex>
#include <vector>

#include <SoDa/Format.hxx>

int main() {
  // create a really long vector
  const uint32_t big = 7935; // use a strange length
  const uint32_t num_trials = 37;
  std::vector<std::complex<float>> x(big);

  SoDa::Periodogram pdg(1024);

  std::ofstream wof("Window.dat");
  for(auto v : pdg.window) {
    wof << SoDa::Format("%0\n").addF(v,'e');
  }
  wof.close();
  
  float ang[2] = {0.0, 0.0};
  float anginc[2] = { 3.0 * M_PI / 57.0, -7.0 * M_PI / 61.0 };
  
  for(int tr = 0; tr < num_trials; tr++) {
    for(auto & xe : x) {
      xe = 0.0; 
      for(int i = 0; i < 2; i++) {
	xe += std::complex<float>(cos(ang[i]), sin(ang[i])); 
	ang[i] = ang[i] + anginc[i];
	if(ang[i] > M_PI) ang[i] = ang[i] - 2.0 * M_PI;
	if(ang[i] < M_PI) ang[i] = ang[i] + 2.0 * M_PI; 
      }
    }
    pdg.accumulate(x);         
  }

  // what does the accumulated thingie look like
  auto res = pdg.get();
  
  auto sf = pdg.getScaleFactor();
  for(const auto v : res) {
    std::cout << SoDa::Format("%0 %1\n")
      .addF(std::abs(v) * sf, 'e')
      .addF(v, 'e');

  }
}
