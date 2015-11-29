#include <iostream>
#include <boost/format.hpp>

#include "Demod.hxx"
using namespace SoDa; 
int main()
{
  Demod * dm0 = new DemodUSB();
  Demod * dm1 = new DemodLSB();

  SoDaBuf * buf = new SoDaBuf(1024);
  float out[1024]; 
  dm0->demodAudio(buf, out, 1.0);
  dm1->demodAudio(buf, out, 1.0); 

  Demod * dmm; 
  dmm = Demod::getDemodulator("USB");
  if(dmm != NULL) dmm->demodAudio(buf, out, 1.0);  
  else std::cout << "USB not found" << std::endl; 
  dmm = Demod::getDemodulator("LSB");
  if(dmm != NULL) dmm->demodAudio(buf, out, 1.0);    
  else std::cout << "LSB not found" << std::endl;   
}
