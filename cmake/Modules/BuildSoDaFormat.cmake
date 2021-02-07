message("In build sodaformat cmake")

# SoDaFormat is a library for intelligent formatting of text from
# C++.  It pays special attention to formatting floating point values,
# offering an option to print in "engineering notation" where the
# exponent is always a multiple of 3.
#
# If the library isn't installed, we'll make a local version.
# This trick doesn't work for MacOS, as the system relies on dynamic libraries
# and so it gets tripped up if the dynamic library doesn't get installed.
#

include(ExternalProject)

IF(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
  # If the package isn't installed, we quit. 
  FIND_PACKAGE(SoDaFormat REQUIRED)
ELSE()
  # If we find the package, yippeeee!
  FIND_PACKAGE(SoDaFormat QUIET)
  IF(NOT SoDaFormat_FOUND)
    # The SoDa::Format package hasn't been installed.
    # Get it and build it as an external package.
    # get sodaformat
    ExternalProject_Add(
      SoDaFormatLib
      #  PREFIX ${PROJECT_BINARY_DIR}/sodaformat-kit
      GIT_REPOSITORY https://github.com/kb1vc/SoDaFormat.git
      GIT_TAG main
      SOURCE_DIR sodaformatlib
      CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
      )

    ExternalProject_Get_Property(SoDaFormatLib INSTALL_DIR)
    set(SoDaFormat_ROOT ${INSTALL_DIR})
    set(SoDaFormat_INCLUDE_DIR ${SoDaFormat_ROOT}/include)
    set(SoDaFormat_LIBRARIES ${SoDaFormat_ROOT}/lib/libsodaformat.a)
    set_property(TARGET SoDaFormatLib PROPERTY IMPORTED_LOCATION ${SoDaFormat_LIB_DIR}/libsodaformat)
  ENDIF()
ENDIF()

  
