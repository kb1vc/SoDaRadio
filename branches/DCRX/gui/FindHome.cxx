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

#include "FindHome.hxx"
extern "C" {
#include <libgen.h>
#include <unistd.h>
#ifdef __linux__
#include <linux/limits.h>
#endif  
}

#include <iostream>
#include <stdexcept>

/**
 * Find the directory in which the calling program resides.
 *
 * Note this feature relies on the existance of the procfs.
 * It works under Linux. I'm not sure what I'll do for other
 * operating systems.  
 *
 * @return string pointing to the program's directory. 
 */
std::string findHome()
{
#ifdef __linux__
  // This solution was suggested by an answer in
  // http://stackoverflow.com/questions/7051844/how-to-find-the-full-path-of-the-c-linux-program-from-within
  char execution_path[PATH_MAX + 1] = {0}; 
  ssize_t st = readlink("/proc/self/exe", execution_path, PATH_MAX);
  if(st < 0) {
    // readlink got an error.... throw something
    throw std::runtime_error("Couldn't open /proc/self/exe");
  }
  // now trim the end of the path off, we just want the directory
  char * mydir = dirname(execution_path);
  return std::string(mydir); 
#elif __OSX__
  // This code has not been tested. 
#endif
}

