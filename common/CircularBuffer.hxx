#ifndef SODA_CIRCULAR_BUFFER_HDR
#define SODA_CIRCULAR_BUFFER_HDR

#include <cstring>
#include <iostream> 
#include <mutex>


namespace SoDa {
  
  /**
   * @class CircularBuffer
   *
   * @brief A circular buffer for vectors of samples (as in, 
   * audio samples waiting to be consumed by a callback). 
   *
   */
  template<typename T> class CircularBuffer {
  public:
    CircularBuffer(size_t elements) {
      buffer_elements = elements; 
      buffer = new T[buffer_elements];
      head_pointer = buffer; 
      tail_pointer = buffer; 
      num_written = 0; 
      num_read = 0; 
    }


    /**
     * @brief clear the buffer by reseting the pointers. 
     */
    void clear() {
      std::lock_guard<std::mutex> lck(pointer_mutex);       
      head_pointer = buffer; 
      tail_pointer = buffer; 
      num_written = num_read = 0; 
    }
    
    size_t put(const T * in, size_t len) {
      // several interesting cases: 
      //
      // 1. room left between head pointer and end of the buffer:
      //    copy the entire buffer, bump head pointer by len; 
      // 2. insufficient room between head pointer and end of buffer:
      //    copy the first part (pre_len) up to the end of the buffer
      //    copy the second part to the start of the buffer. 
      // 
      // Then independently, the head pointer could pass the tail
      // pointer.  We know that it has if len + numElements is greater
      // than the buffer size. If the head pointer passes the tail
      // pointer, then we set the tail pointer equal to the head
      // pointer plus one (wrapped). 
      // 
      bool lock_head_tail = (numElements() + len) > buffer_elements;
      size_t nput = 0; 
      while(len > 0) {
	// need to do this in a loop, because we might actually 
	// have a really really large buffer. 
	// we're only going to copy up to the end of the buffer. 
	size_t this_len = min(len, buffer + buffer_elements - head_pointer);

	// move the samples. 
	memcpy(head_pointer, in, sizeof(T) * this_len);
	{ 
	  std::lock_guard<std::mutex> lck(pointer_mutex); 
	  head_pointer = wrapBufferPointer(head_pointer, this_len);
	}

	// we moved some samples, update the input pointer
	in = in + this_len; 

	// how much is left? 
	len = len - this_len; 
	// how much did we write?
	nput = nput + this_len; 
      }

      if(lock_head_tail) {
	std::lock_guard<std::mutex> lck(pointer_mutex); 
	tail_pointer = head_pointer;
      }

      num_written += nput;
      return nput; 
    }

    size_t get(T * out, size_t len) {
      // several interesting cases, just like "put": 
      //
      // 1. room left between tail pointer and end of the buffer:
      //    copy the entire buffer, bump head pointer by len; 
      // 2. insufficient room between tail pointer and end of buffer:
      //    copy the first part (pre_len) up to the end of the buffer
      //    copy the second part to the start of the buffer. 
      // 
      // If the length is greater than the number of elements in 
      // the buffer, we return a shorter buffer. 
      // 

      if (len > numElements()) {
	len = numElements(); 
      }

      size_t pre_len = min(len, buffer + buffer_elements - tail_pointer);
      size_t post_len = len - pre_len; 
      
      if(pre_len > 0) {
	memcpy(out, tail_pointer, sizeof(T) * pre_len);
	{
	  std::lock_guard<std::mutex> lck(pointer_mutex); 
	  tail_pointer = wrapBufferPointer(tail_pointer, pre_len);
	}
      }
      if(post_len > 0) {
	memcpy(out + pre_len, tail_pointer, sizeof(T) * post_len);
	{ 
	  std::lock_guard<std::mutex> lck(pointer_mutex); 
	  tail_pointer = wrapBufferPointer(tail_pointer, post_len);
	}
      }

      num_read += len; 
      return len; 
    }

    size_t put(const std::vector<T> & in) {
      return put(in.data(), in.size());
    }

    size_t get(std::vector<T> & out) {
      return get(out.data(), out.size());      
    }


    size_t numElements() {
      size_t ret = num_written - num_read; 
      if(ret > buffer_elements) {
	// we have over-run the buffer; 
	ret = buffer_elements; 
	// so we need to correct the number_read
	// to "reset" and ignore the over-written elements. 
	num_read = num_written - buffer_elements; 
      }
      return ret; 
    }

    template <typename T2> void dumpBuf(std::ostream & os) {
      T2 * tptr = (T2*) buffer; 
      for(int i = 0; i < (buffer_elements * sizeof(T)) / sizeof(T2); i++) {
	os << i << " " << tptr[i] << std::endl; 
      }
    }
  private:
    size_t min(size_t a, size_t b) {
      return (a < b) ? a : b;
    }

    T * wrapBufferPointer(T * ptr, size_t len) {
      T * ret = ptr + len;
      size_t offset = ret - buffer; 
      if(offset >= buffer_elements) {
	// the pointer has wrapped. 
	ret = buffer + (offset - buffer_elements);
      }

      return ret; 
    }
    
    T * buffer;
    size_t buffer_elements; 
    T * head_pointer; ///< points to the next element to be written
    T * tail_pointer; ///< points to the next element to be read
    size_t num_written, num_read; 

    // locks for tail and head pointer
    std::mutex pointer_mutex; 
  }; 
}
#endif
