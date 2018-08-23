#ifndef BUFFER_POOL_HDR
#define BUFFER_POOL_HDR


/*
  Copyright (c) 2018, Matthew H. Reilly (kb1vc)
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

#include <boost/thread/mutex.hpp>
#include "SoDaBase.hxx"

namespace SoDa {
  /**
   * @brief BufferPool class
   *
   * Provides allocation, free, and queue maintenance for 
   * a buffer pool. 
   */
  template <typename T> class BufferPool {
  public:
    /**
     * @brief BufferPool create a pool of buffers
     * 
     * @param _buffer_size number of elements in each allocated buffer. 
     * @param initial_allocation number of buffers to "seed" the pool
     */
    BufferPool(unsigned int _buffer_size, unsigned int initial_allocation = 5) {
      buffer_size = _buffer_size; 
      for(int i = 0; i < initial_allocation; i++) {
	addNewBuffer();
      }
    }

    /**
     * @brief Allocate a buffer from the pool. 
     *
     * @return a pointer to a buffer
     */
    T * getBuffer() {
      T * ret; 
      boost::mutex::scoped_lock lock(pool_lock);
      if(pool.empty()) {
	addNewBuffer(); 
	addNewBuffer();
      }
      
      ret = pool.front();
      pool.pop(); 

      return ret; 
    }

    /**
     * @brief Return a buffer to the free pool. 
     * @param b pointer to a buffer to be added to the pool.
     */
    void freeBuffer(T * b) {
      boost::mutex::scoped_lock lock(pool_lock);
      pool.push(b); 
    }
  private:
    /**
     * @brief create a new buffer and add it to the free list. 
     * 
     * Note that this is not protected by a mutex. If necessary, its
     * caller must ensure mutual exclusion. 
     */
    void addNewBuffer() {
      T * b = new T[buffer_size];       
      pool.push(b); 
    }

    std::queue<T*> pool; ///< the buffer pool
    boost::mutex pool_lock; ///< lock for the free_buffers pool    
    unsigned int buffer_size; ///< the size of each block in the buffer pool
  }; 
}


#endif
