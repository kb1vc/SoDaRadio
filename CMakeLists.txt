CMAKE_MINIMUM_REQUIRED(VERSION 2.8.9) # this lets out really old distros like CentOS 6

IF(CMAKE_VERSION VERSION_GREATER 3.0.0)
  CMAKE_POLICY(SET CMP0048 NEW)
ENDIF()

PROJECT(SoDaRadio)

OPTION(BUILD_UNIT_TESTS "Build the unit tests -- not normally useful" ON)

SET(SoDaRadio_VERSION_MAJOR 5)
SET(SoDaRadio_VERSION_MINOR 0)
SET(SoDaRadio_VERSION_PATCH 0)
SET(SoDaRadio_VERSION "${SoDaRadio_VERSION_MAJOR}.${SoDaRadio_VERSION_MINOR}.${SoDaRadio_VERSION_PATCH}")

# Package generation for the source tarball
SET(CPACK_PACKAGE_VERSION_MAJOR ${SoDaRadio_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${SoDaRadio_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${SoDaRadio_VERSION_PATCH})
SET(CPACK_PACKAGE_VERSION ${SoDaRadio_VERSION})
SET(CPACK_PACKAGE_NAME "SoDaRadio")
SET(CPACK_PACKAGE_RELEASE 1)
SET(CPACK_PACKAGE_CONTACT "Matt Reilly kb1vc")
SET(CPACK_PACKAGE_VENDOR "Matt Reilly -- radiogeek381@gmail.com")
SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}.${CMAKE_SYSTEM_VERSION}")
SET(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_GENERATOR "RPM")
set(CPACK_RPM_PACKAGE_RELEASE "1")
set(CPACK_RPM_PACKAGE_LICENSE "BSD-2-Clause")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Communications")
set(CPACK_RPM_PACKAGE_DESCRIPTION 
  "SoDaRadio is a software defined radio implementation of "
  "a general purpose transmitter/receiver using the "
  "Ettus Research USRP hardware.")
set(CPACK_RPM_PACKAGE_REQUIRES "boost >= 1.48, uhd >= 3.10.1.0, fftw-devel >= 3.3, qt5 >= 5.6, qwt-qt5 >= 6.1.2")
set(CPACK_RPM_PACKAGE_RELOCATABLE TRUE)


INCLUDE(CPack)

MESSAGE("SoDaRadio_VERSION=[${SoDaRadio_VERSION}]")


FIND_PACKAGE(Git QUIET)

IF((EXISTS "${CMAKE_SOURCE_DIR}/.git/") AND GIT_FOUND)
  EXECUTE_PROCESS(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  EXECUTE_PROCESS(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  SET(SoDaRadio_GIT_ID "${GIT_BRANCH}:${GIT_COMMIT_HASH}")
  FILE(WRITE ${CMAKE_BINARY_DIR}/saved_git_version.txt "${GIT_BRANCH}\n")
  FILE(APPEND ${CMAKE_BINARY_DIR}/saved_git_version.txt "${GIT_COMMIT_HASH}\n")    
elseIF(EXISTS ${CMAKE_BINARY_DIR}/saved_git_version.txt)
  MESSAGE("Retrieving GIT version info from saved_git_version.txt")
  FILE(STRINGS ${CMAKE_BINARY_DIR}/saved_git_version.txt git_version_strings)
  LIST(GET git_version_strings 0 GIT_BRANCH)
  LIST(GET git_version_strings 1 GIT_COMMIT_HASH)
  SET(SoDaRadio_GIT_ID "${GIT_BRANCH}:${GIT_COMMIT_HASH}")
else()
  SET(SoDaRadio_GIT_ID "UNKNOWN")  
ENDIF()

SET(DOXY_PROJECT_VERSION "${SoDaRadio_VERSION}-${SoDaRadio_GIT_ID}")
SET(DOXY_EXCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/gui/Default.soda_cfg.h ${CMAKE_CURRENT_BINARY_DIR}/SoDaRadio_GUI.cxx")

CONFIGURE_FILE (
"${PROJECT_SOURCE_DIR}/version.h.in"
"${PROJECT_BINARY_DIR}/version.h"
  )
INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}")

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")


FIND_PACKAGE(Boost 1.45 REQUIRED COMPONENTS program_options thread date_time system filesystem serialization regex QUIET)

IF(${NO_UHD})
  MESSAGE("libUHD Support disabled.")
  SET(HAVE_UHD 0)  
ELSE()
  # Enable UHD support
  FIND_PACKAGE(UHD QUIET)
  IF(NOT UHD_FOUND)
    MESSAGE("UHD Library NOT FOUND!")
    SET(HAVE_UHD 0)  
  ELSE()
    SET(HAVE_UHD 1)
  ENDIF()
ENDIF()


# This is the radio interface... 
ADD_SUBDIRECTORY(src)


IF(CMAKE_VERSION VERSION_GREATER 3.0.0)
  ### new GUI is qt5 based -- but we don't want to fiddle with it
  ### unless the cmake version is at 3+
  FIND_PACKAGE(Qt5Widgets )
  IF(Qt5Widgets_FOUND)
    MESSAGE("Building qt gui")
    FIND_PACKAGE(Qt5Core REQUIRED )
    FIND_PACKAGE(Qt5Network REQUIRED )            
    FIND_PACKAGE(Qwt REQUIRED)    
    ADD_SUBDIRECTORY(qtgui)
  ENDIF()
ENDIF()

## Need hamlib for just the include files to get hamlib codes for
## modulation scheme and other settings.
find_package(PkgConfig)
pkg_search_module(HAMLIB REQUIRED hamlib)

### wxWidgets 3.0 is a trainwreck
IF(ENABLE_WX_WIDGETS) 
  set(wxWidgets_CONFIG_OPTIONS --version=2.8)
  find_package(wxWidgets COMPONENTS core base richtext aui xrc qa html adv  QUIET)
  include(${wxWidgets_USE_FILE})
  IF(wxWidgets_FOUND)
    MESSAGE("Building wx gui")  
    ADD_SUBDIRECTORY(wxgui)
  ENDIF()

  IF(BUILD_UNIT_TESTS)
    ADD_SUBDIRECTORY(exp)
  ENDIF()
ENDIF()

FIND_PACKAGE(Doxygen QUIET)

IF(DOXYGEN_FOUND)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  ADD_CUSTOM_TARGET(doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile 
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
ENDIF()
