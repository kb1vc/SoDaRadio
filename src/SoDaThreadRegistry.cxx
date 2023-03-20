#include "SoDaThread.hxx"
#include "SoDaThreadRegistry.hxx"
#include "version.h"
#include <SoDa/Format.hxx>

SoDa::ThreadRegistry * SoDa::ThreadRegistry::registrar = NULL; 

SoDa::ThreadRegistry * SoDa::ThreadRegistry::getRegistrar() {
  if(registrar == NULL) {
    registrar = new ThreadRegistry;
  }
  return registrar; 
}

void SoDa::ThreadRegistry::addThread(SoDa::Thread * thread, const std::string & version) {
  if(version != std::string(SoDaRadio_VERSION)) {
    throw SoDa::Radio::Exception(SoDa::Format("Thread %0 attempted to register.  It was built with version %1, but the registrar requires version %2\n")
			  .addS(thread->getObjName())
			  .addS(version)
			  .addS(SoDaRadio_VERSION), thread);
  }
  threads.push_back(thread);
}

void SoDa::ThreadRegistry::subscribeThreads(SoDa::MailBoxMap & mailbox_map) {
  std::cerr << "Subscribing all threads\n";
  for(auto el : threads) {
    std::cerr << "Subscribing thread " << el->getObjName() << "\n";
    el->subscribeToMailBoxList(mailbox_map); 
  }
}

void SoDa::ThreadRegistry::startThreads() {
  for(auto el : threads) {
    el->start();
  }
} 
 
void SoDa::ThreadRegistry::joinThreads() {
  for(auto el : threads) {
    el->join();
  }
}

void SoDa::ThreadRegistry::shutDownThreads() {
  for(auto el : threads) {
    el->shutDown();
  }
}

