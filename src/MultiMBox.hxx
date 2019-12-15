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

#ifndef MULTI_MBOX_HDR
#define MULTI_MBOX_HDR

#include <queue>
#include <vector>
#include <map>
#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <mutex>
#include <condition_variable>

namespace SoDa {
  
  // base class for all messaging schemes. 
  class MBoxMessage {
  public:
    MBoxMessage()
    {
      reader_count = 0;
    }

    void setReaderCount(unsigned int rc)
    {
      std::lock_guard<std::mutex> lck(rc_mutex);
      reader_count = rc; 
    }

    bool readyToDie() { return reader_count == 1; }
    bool decReaderCount() { return reader_count--; }

    bool free(std::queue<MBoxMessage *> & free_list) {
      std::lock_guard<std::mutex> lck(rc_mutex);
      if(reader_count == 1) {
	reader_count = 0; 
	free_list.push(this); 
      }
      else {
	reader_count--;
      }
      return true;
    }

    // these are to detect "free" actions on the wrong mailbox. 
    void setMBoxTag(void * tag) { mbox_tag = tag; }
    bool checkMBoxTag(void * tag) { return mbox_tag == tag; }
    
  private:
    unsigned int reader_count;
    std::mutex rc_mutex;
    void * mbox_tag; 
  };

  /**
   * A base mailbox class.  
   */
  class BaseMBox {
  public:
    BaseMBox() {}

    virtual ~BaseMBox() { }
  };
  
  template <typename T> class MultiMBox : public BaseMBox {
  public:
    MultiMBox(bool _keep_freelist = true) {
      subscriber_count = 0;
      keep_freelist = _keep_freelist; 
    }

    int subscribe() {
      int subscriber_id = subscriber_count;
      subscriber_count++;
      subscribers[subscriber_id] = new Subscriber; 
      return subscriber_id; 
    }

    int getSubscriberCount() { return subscriber_count; }
    
    void put(T * m) {
      unsigned int i;
      m->setReaderCount(subscriber_count);
      m->setMBoxTag(this); 
      for(i = 0; i < subscriber_count; i++) {
	Subscriber * s = subscribers[i];
	std::lock_guard<std::mutex> lck(s->post_mutex);
	s->posted_list.push(m);
	s->post_count++; 
	s->post_cond.notify_all(); 
      }
    }

    T * get(unsigned int subscriber_id) {
      return getCommon(subscriber_id, false); 
    }

    T * getWait(unsigned int subscriber_id) {
      return getCommon(subscriber_id, true); 
    }

    void free(T * m) {
      if(m != NULL) {
	if(keep_freelist && m->checkMBoxTag(this)) {
	  if(m->readyToDie()) {
	    std::lock_guard<std::mutex> lck(free_mutex);
	    m->free(free_list);
	  }
	  else m->decReaderCount(); 
	}
	else {
	  if(m->readyToDie()) delete m;
	  else m->decReaderCount(); 
	}
      }
    }

    T * alloc()
    {
      T * ret = NULL;
      std::lock_guard<std::mutex> lck(free_mutex);
      if(!free_list.empty() && keep_freelist) {
	ret = (T*) free_list.front();
	free_list.pop();
      }

      return ret; 
    }

    void addToPool(T * v) {
      if(keep_freelist) {
	std::lock_guard<std::mutex> lck(free_mutex);	
	free_list.push(v);
      }
    }

    unsigned int inFlightCount() {
      // go through the subscribers and find the
      // longest inflight list.
      int max_len = 0;
      int itercount = 0; 
      typename std::map<int, typename MultiMBox< T >::Subscriber * >::iterator sl; 
      for(sl = subscribers.begin();
	  sl != subscribers.end();
	  ++sl) {
	Subscriber * s = sl->second;
	{
	  std::lock_guard<std::mutex> lck(s->post_mutex);
	  int le = s->posted_list.size();
	  itercount++; 
	  // std::cerr << "multimbox list len = " << le
	  // 	    << " iter count = " << itercount
	  // 	    << " posted count = " << s->post_count << std::endl;
	  if(le > max_len) max_len = le; 
	}
      }

      return max_len; 
    }

    /**
     * @brief flush all items from this mailbox for this subscriber
     *
     * @param subscriber_id oddly named, this is the identity of the requesting subscriber.
     */
    bool flush(unsigned int subscriber_id) {
      T * dummy; 
      while((dummy = getCommon(subscriber_id, false)) != NULL) { 
	free(dummy);
      }
      return true; 
    }
      
    
  private:
    T * getCommon(unsigned int subscriber_id, bool wait)
    {
      if(subscriber_id >= subscriber_count) {
	// someone is asking for a message from
	// an unallocated subscriber id... throw
	// an exception.
	// or just return empty. 
	return NULL; 
      }
    
      Subscriber * s = subscribers[subscriber_id];
      std::unique_lock<std::mutex> lck(s->post_mutex);	  
      if(s->posted_list.empty()) {
	if(!wait) {
	  return NULL; 
	}
	else {
	  while(s->posted_list.empty()) {
	    s->post_cond.wait(lck);
	  }
	}
      }

      // if we get here, then we have a message.
      T * ret = NULL;
      ret = s->posted_list.front();
      s->posted_list.pop();
      s->post_count--; 
      return ret; 
    }

    unsigned int subscriber_count;
    bool keep_freelist; 

    class Subscriber {
    public:
      Subscriber() { post_count = 0; } 
      std::queue<T *> posted_list;
      int post_count; 
      std::mutex post_mutex; 
      std::condition_variable post_cond; 
    };
    std::map<int, Subscriber *> subscribers;     

    std::queue<MBoxMessage *> free_list;
    std::mutex free_mutex; 
  }; 
}

#endif
