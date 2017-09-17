#include <boost/format.hpp>
#include "ProcInfo.hxx"
#include <string>
#include <iostream>
#include <unistd.h>
int main()
{
  kb1vc::ProcInfo pi("proc_info_test.dat", "test_proc");

  int * intp;

  for(int i = 0; i < 10; i++) {
    pi.reportInfo(i > 0); 
    pi.printInfo(std::cerr); 
    intp = new int[(i + 1) * 4096 * 64]; 

    intp[i] = i * i;
    pi.printInfo(std::cerr);     
    sleep(1); 
  }
}
