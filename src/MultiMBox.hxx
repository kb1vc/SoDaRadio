/*
  Copyright (c) 2012, 2023 Matthew H. Reilly (kb1vc)
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

#pragma once

#include <queue>
#include <vector>
#include <map>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include "SoDaBase.hxx"

namespace SoDa
{
  // base class for all messaging schemes.
  // This once had a self-managed buffer pool.  Not any more.
  // buffers (messages) are now smart pointers. 

  /**
   * A base mailbox class.  
   */
  class BaseMBox
  {
  public:
    BaseMBox(const std::string & name) : name(name) {}

    virtual ~BaseMBox() {}
    
    std::string getName() { return name; }
    std::string name;
    
  };

  template <typename T>
  class Subscription 
  {
  public:
    Subscription() {}
    
    void flush() {
      std::lock_guard<std::mutex> lck(post_mutex);      
      while(!posted_list.empty()) {
	auto ptr = posted_list.front();
	posted_list.pop();
	ptr == nullptr;	
      }
    }
    std::queue<std::shared_ptr<T>> posted_list;

    std::mutex post_mutex;
    std::condition_variable post_cond;
  };

  template <typename T>
  class MultiMBox : public BaseMBox
  {
  public:
    MultiMBox(const std::string &name = "NONE")
      : BaseMBox(name)
    {
      last_subscriber_idx = 0;
      put_count = 0; 
    }

    ~MultiMBox() { 
      for(auto & sub_e : subscriptions) {
	unsubscribe(sub_e.first);
      }
    }

    void subscribe(SoDa::Base * subscriber_id)
    {
      subscriptions[subscriber_id] = std::make_shared<Subscription<T>>();
    }

    int getSubscriptionCount() { return subscriptions.size(); }
    
    void put(std::shared_ptr<T> m)
    {
      unsigned int i;
      std::lock_guard<std::mutex> lcks(subscription_mutex);
      put_count++; 
      for(auto & sub_e : subscriptions) {
	auto s = sub_e.second; 
	std::lock_guard<std::mutex> lck(s->post_mutex);
	s->posted_list.push(m);
	s->post_cond.notify_all();
      }
    }

    std::shared_ptr<T> get(SoDa::Base * subscriber_id)
    {
      return getCommon(subscriber_id, false);
    }

    std::shared_ptr<T> getWait(SoDa::Base * subscriber_id)
    {
      return getCommon(subscriber_id, true);
    }


    unsigned int inFlightCount()
    {
      // go through the subscriptions and find the
      // longest inflight list.
      int max_len = 0;
      int itercount = 0;
      std::lock_guard<std::mutex> lcks(subscription_mutex);
      for (auto &sl : subscriptions)
	{
	  auto s = sl.second;
	  {
	    std::lock_guard<std::mutex> lck(s->post_mutex);
	    int le = s->posted_list.size();
	    itercount++;
	    if (le > max_len)
	      max_len = le;
	  }
	}

      return max_len;
    }

    void unsubscribe(SoDa::Base * subscriber_id) {
      std::lock_guard<std::mutex> lcks(subscription_mutex);      
      if(subscriptions.count(subscriber_id) != 0) {      
	auto sub = subscriptions[subscriber_id];
	sub->flush();
	subscriptions.erase(subscriber_id);
      }

    }

    /**
     * @brief flush all items from this mailbox for this subscriber
     *
     * @param subscriber_id this is the identity of the requesting subscriber.
     */
    bool flush(SoDa::Base * subscriber_id)
    {
      std::lock_guard<std::mutex> lcks(subscription_mutex);      
      if (subscriptions.count(subscriber_id) == 0) return false; 
      
      auto sub = subscriptions[subscriber_id]; 
      sub->flush();
      return true;
    }

    static std::shared_ptr<MultiMBox<T>> make(const std::string & name) {
      return std::make_shared<MultiMBox<T>>(name);
    }

    void dumpStatus(std::ostream & os) {
      for(auto s : subscriptions) {
	os << name << " subscription:[" << s.first << "].size " << s.second->posted_list.size() << "\n";
      }
    }

  private:
    std::shared_ptr<T> getCommon(SoDa::Base * subscriber_id, bool wait)
    {
      std::lock_guard<std::mutex> lcks(subscription_mutex);      
      if (subscriptions.count(subscriber_id) == 0) return nullptr;

      auto s = subscriptions[subscriber_id];
      std::unique_lock<std::mutex> lck(s->post_mutex);
      if (s->posted_list.empty())
	{
	  if (!wait)
	    {
	      return nullptr; 
	    }
	  else
	    {
	      while (s->posted_list.empty())
		{
		  s->post_cond.wait(lck);
		}
	    }
	}

      // if we get here, then we have a message.
      auto ret = s->posted_list.front();
      s->posted_list.pop();
      return ret;
    }

    unsigned int last_subscriber_idx;
    bool keep_freelist;
    
    unsigned int put_count; 

    std::map<SoDa::Base *, std::shared_ptr<Subscription<T>>> subscriptions;
    std::mutex subscription_mutex;

    std::mutex free_mutex;
  };
} // namespace SoDa
