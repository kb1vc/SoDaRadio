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
  thread_list.push_back(thread);
}

void SoDa::ThreadRegistry::apply(std::function<bool(SoDa::ThreadPtr)> f) {
  for(auto el : thread_list) {
    f(el);
  }
}

void SoDa::ThreadRegistry::subscribeThreads(const std::vector<SoDa::MailBoxBasePtr> & mailboxes); 
  apply([mailboxes](SoDa::ThreadPtr el) ->
	bool { 
	  el->subscribeToMailes(mailboxes); 
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

