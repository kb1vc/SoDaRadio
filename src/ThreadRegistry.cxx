#include "Radio.hxx"
#include "Thread.hxx"
#include "ThreadRegistry.hxx"
#include "version.h"
#include <SoDa/Format.hxx>

namespace SoDa {

  ThreadRegistry * ThreadRegistry::registrar = NULL; 

  ThreadRegistry * ThreadRegistry::getRegistrar() {
    if(registrar == NULL) {
      registrar = new ThreadRegistry;
    }
    return registrar; 
  }

  void ThreadRegistry::addThread(Thread * thread, const std::string & version) {
    if(version != std::string(SoDaRadio_VERSION)) {
      throw Radio::Exception(Format("Thread %0 attempted to register.  It was built with version %1, but the registrar requires version %2\n")
			     .addS(thread->getObjName())
			     .addS(version)
			     .addS(SoDaRadio_VERSION).str(), thread);
    }
    thread_list.push_back(thread);
  }


  void ThreadRegistry::subscribeThreads() {
    for(auto el : thread_list) {
      el->subscribe();
    }
  }

  void ThreadRegistry::startThreads() {
    for(auto el : thread_list) {
      el->start(); 
    }
  } 
 
  void ThreadRegistry::joinThreads() {
    for(auto el : thread_list) {
      el->join();
    }
  }

  void ThreadRegistry::shutDownThreads() {
    for(auto el : thread_list) {
      el->shutDown(); 
    }
  }
}



