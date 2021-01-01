#
#  Copyright (c) 2019 Matthew H. Reilly (kb1vc)
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#  Redistributions of source code must retain the above copyright
#  notice, this list of conditions and the following disclaimer.
#  Redistributions in binary form must reproduce the above copyright
#  notice, this list of conditions and the following disclaimer in
#  the documentation and/or other materials provided with the
#  distribution.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

set(SoDaFormat_FOUND TRUE)

include(FindPkgConfig)

set(SoDaFormat_INCLUDE_HINTS)
set(SoDaFormat_LIB_HINTS)



find_path(SoDaFormat_INCLUDE_DIRS
  NAMES SoDaFormat/Format.hxx
  HINTS ${SoDaFormat_INCLUDE_HINTS}
  PATHS /usr/local/include
        /usr/include
	/opt/local/include
)

find_library(SoDaFormat_LIBRARIES
  NAMES sodaformat
  HINTS ${SoDaFormat_LIB_HINTS}
  PATHS /usr/local/lib
       /usr/lib
       /usr/local/lib64
       /usr/lib64
       /opt/local/lib
)

if(SoDaFormat_INCLUDE_DIRS AND SoDaFormat_LIBRARIES)
  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(SoDaFormat DEFAULT_MSG SoDaFormat_LIBRARIES SoDaFormat_INCLUDE_DIRS)
  mark_as_advanced(SoDaFormat_LIBRARIES SoDaFormat_INCLUDE_DIRS)
elseif(SoDaFormat_FIND_REQUIRED)
  message(FATAL_ERROR "SoDaFormat lib is required, but not found.")
endif()

FUNCTION(SODAFORMAT_BUILD_PLUGIN plugin_name source_file_list)
  FIND_FILE(VERSION_FILE "SoDaFormat/version.h" PATHS ${SoDaFormat_INCLUDE_DIRS})
  LIST(GET source_file_list 0 first_source_file)
  SET_SOURCE_FILES_PROPERTIES(${first_source_file} PROPERTIES OBJECT_DEPENDS ${VERSION_FILE})
  ADD_LIBRARY(${plugin_name} SHARED ${source_file_list})
  TARGET_INCLUDE_DIRECTORIES(${plugin_name} PUBLIC ${SoDaFormat_INCLUDE_DIRS})
ENDFUNCTION()

