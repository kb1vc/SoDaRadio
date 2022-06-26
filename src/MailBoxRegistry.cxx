#include "MailBoxRegistry.hxx"
#include <SoDa/Format.hxx>

namespace SoDa {
  MailBoxRegistry * MailBoxRegistry::registrar = nullptr; 

  MailBoxRegistry * MailBoxRegistry::getRegistrar() {
    if(registrar == NULL) {
      registrar = new MailBoxRegistry;
    }
    return registrar; 
  }
  
  void MailBoxRegistry::add(const std::string & name, 
				 std::shared_ptr<MailBoxBase> mailbox_ptr) {
    if(exists(name)) {
      throw MailBoxExists(name); 
    }
    
    mailboxes[name] = mailbox_ptr; 
  }

  std::shared_ptr<MailBoxBase> MailBoxRegistry::get(const std::string & name) {
    if(!exists(name)) {
      throw MailBoxMissing(name);
    }
    
    else return mailboxes[name]; 
  }

  bool MailBoxRegistry::exists(const std::string & name) {
    return mailboxes.find(name) != mailboxes.end(); 
  }
  
}

