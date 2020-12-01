The rules for contributing code to the SoDaRadio project are simple: 
1. The code you provide must be your own. Do not lift code from other projects.
Yes, the other projects may be open-source, but they may or may not have
compatible license terms.  We owe the originators of the code the respect
we'd expect from them. So if the code you need comes from another project, 
post an issue and let's talk about it.  Maybe importing the code is fine, 
but we need to check license terms and such. 
2. The code you provide will be covered by the BSD 2 clause license: https://opensource.org/licenses/BSD-2-Clause
It is a pretty open license. But you must be willing to contribute on that basis. 
3. Be clean. Write clean code. Avoid dragging in other libraries or dependencies. 
4. Write your code in C++.  Don't write FORTRAN with C++ syntax.;)  Do the best you can to do it the C++ way.
When in doubt, post an issue and we can talk about it. 
5. Be gentle. Some of the code in SoDa dates back eight years or more. It has some barnacles: 
* There are a lot of boostisms and boost stuff still in the code.  I'm trying to remove boost
from the dependencies, as much of it has been suplanted by modern C++. 
* Some of my earlier design decisions were misguided or predated a more nuanced understanding of "the C++ way." 
I am working on reforming.  We are all works in progress. 
6. Minimize the blast radius. If you find yourself changing 10 files to add a single feature, 
you are probably doing something wrong or the partitioning of the original code wasn't right. 
7. Remember that SoDaRadio is supposed to run across the entire USRP range. Be careful around model specific features.

All that said, I welcome suggestions and contributions. It make take a while to get to them. 

If you don't feel comfortable contributing a change, but want to add a feature, post an issue and we'll talk 
about it.  
