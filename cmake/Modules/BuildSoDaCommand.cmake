message("In build sodaformat cmake")

# SoDaCommand is a library for simple parsing of command line options.
#
# If the library isn't installed, we'll make a local version.
# This trick doesn't work for MacOS, as the system relies on dynamic libraries
# and so it gets tripped up if the dynamic library doesn't get installed.
#

include(ExternalProject)

IF(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
  # If the package isn't installed, we quit. 
  FIND_PACKAGE(SoDaCommand REQUIRED)
ELSE()
  # If we find the package, yippeeee!
  FIND_PACKAGE(SoDaCommand QUIET)
  IF(NOT SoDaCommand_FOUND)
    # The SoDa::Command package hasn't been installed.
    # Get it and build it as an external package.
    # get sodacommand
    ExternalProject_Add(
      SoDaCommandLib
      #  PREFIX ${PROJECT_BINARY_DIR}/sodacommand-kit
      GIT_REPOSITORY https://github.com/kb1vc/SoDaCommand.git
      GIT_TAG main
      SOURCE_DIR sodacommandlib
      CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
      )

    ExternalProject_Get_Property(SoDaCommandLib INSTALL_DIR)
    set(SoDaCommand_ROOT ${INSTALL_DIR})
    set(SoDaCommand_INCLUDE_DIR ${SoDaCommand_ROOT}/include)
    set(SoDaCommand_LIBRARIES ${SoDaCommand_ROOT}/lib/libsodacommand.a)
    set_property(TARGET SoDaCommandLib PROPERTY IMPORTED_LOCATION ${SoDaCommand_LIB_DIR}/libsodacommand)
  ENDIF()
ENDIF()

  
