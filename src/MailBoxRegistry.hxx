#pragma once
/*
Copyright (c) 2022 Matthew H. Reilly (kb1vc)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in
    the documentation and/or other materials provided with the
    distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <memory>
#include <map>
#include "Debug.hxx"

 /**
  * @file MailBoxRegistry.hxx
  * 
  * A singleton object that records instances of mailboxes.
  *
  * This allows creation of mailboxes by plugins as well as the 
  * "normal" SoDaRadio components.  
  *
  * @author Matt Reilly (kb1vc)
  *
  */

#include <SoDa/MailBox.hxx>

namespace SoDa { 
  
  class MailBoxRegistry {
  public:

    static MailBoxRegistry * getRegistrar();

    /**
     * @brief register a mailbox
     * 
     * @param name the name of the mailbox
     * @param mailbox_ptr a shared_ptr to the mailbox. 
     *
     */
    void add(const std::string & name, 
		  std::shared_ptr<MailBoxBase> mailbox_ptr);

    std::shared_ptr<MailBoxBase> get(const std::string & name);

    bool exists(const std::string & name); 

    class MailBoxExists : public SoDa::Exception {
    public:
      MailBoxExists(const std::string & name) : 
	SoDa::Exception(SoDa::Format("MailBoxRegistry: attempt to register mailbox with name \"%0\" but this name is already taken.\n")
			.addS(name).str())
      {
      }
    }; 

    class MailBoxMissing : public SoDa::Exception {
    public:
      MailBoxMissing(const std::string & name) : 
	SoDa::Exception(SoDa::Format("MailBoxRegistry: can't find mailbox named \"%0\"\n")
			.addS(name).str())
      {
      }
    }; 

  private:
    MailBoxRegistry() { }    

    std::map<std::string, std::shared_ptr<MailBoxBase>> mailboxes; 
    
    static MailBoxRegistry * registrar; 
  };
}

