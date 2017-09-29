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

#include "ProcInfo.hxx"

void kb1vc::ProcInfo::openFiles(const std::string & of_name)
{
  f_statm.open(statm_file_name.c_str());
  if(f_statm.bad()) {
    std::cerr << boost::format("ProcInfo could not open input file %s for object %s\n")
      % statm_file_name % unit_name; 
    stat_file_ok = false; 
  }
  else {
    stat_file_ok = true; 
  }

  f_report.open(of_name.c_str()); 
  if(f_report.bad()) {
    std::cerr << boost::format("ProcInfo could not open output report file %s for object %s\n")
      % of_name % unit_name; 
    report_file_ok = false; 
  }
  else {
    report_file_ok = true; 
  }
}

/**
 * @brief retrieve current memory usage info from statm
 * 
 * @return true if any of the mem stats changed. 
 */
bool kb1vc::ProcInfo::getInfo()
{
  if(stat_file_ok) {
    f_statm.seekg(0);

    unsigned long n_vm_size, n_resident_pages, 
      n_shared_pages, n_text_pages, 
      n_lib_pages, n_data_stack_pages, n_dirty_pages; 
      
    f_statm >> n_vm_size 
	    >> n_resident_pages 
	    >> n_shared_pages 
	    >> n_text_pages 
	    >> n_lib_pages 
	    >> n_data_stack_pages 
	    >> n_dirty_pages; 

    stats_changed = false; 
    if(n_vm_size != vm_size) {
      stats_changed = true;
      vm_size = n_vm_size;
    }
    if(n_resident_pages != resident_pages) {
      stats_changed = true;
      resident_pages = n_resident_pages;
    }
    if(n_shared_pages != shared_pages) {
      stats_changed = true;
      shared_pages = n_shared_pages;
    }
    if(n_text_pages != text_pages) {
      stats_changed = true;
      text_pages = n_text_pages;
    }
    if(n_lib_pages != lib_pages) {
      stats_changed = true;
      lib_pages = n_lib_pages;
    }
    if(n_data_stack_pages != data_stack_pages) {
      stats_changed = true;
      data_stack_pages = n_data_stack_pages;
    }
    if(n_dirty_pages != dirty_pages) {
      stats_changed = true;
      dirty_pages = n_dirty_pages;
    }
	       
    if(stats_changed) last_elapsed = getElapsedTime();
  }

  return stats_changed;
}


void kb1vc::ProcInfo::printInfo(std::ostream & out)
{
  out << boost::format("%s %s %ld vm_size %d rss %d sh %d txt %d lib %d dss %d dirty %d\n")
    % curDateTime() % unit_name % last_elapsed % vm_size % resident_pages % shared_pages 
    % text_pages % lib_pages % data_stack_pages % dirty_pages; 
  out.flush();
}


std::string kb1vc::ProcInfo::curDateTime() 
{ 
  boost::posix_time::ptime t1; 
  t1 = boost::posix_time::second_clock::local_time();
  return to_simple_string(t1);
}
