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

set(SoDaRadio_FOUND TRUE)

include(FindPkgConfig)

set(SoDaRadio_INCLUDE_HINTS)
set(SoDaRadio_LIB_HINTS)

if(SoDaRadio_DIR)
  list(APPEND UHD_INCLUDE_HINTS ${SoDaRadio_DIR}/include)
  list(APPEND UHD_LIB_HINTS ${SoDaRadio_DIR}/lib)  
endif()


find_path(SoDaRadio_INCLUDE_DIRS
  NAMES SoDaRadio/UDSockets.hxx
  HINTS ${SoDaRadio_INCLUDE_HINTS}
  PATHS /usr/local/include
        /usr/include
)

find_library(SoDaRadio_LIBRARIES
  NAMES SoDaSockets
  HINTS ${SoDaRadio_LIB_HINTS}
  PATHS /usr/local/lib
       /usr/lib
)

if(SoDaRadio_INCLUDE_DIRS AND SoDaRadio_LIBRARIES)
  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(SoDaRadio DEFAULT_MSG SoDaRadio_LIBRARIES SoDaRadio_INCLUDE_DIRS)
  mark_as_advanced(SoDaRadio_LIBRARIES SoDaRadio_INCLUDE_DIRS)
elseif(SoDaRadio_FIND_REQUIRED)
  message(FATAL_ERROR "SoDaRadio lib is required, but not found.")
endif()
