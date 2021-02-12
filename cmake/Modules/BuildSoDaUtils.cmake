# SoDaUtils is a set of useful functions and classes that would otherwise require
# larger libraries or collections (like boost or Qt) to get simular (but generally
# superior) capabilities. 
#
# If the library isn't installed, we'll make a local version.
# This trick doesn't work for MacOS, as the system relies on dynamic libraries
# and so it gets tripped up if the dynamic library doesn't get installed.
#

include(ExternalProject)

IF(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
  # If the package isn't installed, we quit. 
  FIND_PACKAGE(SoDaUtils REQUIRED)
ELSE()
  # If we find the package, yippeeee!
  FIND_PACKAGE(SoDaUtils QUIET)
  IF(NOT SoDaUtils_FOUND)
    # The SoDa::Utils package hasn't been installed.
    # Get it and build it as an external package.
    # get sodautils
    ExternalProject_Add(
      SoDaUtilsLib
      #  PREFIX ${PROJECT_BINARY_DIR}/sodautils-kit
      GIT_REPOSITORY https://github.com/kb1vc/SoDaUtils.git
      GIT_TAG main
      SOURCE_DIR sodautilslib
      CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
      )

    ExternalProject_Get_Property(SoDaUtilsLib INSTALL_DIR)
    set(SoDaUtils_ROOT ${INSTALL_DIR})
    set(SoDaUtils_INCLUDE_DIR ${SoDaUtils_ROOT}/include)
    set(SoDaUtils_LIBRARIES ${SoDaUtils_ROOT}/lib/libsodautils.a)
    set_property(TARGET SoDaUtilsLib PROPERTY IMPORTED_LOCATION ${SoDaUtils_LIB_DIR}/libsodautils)
  ENDIF()
ENDIF()

  
