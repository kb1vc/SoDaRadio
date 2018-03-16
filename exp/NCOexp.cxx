#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <cmath>
#include <complex>

class NCO {
public: 
  NCO(double _fsamp) {
    fsamp = _fsamp;
  }

  virtual void setFreq(double f) = 0; 

  virtual std::complex<double> step() = 0; 

  double fsamp;
};

class NCOSinCos : public NCO {
public:
  NCOSinCos(double _fsamp) : NCO(_fsamp) {
    idx = 0; 
    ang = 0.0; 
  }

  void setFreq(double f) {
    ainc = 2.0 * M_PI * f / fsamp;
  }

  std::complex<double> step() {
    double s, c; 
    ang += ainc; 
    ang = (ang > M_PI) ? (ang - (2.0 * M_PI)) : ang; 
    sincos(ang, &s, &c); 
    return std::complex<double>(c, -s);
  }

private:
  double ang; 
  double ainc; 
  int idx; 
}; 


class NCORecursive : public NCO {
public:
  NCORecursive(double _fsamp) : NCO(_fsamp) {
    idx = 0; 
    ang = 0.0; 
    ejw = std::complex<double>(1.0, 0.0);
    last = std::complex<double>(1.0, 0.0);
  }

  void setFreq(double f) {
    ainc = 2.0 * M_PI * f / fsamp; 
    ejw = exp(std::complex<double>(0.0, -ainc));
  }

  std::complex<double> step() {
    std::complex<double> nval;
    idx++;
    ang += ainc; 
    ang = (ang > M_PI) ? (ang - (2.0 * M_PI)) : ang; 
    if(idx == 2048) {
      idx = 0; 
      double s, c; 
      sincos(ang, &s, &c); 
      nval = std::complex<double>(c, -s);
    }
    else {
      nval = last * ejw; 
    }
    last = nval;
    return nval; 
  }

private:
  double ang; 
  double ainc; 
  std::complex<double> ejw, last;
  int idx; 
}; 

class NCORecursive2 : public NCO {
public:
  NCORecursive2(double _fsamp) : NCO(_fsamp) {
    idx = 0; 
    ejw = std::complex<double>(1.0, 0.0);
    last = std::complex<double>(1.0, 0.0);
  }

  void setFreq(double f) {
    double ainc = 2.0 * M_PI * f / fsamp; 
    ejw = exp(std::complex<double>(0.0, -ainc));
  }

  std::complex<double> step() {
    std::complex<double> nval;
    idx++;
    nval = last * ejw;     
    if(idx == 512) {
      idx = 0; 
      nval = nval / abs(nval);
    }
    last = nval;
    return nval; 
  }

private:
  std::complex<double> ejw, last;
  int idx; 
}; 


int main(int argc, char * argv[])
{
  unsigned long testcount = 100000000;
  //  unsigned long testcount = 10000;


  if(argc == 1) {
    // compare the two schemes
    double maxerr[2] = { 0.0, 0.0 }; 
    double toterr[2] = { 0.0, 0.0 }; 
    NCOSinCos sco(100e3);
    NCORecursive ro(100e3);
    NCORecursive2 ro2(100e3);

    sco.setFreq(2.5325e3);
    ro.setFreq(2.5325e3);
    ro2.setFreq(2.5325e3);        
    std::complex<double> scv, rv, rv2; 
    for(unsigned long k = 0; k < 10; k++) {
      for(unsigned long i =  0; i < testcount; i++) {
	scv = sco.step();
	rv = ro.step();
	rv2 = ro2.step();      
	// std::cout << boost::format("%d %g %g %g %g\n")
	//  	% i % scv.real() % scv.imag() % rv.real() % rv.imag(); 
	std::complex<double> diff[2];
	diff[0] = rv - scv;
	diff[1] = rv2 - scv; 
	for(int j = 0; j < 2; j++) {
	  double err = abs(diff[j]);	
	  if(err > maxerr[j]) maxerr[j] = err; 
	  toterr[j] += err; 
	}

      }

      for(int j = 0; j < 2; j++) {
	std::cout << boost::format("%d : RO[%d] Max error = %g  average error = %g\n")
	  % k % j % maxerr[j] % (toterr[j] / ((double) testcount));
      }
    }
  }
  else if(argv[1][0] == 't') {
    // do the trig version
    double sum = 0.0; 
    NCOSinCos o(100e3);
    o.setFreq(2.5325e3);
    std::complex<double> v;
    for(unsigned long i =  0; i < testcount; i++) {
      v = o.step();
      sum += v.real();
    }
    std::cout << "NCOSinCos sum = " <<  sum << std::endl; 
  }
  else if(argv[1][0] == '2') {
    // do the recursive gain compensation version
    double sum = 0.0; 
    NCORecursive2 o(100e3);
    o.setFreq(2.5325e3);
    std::complex<double> v;
    for(unsigned long i =  0; i < testcount; i++) {
      v = o.step();
      sum += v.real();
    }
    std::cout << "NCORecursive2 sum = " << sum << std::endl; 
  }
  else {
    // do the recursive version
    double sum = 0.0; 
    NCORecursive o(100e3);
    o.setFreq(2.5325e3);
    std::complex<double> v;
    for(unsigned long i =  0; i < testcount; i++) {
      v = o.step();
      sum += v.real();
    }
    std::cout << "NCORecursive sum = " << sum << std::endl; 
  }
}
