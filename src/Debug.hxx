/*
  Copyright (c) 2014, Matthew H. Reilly (kb1vc)
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

#ifndef DEBUG_HDR
#define DEBUG_HDR

#include <string>
#include <mutex>
#include <SoDa/Format.hxx>

namespace SoDa {
  /** 
   * A simple base class to provide debug messaging from any derived class. 
   */
  class Debug {
  public:
    Debug(std::string _unit_name = std::string("UNKNOWN")) {
      unit_name = _unit_name;
      debug_level = default_debug_level;
    }

    Debug(const char * _unit_name_cstr) {
      unit_name = std::string(_unit_name_cstr);
      debug_level = default_debug_level;
    }
    
    Debug(unsigned int _debug_level, std::string _unit_name = std::string("UNKNOWN")) {
      unit_name = _unit_name;
      debug_level = _debug_level;
    }

    Debug(unsigned int _debug_level, const char * _unit_name_cstr) {
      unit_name = std::string(_unit_name_cstr);
      debug_level = _debug_level;
    }

    void debugMsg(const std::string & msg, unsigned int threshold = 1) {
      std::lock_guard<std::mutex> lock(debug_msg_mutex);
      if((debug_level >= threshold) || (global_debug_level >= threshold)) {
	std::cerr << SoDa::Format("%0 %1\t%2\n")
	  .addS(unit_name, 20)
	  .addS(curDateTime())
	  .addS(msg); 
      }
    }

    void debugMsg(const SoDa::Format & fmt, unsigned int threshold = 1) {
      debugMsg(fmt.str(), threshold);
    }
    
    void debugMsg(const char * msg, unsigned int threshold = 1) {
      debugMsg(std::string(msg), threshold); 
    }

    void setDebugLevel(unsigned int v) { debug_level = v; }
    unsigned int  getDebugLevel() { return debug_level; }

    static void setDefaultLevel(unsigned int v) { default_debug_level = v; }
    static unsigned int  getDefaultLevel() { return default_debug_level; }

    static void setGlobalLevel(unsigned int v) { global_debug_level = v; }
    static unsigned int  getGlobalLevel() { return global_debug_level; }

    static std::mutex debug_msg_mutex; 
    
  protected:

    std::string curDateTime(); 

    std::string unit_name; ///< the name of the unit reporting status
    unsigned int debug_level; ///< the debug level (threshold) for messages

    static unsigned int default_debug_level; 
    static unsigned int global_debug_level; 
  };
}


#endif
