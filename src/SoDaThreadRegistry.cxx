#include "SoDaThread.hxx"
#include "SoDaThreadRegistry.hxx"
#include "version.h"
#include <SoDa/Format.hxx>

SoDa::ThreadRegistryPtr SoDa::ThreadRegistry::registrar = nullptr; 

SoDa::ThreadRegistryPtr SoDa::ThreadRegistry::getRegistrar() {
  if(registrar == NULL) {
    registrar = std::shared_ptr<SoDa::ThreadRegistry>(new ThreadRegistry);
  }
  return registrar; 
}


void SoDa::ThreadRegistry::priv_addThread(SoDa::ThreadPtr thread, const std::string & version) {
  if(version != std::string(SoDaRadio_VERSION)) {
    throw SoDa::Radio::Exception(SoDa::Format("Thread %0 attempted to register.  It was built with version %1, but the registrar requires version %2\n")
			  .addS(thread->getObjName())
			  .addS(version)
			  .addS(SoDaRadio_VERSION), thread);
  }
  thread_list.push_back(thread);
}

void SoDa::ThreadRegistry::priv_apply(std::function<bool(SoDa::ThreadPtr)> f) {
  for(auto el : thread_list) {
    f(el);
  }
}

void SoDa::ThreadRegistry::priv_subscribeThreads(const std::vector<SoDa::MailBoxBasePtr> & mailboxes) {
  apply([mailboxes](SoDa::ThreadPtr el) ->
	bool { 
	  el->subscribeToMailBoxes(mailboxes); 
	  return true;
	});

}

void SoDa::ThreadRegistry::priv_startThreads() {
  apply([](SoDa::ThreadPtr el) ->
	bool {
	  el->start();
	  return true; 
	});
} 
 
void SoDa::ThreadRegistry::priv_joinThreads() {
  apply([](SoDa::ThreadPtr el) ->
	bool {
	  el->join();
	  return true;
	});
}

void SoDa::ThreadRegistry::priv_shutDownThreads() {
  apply([](SoDa::ThreadPtr el) ->
	bool {
	  el->shutDown();
	  return true;
	});
}


void SoDa::ThreadRegistry::addThread(SoDa::ThreadPtr thread, const std::string & version) {
  auto reg = getRegistrar();
  reg->priv_addThread(thread, version);
}
    
void SoDa::ThreadRegistry::apply(std::function<bool(SoDa::ThreadPtr)> f) {
  auto reg = getRegistrar();
  reg->priv_apply(f);
}

void SoDa::ThreadRegistry::subscribeThreads(const std::vector<SoDa::MailBoxBasePtr> & mailboxes) {
  auto reg = getRegistrar();
  reg->priv_subscribeThreads(mailboxes);
} 
void SoDa::ThreadRegistry::startThreads() {
  auto reg = getRegistrar();
  reg->priv_startThreads();
}
void SoDa::ThreadRegistry::joinThreads() {
  auto reg = getRegistrar();
  reg->priv_joinThreads();
}
void SoDa::ThreadRegistry::shutDownThreads() {
  auto reg = getRegistrar();
  reg->priv_shutDownThreads();
}
