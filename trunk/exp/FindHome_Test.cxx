#include "../gui/FindHome.hxx"
#include <boost/format.hpp>
#include <iostream>

int main(int argc, char * argv[])
{
  std::string myhome = findHome();

  std::cerr << "My home is [" << myhome << "]" << std::endl;
}
