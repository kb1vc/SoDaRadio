/*
  Copyright (c) 2012,2023 Matthew H. Reilly (kb1vc)
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
#include <SoDa/Format.hxx>
#include <iostream>
#include <string>

std::mutex iomutex; 

namespace SoDaTest {
  
  class MyMsg {
  public:
    enum CMD { START, BCAST, KILL, STOP };
    MyMsg(const std::string ss, CMD _c, int dest) :
      s(ss), cmd(_c), dest_idx(dest)
    {
      create_count++; 
    }

    MyMsg() {
      s = "Dunno"; 
      create_count++;
    }

    ~MyMsg() {
      destroy_count++; 
    }

    static std::shared_ptr<MyMsg> make(const std::string ss, CMD _c, int idx) {
      return std::make_shared<MyMsg>(ss, _c, idx);
    }

    std::string s;
    CMD cmd;
    int dest_idx;
    static int create_count;
    static int destroy_count; 
  };

  int MyMsg::create_count = 0;
  int MyMsg::destroy_count = 0;
  
  typedef std::shared_ptr<MyMsg> MyMsgPtr;
  typedef std::shared_ptr<SoDa::MultiMBox<MyMsg>> MyMsgMBoxPtr; 
  
  class MultiMBox_Test_Thread: public SoDa::Thread {
  public:
    MultiMBox_Test_Thread(const std::string & n,
			  int my_idx, 
			  int next_mbox_idx) :
      SoDa::Thread(n), my_idx(my_idx), next_mbox_idx(next_mbox_idx) {
      name = n;
      all_done = false; 
    }

    ~MultiMBox_Test_Thread() {
      mbox->unsubscribe(this);
    }
    
    void execute(MyMsgPtr msg) {
      if(msg->dest_idx != my_idx) return;
      
      switch (msg->cmd) {
      case MyMsg::START:
	if(next_mbox_idx != 0) {
	  mbox->put(MyMsg::make("from" + name, MyMsg::START, next_mbox_idx));
	}
	if(next_mbox_idx == 0) {
	  mbox->put(MyMsg::make("from" + name, MyMsg::KILL, next_mbox_idx));
	}
	break;
      case MyMsg::BCAST:
	break;
      case MyMsg::KILL:
	if(next_mbox_idx != 0) {
	  mbox->put(MyMsg::make("from" + name, MyMsg::KILL, next_mbox_idx));
	}
	all_done = true; 	
	return; 
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
      while(!all_done) {
	bool flag = false; 
	// check to see if there is a message on the mailbox.
	if((msg = mbox->get(this)) != nullptr) {
	  execute(msg);
	  auto nm = msg->s; 
	  auto cmd = msg->cmd;
	  msg = nullptr;
	  
	  flag = true; 
	}

	if(!flag) usleep(10000);
      }
      
      {
	std::lock_guard<std::mutex> lock(iomutex); 
	std::cerr << "Thread " << name << " leaving runloop\n";
      }

    }
  

    void subscribe(MyMsgMBoxPtr mbx) {
      mbx->subscribe(this);
      mbox = mbx; 
    }
    
    std::string name;
    unsigned int my_idx, next_mbox_idx; 
    bool all_done;
    MyMsgMBoxPtr mbox; 
  }; 
}

int main(int argc, char * argv[])
{
  (void) argc; (void) argv; 
  
  auto mbox =  SoDa::MultiMBox<SoDaTest::MyMsg>::make("Test Mailbox");

  std::list<SoDaTest::MultiMBox_Test_Thread *> threads;
  threads.push_back(new SoDaTest::MultiMBox_Test_Thread (std::string("A"), 0, 1));
  threads.push_back(new SoDaTest::MultiMBox_Test_Thread (std::string("B"), 1, 2));
  threads.push_back(new SoDaTest::MultiMBox_Test_Thread (std::string("C"), 2, 3));
  threads.push_back(new SoDaTest::MultiMBox_Test_Thread (std::string("D"), 3, 4));
  threads.push_back(new SoDaTest::MultiMBox_Test_Thread (std::string("E"), 4, 0));

  for(auto th : threads) {
    th->subscribe(mbox);
  }

  for(auto th : threads) {
    {
      std::lock_guard<std::mutex> lock(iomutex);      
      std::cerr << SoDa::Format("starting thread %0\n")
	.addS(th->getObjName());
    }
    th->start();
  }

  mbox->put(SoDaTest::MyMsg::make("first", SoDaTest::MyMsg::START, 0));
  
  for(auto th : threads) {
    th->join();
    std::cerr << "Joined " << th->getObjName() << "\n";
    delete(th);
  }

  std::cerr << "joined all\n";
  
  if(SoDaTest::MyMsg::create_count == SoDaTest::MyMsg::destroy_count) {
    std::cerr << SoDa::Format("MultiMBox_Test PASSED create_count = %0 destry_count = %1\n")
    .addI(SoDaTest::MyMsg::create_count).addI(SoDaTest::MyMsg::destroy_count);      

  }
  else {
    std::cerr << SoDa::Format("MultiMBox_Test FAILED create_count = %0 destry_count = %1\n")
    .addI(SoDaTest::MyMsg::create_count).addI(SoDaTest::MyMsg::destroy_count);      
    
  }
}

  
