#include <boost/format.hpp>
#include "RangeMap.hxx"
#include <string>

int main()
{
  SoDa::Range<double> k1(1.0, 2.0);
  SoDa::RangeMap<double, std::string> rmap;


  rmap[k1] = std::string("One-to-two");
  
  rmap[SoDa::Range<double>(2.0, 3.0)] = std::string("two-to-three");
  rmap[SoDa::Range<double>(3.0, 4.0)] = std::string("three-to-four");

  std::cerr << rmap[2.99999] << std::endl; 
}
