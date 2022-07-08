#include "MailBoxRegistry.hxx"
#include <SoDa/Format.hxx>

namespace SoDa {
  MailBoxRegistry * MailBoxRegistry::registrar = nullptr; 
  std::mutex MailBoxRegistry::reg_mutex; 

  MailBoxRegistry * MailBoxRegistry::getRegistrar() {
    std::lock_guard<std::mutex> lock(reg_mutex);
    if(registrar == NULL) {
      registrar = new MailBoxRegistry;
    }
    return registrar; 
  }
  
  void MailBoxRegistry::add(const std::string & name, 
				 std::shared_ptr<MailBoxBase> mailbox_ptr) {
    std::lock_guard<std::mutex> lock(reg_mutex);    
    if(exists(name)) {
      throw MailBoxExists(name); 
    }
    
    mailboxes[name] = mailbox_ptr; 
  }

  std::shared_ptr<MailBoxBase> MailBoxRegistry::get(const std::string & name) {
    std::lock_guard<std::mutex> lock(reg_mutex);    
    if(!exists(name)) {
      throw MailBoxMissing(name);
    }
    
    else return mailboxes[name]; 
  }

  bool MailBoxRegistry::exists(const std::string & name) {
    return mailboxes.find(name) != mailboxes.end(); 
  }
  
}

