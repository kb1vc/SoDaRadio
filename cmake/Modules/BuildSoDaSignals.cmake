message("In build sodasignals cmake")

# SoDaSignals is a library containing:
#   SoDa::Format intelligent formatting of text from
#                C++.  It pays special attention to formatting 
#                floating point values,
#                offering an option to print in "engineering notation"
#                where the exponent is always a multiple of 3.
#
# If the library isn't installed, we'll make a local version.
# This trick doesn't work for MacOS, as the system relies on dynamic libraries
# and so it gets tripped up if the dynamic library doesn't get installed.
#

include(ExternalProject)

IF(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
  # If the package isn't installed, we quit. 
  FIND_PACKAGE(SoDaSignals REQUIRED)
ELSE()
  # If we find the package, yippeeee!
  FIND_PACKAGE(SoDaSignals QUIET)
  message("SoDaSignals_FOUND = ${SoDaSignals_FOUND}")
  IF(NOT SoDaSignals_FOUND)
    # The SoDa::Signals package hasn't been installed.
    # Get it and build it as an external package.
    # get sodaformat
    ExternalProject_Add(
      SoDaSignalsLib
      #  PREFIX ${PROJECT_BINARY_DIR}/sodaformat-kit
      GIT_REPOSITORY https://github.com/kb1vc/SoDaSignals.git
      GIT_TAG main
      SOURCE_DIR sodasignalslib
      CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
      )

    ExternalProject_Get_Property(SoDaSignalsLib INSTALL_DIR)
    message("About to set SoDaSignals stuff... to ${INSTALL_DIR}")
    set(SoDaSignals_ROOT ${INSTALL_DIR})
    set(SoDaSignals_INCLUDE_DIR ${SoDaSignals_ROOT}/include)
    set(SoDaSignals_LIBRARIES ${SoDaSignals_ROOT}/lib64/libsodasignals.a)
    set_property(TARGET SoDaSignalsLib PROPERTY IMPORTED_LOCATION ${SoDaSignals_LIB_DIR}/libsodasignals)
  ENDIF()
ENDIF()

  
