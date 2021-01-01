#include <SoDaFormat/Format.hxx>
#include <iostream>

int main() {
  SoDa::Format sft("Avogadro's number: %0\n"); 

  // Print Avogadro's number the way we all remembered from high school. 
  double av = 6.02214076e23;
  std::cout << sft.addF(av, 's');

  // But all that 10^23 jazz is the result of the evil CGS system.
  // Let's do this the way a respectable MKS user would have wanted it.
  std::cout << "Here's how right thinking people write "
            << sft.reset().addF(av, 'e');
}
