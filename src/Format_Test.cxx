#include "Format.hxx"
#include <iostream>

int main() {

  
  int i = 5;
  double f = 6;
  double ef = 3215.0;
  char c = 'T';
  std::string fred("Fred");
    
  std::cout <<   SoDa::Format("this %% is %0 a test %1 repeat the first %0 engineering %2\n").addI(i).addD(f).addD(ef, 'e');      
  

  std::cout << SoDa::Format("int [%0] double f [%1] double g [%2] double e [%3] string [%4] char [%5] char6[%6] char7[%7] char8[%8] char9[%9] char10[%10] char11[%11]\n")
    .addI(i).addD(f, 'f').addD(f, 'g').addD(f, 'e').addS(fred).addC(c)
    .addC('6')
    .addC('7')
    .addC('8')
    .addC('9')
    .addC('A')
    .addC('B');    

  SoDa::Format format("%0 p = %1 : [%2]\n");
  
  std::list<char> fs;
  fs.push_back('f'); fs.push_back('g'); fs.push_back('e'); fs.push_back('s'); 
  for(i = 0; i < 10; i++) {
    if(i > 8) {
      std::cout << "Gimme a number: "; std::cout.flush();
      std::cin >> ef;
    }
    std::cout << "---------\n" << ef << "\n--------------\n";    
    for(char fmt : fs) {
      for(int p = 1; p < 7; p++) {
        std::cout <<   format.reset().addC(fmt).addI(p).addD(ef, fmt, p+10, p);
      }
    }
    ef = ef * 0.1;
    std:: cout << "\n\n";
  }
}
