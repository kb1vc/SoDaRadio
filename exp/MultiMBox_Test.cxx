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
	boost::mutex::scoped_lock lock(iomutex);
	std::cerr << "Creating message [" << s << "," << cmd << "]" << std::endl;
      }
    }

    MyMsg() {
      s = "Dunno"; 
    }

    ~MyMsg() {
      boost::mutex::scoped_lock lock(iomutex);
      std::cerr << "Destroying message [" << s << "," << cmd << "]" << std::endl;
    }
    std::string s;
    CMD cmd; 
  };

  class MultiMBox_Test_Thread: public SoDa::SoDaThread {
  public:
    MultiMBox_Test_Thread(const std::string & n) : SoDaThread(n) {
      name = n;
      sub_count = 0; 
    }

    void subscribe(SoDa::MultiMBox<MyMsg> * mbox) {
      if(sub_count < 10) {
	subscriptions[sub_count] = mbox->subscribe();
	mboxes[sub_count] = mbox;
	sub_count++; 
      }
    }

    void execute(MyMsg * msg, int i) {
      {
	boost::mutex::scoped_lock lock(iomutex);
	std::cerr << "Process " << name << "got message " << msg->cmd << " " << msg->s << " subcount = " << sub_count << " i = " << i << std::endl;
      }
      switch (msg->cmd) {
      case MyMsg::START:
	if((i + 1) < sub_count) {
	  mboxes[i+1]->put(new MyMsg("from" + name, MyMsg::KILL));
	  name = name + ".";
	}
	break;
      case MyMsg::BCAST:
	break;
      case MyMsg::KILL:
	if((i + 1) < sub_count) {
	  mboxes[i+1]->put(new MyMsg("from" + name, MyMsg::KILL));
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
      MyMsg * msg;
      while(1) {
	bool flag = false; 
	for(i = 0; i < sub_count; i++) {
	  // check to see if there is a message on the mailbox.
	  if((msg = (mboxes[i]->get(subscriptions[i])))) {
	    execute(msg, i);
	    mboxes[i]->free(msg);
	    flag = true; 
	  }
	}

	if(!flag) usleep(10000);
      }
    }
  
  
    std::string name;

    int subscriptions[10];
    SoDa::MultiMBox<MyMsg> * mboxes[10];
    int sub_count;
  }; 
}

int main(int argc, char * argv[])
{
  (void) argc; (void) argv; 
  
  SoDa::MultiMBox<SoDaTest::MyMsg> * mbox[5];

  int i;
  for(i = 0; i < 5; i++) mbox[i] = new SoDa::MultiMBox<SoDaTest::MyMsg>(false); 

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

  mbox[0]->put(new SoDaTest::MyMsg("first_toall", SoDaTest::MyMsg::START));
  while(1) {
    usleep(10000);
  }
}

  
