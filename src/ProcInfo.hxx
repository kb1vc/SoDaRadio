/*
  Copyright (c) 2017, Matthew H. Reilly (kb1vc)
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

#ifndef ProcInfo_HDR
#define ProcInfo_HDR

#include <fstream>
#include <string>
#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace kb1vc {
  /** 
   * A simple base class to provide debug messaging from any derived class. 
   */
  class ProcInfo {
  public:
    ProcInfo(const std::string &  outfile,
	     const std::string & _unit_name = std::string("UNKNOWN")) {
      unit_name = _unit_name;
      statm_file_name = "/proc/self/statm";
      init(outfile);
    }

    ProcInfo(const std::string &  outfile,
	     unsigned int pid, 
	     const std::string & _unit_name = std::string("UNKNOWN")) {
      unit_name = _unit_name;
      statm_file_name = (boost::format("/proc/%d/statm") % pid).str();
      init(outfile);
    }
    
    bool getInfo();

    void reportInfo(bool only_if_changed = false) {
      getInfo();
      if(!only_if_changed || stats_changed) {      
	printInfo(); 
      }
    }

    void printInfo(std::ostream & out);

    void printInfo() { if(report_file_ok) printInfo(f_report); }


  protected:
    // the stats we gather
    unsigned long vm_size, resident_pages, 
      shared_pages, text_pages, 
      lib_pages, data_stack_pages, dirty_pages; 
    // true if things changed
    bool stats_changed; 

    void init(const std::string & outfile) {
      start_seconds = boost::posix_time::second_clock::local_time();      
      openFiles(outfile);
      clearStats();
      getInfo();
    }

    void openFiles(const std::string & of_name);

    void clearStats() {
      vm_size = 0; 
      resident_pages = 0; 
      shared_pages = 0; 
      text_pages = 0; 
      lib_pages = 0; 
      data_stack_pages = 0; 
      dirty_pages = 0;
    };
    
    std::string curDateTime();

    unsigned long getElapsedTime() {
      boost::posix_time::ptime now; 
      now = boost::posix_time::second_clock::local_time();
      boost::posix_time::time_duration duration = now - start_seconds; 
      return duration.total_seconds();
    }

    bool report_file_ok; 
    bool stat_file_ok; 

    std::string unit_name; ///< the name of the unit reporting status
    std::string statm_file_name; 
    std::ifstream f_statm; 
    std::ofstream f_report; 

    boost::posix_time::ptime start_seconds; 
    unsigned long last_elapsed; 
  };
}


#endif
