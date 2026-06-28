#include "../src/FindHome.hxx"
#include <iostream>

int main(int argc, char * argv[])
{
  (void) argc; (void) argv; 
  std::string myhome = findHome();

  std::cerr << "My home is [" << myhome << "]" << std::endl;
}
