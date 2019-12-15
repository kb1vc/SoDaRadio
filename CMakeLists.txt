CMAKE_MINIMUM_REQUIRED(VERSION 2.8.9) # this lets out really old distros like CentOS 6

OPTION(BUILD_RPM "Build an RPM package for this platform, or something like it." OFF)
OPTION(BUILD_DEB "Build a Debian package for this platform, or something like it." OFF)
OPTION(BUILD_UNIT_TESTS "Build the unit tests -- not normally useful" OFF)

IF(CMAKE_VERSION VERSION_GREATER 3.0.0)
  CMAKE_POLICY(SET CMP0048 NEW)
  SET(CMAKE_CXX_STANDARD 11)
ENDIF()

PROJECT(SoDaRadio)

SET(SoDaRadio_VERSION_MAJOR 7)
SET(SoDaRadio_VERSION_MINOR 0)
SET(SoDaRadio_VERSION_PATCH 0)
SET(SoDaRadio_VERSION "${SoDaRadio_VERSION_MAJOR}.${SoDaRadio_VERSION_MINOR}.${SoDaRadio_VERSION_PATCH}")

set(SoDaRadio_PACKAGE_DESCRIPTION 
  "SoDaRadio is a software defined radio implementation of "
  "a general purpose transmitter/receiver using the "
  "Ettus Research USRP hardware.")

IF(BUILD_RPM)
  set(CPACK_GENERATOR "RPM")
ENDIF()

IF(${BUILD_DEB})
  set(CPACK_GENERATOR "DEB")
ENDIF()

# Find the platform release name
FIND_PROGRAM(LSB_RELEASE lsb_release)
EXECUTE_PROCESS(COMMAND ${LSB_RELEASE} -is
  OUTPUT_VARIABLE OS_RELEASE_NAME
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
EXECUTE_PROCESS(COMMAND ${LSB_RELEASE} -rs
  OUTPUT_VARIABLE OS_RELEASE_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Package generation for the source tarball
SET(CPACK_PACKAGE_VERSION_MAJOR ${SoDaRadio_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${SoDaRadio_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${SoDaRadio_VERSION_PATCH})
SET(CPACK_PACKAGE_VERSION ${SoDaRadio_VERSION})
SET(CPACK_PACKAGE_NAME "SoDaRadio")
SET(CPACK_PACKAGE_RELEASE 1)
SET(CPACK_PACKAGE_CONTACT "Matt Reilly kb1vc")
SET(CPACK_PACKAGE_VENDOR "Matt Reilly -- radiogeek381@gmail.com")
SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}.${OS_RELEASE_NAME}-${OS_RELEASE_VERSION}")
SET(CPACK_PACKAGING_INSTALL_PREFIX "/usr")

set(CPACK_RPM_PACKAGE_RELEASE "1")
set(CPACK_RPM_PACKAGE_LICENSE "BSD-2-Clause")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Communications")
set(CPACK_RPM_PACKAGE_DESCRIPTION  ${SoDaRadio_PACKAGE_DESCRIPTION})
set(CPACK_RPM_PACKAGE_REQUIRES "boost >= 1.48, uhd >= 3.12.0, fftw-devel >= 3.3, qt5-qtbase >= 5.6, qwt-qt5 >= 6.1.2, gpsd >= 3.1, gpsd-libs >= 3.1, libsndfile, qt5-qtmultimedia")
set(CPACK_RPM_PACKAGE_RELOCATABLE TRUE)
# RPM makes a hash out of the uhd dependency if we don't do this:
set(CPACK_RPM_PACKAGE_AUTOREQPROV " no")
 
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION ${SODA_PACKAGE_DESCRIPTION})
set(CPACK_DEBIAN_PACKAGE_SECTION "hamradio")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libuhd003 (>= 3.12), uhd-host (>= 3.12), libgps-dev (>= 3.1), gpsd (>= 3.1), libsndfile1, libqt5multimedia5-plugins")

INCLUDE(CPack)

# optimizations
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
ENDIF()

SET(CMAKE_CXX_FLAGS_DEBUG "-g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math")

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

FIND_PACKAGE(SndFile REQUIRED)

FIND_PACKAGE(Boost 1.45 REQUIRED COMPONENTS program_options thread date_time system filesystem serialization regex QUIET)

IF(${NO_UHD})
  MESSAGE("libUHD Support disabled.")
  SET(HAVE_UHD 0)  
ELSE()
  # Enable UHD support
  FIND_PACKAGE(UHD "3.11" REQUIRED)
  IF(NOT UHD_FOUND)
    MESSAGE("UHD Library NOT FOUND!")
    SET(HAVE_UHD 0)  
  ELSE()
    SET(HAVE_UHD 1)
  ENDIF()
ENDIF()



IF(CMAKE_VERSION VERSION_GREATER 3.0.0)
  ### new GUI is qt5 based -- but we don't want to fiddle with it
  ### unless the cmake version is at 3+
  FIND_PACKAGE(Qt5Widgets )
  IF(Qt5Widgets_FOUND)
    MESSAGE("Building qt gui")
    FIND_PACKAGE(Qt5Core REQUIRED )
    FIND_PACKAGE(Qt5Network REQUIRED )
    FIND_PACKAGE(Qt5PrintSupport REQUIRED )
    FIND_PACKAGE(Qt5Multimedia REQUIRED )                    
    FIND_PACKAGE(Qwt REQUIRED)    
    IF(Qt5Widgets_VERSION VERSION_LESS 5.7.0)
      MESSAGE(FATAL_ERROR "Minimum supported Qt5 version is 5.7")
    ENDIF()
    ADD_SUBDIRECTORY(qtgui)
  ELSE()
    MESSAGE(FATAL_ERROR "The Qt5Widgets library is required to build SoDaRadio")
  ENDIF()
ENDIF()

## Need hamlib for just the include files to get hamlib codes for
## modulation scheme and other settings.
find_package(PkgConfig)
pkg_search_module(HAMLIB REQUIRED hamlib)
pkg_search_module(GPSLIB OPTIONAL libgps)
IF(GPSLIB_FOUND)
  SET(HAVE_GPSLIB 1)
  message("FOUND GPSLIB")
ELSE()
  SET(HAVE_GPSLIB 0)
  message("DID NOT FIND GPSLIB")  
ENDIF()

# This is the radio interface... 
ADD_SUBDIRECTORY(src)

ADD_SUBDIRECTORY(exp)

FIND_PACKAGE(Doxygen QUIET)

IF(DOXYGEN_FOUND)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  ADD_CUSTOM_TARGET(doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile 
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
ENDIF()

