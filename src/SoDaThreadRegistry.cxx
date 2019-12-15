#include "SoDaThread.hxx"
#include "SoDaThreadRegistry.hxx"

SoDa::ThreadRegistry * SoDa::ThreadRegistry::registrar = NULL; 

SoDa::ThreadRegistry * SoDa::ThreadRegistry::getRegistrar() {
  if(registrar == NULL) {
    registrar = new ThreadRegistry;
  }
  return registrar; 
}

void SoDa::ThreadRegistry::addThread(SoDa::Thread * thread) {
  push_back(thread);
}

void SoDa::ThreadRegistry::apply(std::function<bool(SoDa::Thread*)> f) {
  for(auto el : *this) {
    f(el);
  }
}
