/*
  Copyright (c) 2012, Matthew H. Reilly (kb1vc)
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

#include "MultiMBox.hxx"
#include "SoDaBase.hxx"
#include "SoDaThread.hxx"
#include <memory>
#include <iostream>
#include <string>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

boost::mutex iomutex;
namespace SoDaTest {
  
  class MyMsg : public SoDa::MBoxMessage {
  public:
    enum CMD { START, BCAST, KILL, STOP };
    MyMsg(const std::string ss, CMD _c) {
      s = ss;
      cmd = _c;
      {
	std::cerr << "Creating message [" << s << "," << cmd << "]" << std::endl;
      }
    }

    MyMsg(const MyMsg & other) {
      s = other.s;
      cmd = other.cmd;
    }

    MyMsg() {
      s = "Dunno"; 
    }

    ~MyMsg() {
      std::cerr << "Destroying message [" << s << "," << cmd << "]" << std::endl;
    }
    std::string s;
    CMD cmd; 
  };

  typedef std::shared_ptr<MyMsg> MyMsgPtr; 

  MyMsgPtr makeMsg(const std::string ss, MyMsg::CMD _c) {
    return std::make_shared<MyMsg>(ss, _c);
  }
  
  class MultiMBox_Test_Thread: public SoDa::Thread {
  public:
    MultiMBox_Test_Thread(const std::string & n) : SoDa::Thread(n) {
      name = n;
    }

    void subscribe(std::shared_ptr<SoDa::MultiMBox<MyMsgPtr>> mbox) {
      subscriptions.push_back(mbox->subscribe());
      mboxes.push_back(mbox);
    }

    void execute(MyMsgPtr msg, int i) {
      {
	boost::mutex::scoped_lock lock(iomutex);
	std::cerr << "Process " << name << "got message " << msg->cmd << " " << msg->s << " subcount = " << mboxes.size() << " i = " << i << std::endl;
      }
      switch (msg->cmd) {
      case MyMsg::START:
	if((i + 1) < mboxes.size()) {
	  mboxes[i+1]->put(makeMsg("from" + name, MyMsg::KILL));
	  name = name + ".";
	}
	break;
      case MyMsg::BCAST:
	break;
      case MyMsg::KILL:
	if((i + 1) < mboxes.size()) {
	  mboxes[i+1]->put(makeMsg("from" + name, MyMsg::KILL));
	  name = name + ".";
	}
	break;
	break;
      case MyMsg::STOP:
	break;
      default:
	break; 
      }
    }
  
    void run()
    {
      // iterate through our subscriptions and print out the
      // contents of each one.
      int i;
      MyMsgPtr msg;
      while(1) {
	bool flag = false; 
	for(i = 0; i < mboxes.size(); i++) {
	  // check to see if there is a message on the mailbox.
	  if((msg = (mboxes[i]->get(subscriptions[i])))) {
	    execute(msg, i);
	    flag = true; 
	  }
	}

	if(!flag) usleep(10000);
      }
    }
  
  
    std::string name;

    std::vector<int> subscriptions;
    std::vector<std::shared_ptr<SoDa::MultiMBox<MyMsgPtr>>> mboxes;
  }; 
}

int main(int argc, char * argv[])
{
  (void) argc; (void) argv; 
  
  std::shared_ptr<SoDa::MultiMBox<SoDaTest::MyMsgPtr>> mbox[5];

  int i;
  for(i = 0; i < 5; i++) mbox[i] = std::make_shared<SoDa::MultiMBox<SoDaTest::MyMsgPtr>>();

  SoDaTest::MultiMBox_Test_Thread a(std::string("A"));
  SoDaTest::MultiMBox_Test_Thread b(std::string("B"));
  SoDaTest::MultiMBox_Test_Thread c(std::string("C"));
  SoDaTest::MultiMBox_Test_Thread d(std::string("D"));
  SoDaTest::MultiMBox_Test_Thread e(std::string("E"));

  SoDaTest::MultiMBox_Test_Thread * v[5];
  v[0] = &a; v[1] = &b; v[2] = &c; v[3] = &d; v[4] = &e;

  int j; 
  for(i = 0; i < 5; i++) {
    for(j = i; j < 5; j++) {
      v[j]->subscribe(mbox[i]);
    }
  }

  for(i = 0; i < 5; i++) v[i]->start();

  mbox[0]->put(makeMsg("first_toall", SoDaTest::MyMsg::START));
  while(1) {
    usleep(10000);
  }
}

  
