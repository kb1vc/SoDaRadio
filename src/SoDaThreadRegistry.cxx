#include "SoDaThread.hxx"
#include "SoDaThreadRegistry.hxx"
#include "version.h"

SoDa::ThreadRegistry * SoDa::ThreadRegistry::registrar = NULL; 

SoDa::ThreadRegistry * SoDa::ThreadRegistry::getRegistrar() {
  if(registrar == NULL) {
    registrar = new ThreadRegistry;
  }
  return registrar; 
}

void SoDa::ThreadRegistry::addThread(SoDa::Thread * thread, const std::string & version) {
  if(version != std::string(SoDaRadio_VERSION)) {
    throw SoDa::Exception((boost::format("Thread %s attempted to register.  It was built with version %s, but the registrar requires version %s\n")
			   % thread->getObjName() % version % SoDaRadio_VERSION).str(), thread);
  }
  push_back(thread);
}

void SoDa::ThreadRegistry::apply(std::function<bool(SoDa::Thread*)> f) {
  for(auto el : *this) {
    f(el);
  }
}

void SoDa::ThreadRegistry::subscribeThreads(const SoDa::MailBoxMap & mailbox_map) {
  apply([mailbox_map](SoDa::Thread * el) ->
	bool { 
	  el->subscribeToMailBoxList(mailbox_map); 
	  return true;
	});

}

void SoDa::ThreadRegistry::startThreads() {
  apply([](SoDa::Thread * el) ->
	bool {
	  el->start();
	  return true; 
	});
} 
 
void SoDa::ThreadRegistry::joinThreads() {
  apply([](SoDa::Thread * el) ->
	bool {
	  el->join();
	  return true;
	});
}

void SoDa::ThreadRegistry::shutDownThreads() {
  apply([](SoDa::Thread * el) ->
	bool {
	  el->shutDown();
	  return true;
	});
}

