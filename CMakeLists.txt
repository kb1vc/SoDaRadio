# Substantial parts written or modified by Claude Sonnet 4.6 (claude-sonnet-4-6)
CMAKE_MINIMUM_REQUIRED(VERSION 3.11.0) # this lets out really old distros like CentOS 6

include(CMakePrintHelpers)
include(GNUInstallDirs)

# Can't seem to fix Package Version warnings... defer.  Never put off to tomorrow what you can do at the end of time
# see here: https://github.com/PointCloudLibrary/pcl/issues/3680
# when this is fixed, we can remove the following 3 lines.
if(NOT DEFINED CMAKE_SUPPRESS_DEVELOPER_WARNINGS)
  set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE INTERNAL "No dev warnings")
endif()
   
OPTION(BUILD_RPM     "Build a binary RPM package (Fedora) via 'make package'." OFF)
OPTION(BUILD_DEB     "Build a binary Debian package (Ubuntu) via 'make package'." OFF)
OPTION(BUILD_SRC_DEB "Build an Ubuntu/Debian source package via 'make package_src_deb'." OFF)
OPTION(BUILD_UNIT_TESTS "Build the unit tests -- not normally useful" OFF)

IF(CMAKE_VERSION VERSION_GREATER 3.0.0)
  CMAKE_POLICY(SET CMP0048 NEW)
  SET(CMAKE_CXX_STANDARD 11)
ENDIF()

IF(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
  message("Hey!  This must be a Mac!")
  set(MACOSX 1)
  ADD_DEFINITIONS(-DMACOSX=1)
ELSE()
  set(MACOSX 0)
  ADD_DEFINITIONS(-DMACOSX=0)  
ENDIF()
  

  
PROJECT(SoDaRadio)
# Huge bump in version -- replacing crufty stuff with new SoDaLibs
# implementations of filters, resamplers, mailboxes, ....
# version 10 moves TX audio from portaudio/whatever to Qt audio stream
# 10.1.0 Fixed NBMF demod and a mistake in LSB demod. (LSB filter was
# "on the wrong side" and nbmf demod had the squelch set wayyyyy too high.
# there must have been something that changed higher-up in the chain.
# 11.0 - Beginning of port to qt6 -- RX works now!
SET(SoDaRadio_VERSION_MAJOR 11)
SET(SoDaRadio_VERSION_MINOR 0)
SET(SoDaRadio_VERSION_PATCH 0)
SET(SoDaRadio_VERSION "${SoDaRadio_VERSION_MAJOR}.${SoDaRadio_VERSION_MINOR}.${SoDaRadio_VERSION_PATCH}")

set(SoDaRadio_PACKAGE_DESCRIPTION 
  "SoDaRadio is a software defined radio implementation of "
  "a general purpose transmitter/receiver using the "
  "Ettus Research USRP hardware.")

# Select package generators.  RPM and DEB can be enabled together.
set(CPACK_GENERATOR "")
IF(BUILD_RPM)
  list(APPEND CPACK_GENERATOR "RPM")
ENDIF()
IF(BUILD_DEB)
  list(APPEND CPACK_GENERATOR "DEB")
ENDIF()

# Find the platform release name.  Prefer lsb_release; fall back to
# /etc/os-release (lsb_release is no longer installed by default on
# minimal Fedora/Debian images).
FIND_PROGRAM(LSB_RELEASE lsb_release)
IF(LSB_RELEASE)
  EXECUTE_PROCESS(COMMAND ${LSB_RELEASE} -is
    OUTPUT_VARIABLE OS_RELEASE_NAME
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  EXECUTE_PROCESS(COMMAND ${LSB_RELEASE} -rs
    OUTPUT_VARIABLE OS_RELEASE_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)
ELSEIF(EXISTS "/etc/os-release")
  file(STRINGS "/etc/os-release" _os_release_lines)
  foreach(_line IN LISTS _os_release_lines)
    if(_line MATCHES "^ID=\"?([^\"]+)\"?$")
      set(OS_RELEASE_NAME "${CMAKE_MATCH_1}")
    elseif(_line MATCHES "^VERSION_ID=\"?([^\"]+)\"?$")
      set(OS_RELEASE_VERSION "${CMAKE_MATCH_1}")
    endif()
  endforeach()
ENDIF()
IF(NOT OS_RELEASE_NAME)
  SET(OS_RELEASE_NAME "unknown")
ENDIF()
IF(NOT OS_RELEASE_VERSION)
  SET(OS_RELEASE_VERSION "0")
ENDIF()

function (getListOfVarsStartingWith _prefix _varResult)
    get_cmake_property(_vars VARIABLES)
    string (REGEX MATCHALL "(^|;)${_prefix}[A-Za-z0-9_]*" _matchedVars "${_vars}")
    set (${_varResult} ${_matchedVars} PARENT_SCOPE)
endfunction()

FIND_PACKAGE(Threads REQUIRED)

# ---------------------------------------------------------------------------
# Common CPack settings
# ---------------------------------------------------------------------------
SET(CPACK_PACKAGE_VERSION_MAJOR ${SoDaRadio_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${SoDaRadio_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${SoDaRadio_VERSION_PATCH})
SET(CPACK_PACKAGE_VERSION ${SoDaRadio_VERSION})
SET(CPACK_PACKAGE_NAME "SoDaRadio")
SET(CPACK_PACKAGE_RELEASE 1)
SET(CPACK_PACKAGE_CONTACT "Matt Reilly <radiogeek381@gmail.com>")
SET(CPACK_PACKAGE_VENDOR "Matt Reilly")
SET(CPACK_PACKAGE_HOMEPAGE_URL "https://kb1vc.github.io/SoDaRadio/")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY
  "Software-defined radio for USRP, ADALM-Pluto, and RTL-SDR hardware")
SET(CPACK_PACKAGE_DESCRIPTION ${SoDaRadio_PACKAGE_DESCRIPTION})
IF(EXISTS "${PROJECT_SOURCE_DIR}/COPYING")
  SET(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/COPYING")
ENDIF()
# Default file name (the generators below override this with their own
# conventions: name-ver-rel.arch.rpm  and  name_ver_arch.deb).
SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}.${OS_RELEASE_NAME}-${OS_RELEASE_VERSION}")
# Both the RPM and DEB generators default the install prefix to /usr, so we
# don't set CPACK_PACKAGING_INSTALL_PREFIX globally -- setting it would also
# bleed into the source-package generator and prefix every source file with
# /usr/.

# ---------------------------------------------------------------------------
# RPM (Fedora) settings
# ---------------------------------------------------------------------------
set(CPACK_RPM_PACKAGE_RELEASE "1")
set(CPACK_RPM_PACKAGE_LICENSE "BSD-2-Clause")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Communications")
set(CPACK_RPM_PACKAGE_URL "${CPACK_PACKAGE_HOMEPAGE_URL}")
set(CPACK_RPM_PACKAGE_SUMMARY "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
set(CPACK_RPM_PACKAGE_DESCRIPTION ${SoDaRadio_PACKAGE_DESCRIPTION})
set(CPACK_RPM_PACKAGE_RELOCATABLE TRUE)
# Runtime dependencies on Fedora.  We disable auto-requires because rpm
# turns the absolute uhd version into an unsatisfiable expression
# otherwise.
set(CPACK_RPM_PACKAGE_AUTOREQPROV "no")
set(CPACK_RPM_PACKAGE_REQUIRES
  "uhd >= 3.12.0, "
  "fftw-libs >= 3.3, "
  "qt6-qtbase >= 6.2, "
  "qt6-qtmultimedia >= 6.2, "
  "qwt-qt6 >= 6.1.2, "
  "gpsd >= 3.1, "
  "gpsd-libs >= 3.1, "
  "hamlib")
string(REPLACE ";" "" CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}")
set(CPACK_RPM_FILE_NAME
  "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}.rpm")

# ---------------------------------------------------------------------------
# DEB (Ubuntu / Debian) settings
# ---------------------------------------------------------------------------
# Debian convention is an all-lowercase package name.
set(CPACK_DEBIAN_PACKAGE_NAME "sodaradio")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_CONTACT}")
set(CPACK_DEBIAN_PACKAGE_SECTION "hamradio")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "${CPACK_PACKAGE_HOMEPAGE_URL}")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION
  "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}\n SoDaRadio is a software defined radio implementation of a general\n purpose transmitter/receiver supporting Ettus USRP, ADALM-Pluto, and\n RTL-SDR hardware.")
# Let dpkg-shlibdeps derive the .so library dependencies from the built
# binaries -- this avoids hard-coding distro-specific package names
# (e.g. libuhdNNN where the soname version changes per release).
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
# Add packages that aren't picked up by shlibdeps (firmware/daemon
# helpers).  These are merged with the shlibdeps results.
set(CPACK_DEBIAN_PACKAGE_DEPENDS "uhd-host (>= 3.12), gpsd (>= 3.1)")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

INCLUDE(CPack)

# ---------------------------------------------------------------------------
# Ubuntu / Debian source package  ('make package_src_deb')
# ---------------------------------------------------------------------------
# Stages a clean copy of the source tree, drops in a configured debian/
# directory built from packaging/debian/, and runs dpkg-source -b to
# produce  sodaradio_<ver>-1.dsc / .debian.tar.xz / .orig.tar.xz  ready
# for upload to a PPA or rebuild with sbuild / pbuilder.
IF(BUILD_SRC_DEB)
  # Generate the changelog with the current version and RFC 5322 date.
  execute_process(COMMAND date "-R"
                  OUTPUT_VARIABLE DEB_CHANGELOG_DATE
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  CONFIGURE_FILE(
    "${PROJECT_SOURCE_DIR}/packaging/debian/changelog.in"
    "${PROJECT_BINARY_DIR}/debian/changelog"
    @ONLY)
  # The static debian/ files just get copied across.
  FILE(COPY
    "${PROJECT_SOURCE_DIR}/packaging/debian/control"
    "${PROJECT_SOURCE_DIR}/packaging/debian/rules"
    "${PROJECT_SOURCE_DIR}/packaging/debian/copyright"
    DESTINATION "${PROJECT_BINARY_DIR}/debian"
    USE_SOURCE_PERMISSIONS)
  FILE(COPY
    "${PROJECT_SOURCE_DIR}/packaging/debian/source"
    DESTINATION "${PROJECT_BINARY_DIR}/debian")

  ADD_CUSTOM_TARGET(package_src_deb
    COMMAND ${CMAKE_COMMAND}
      -DSRC_DIR=${PROJECT_SOURCE_DIR}
      -DBIN_DIR=${PROJECT_BINARY_DIR}
      -DDEBIAN_DIR=${PROJECT_BINARY_DIR}/debian
      -DDEB_SRC_NAME=sodaradio
      -DVERSION=${SoDaRadio_VERSION}
      -P ${PROJECT_SOURCE_DIR}/cmake/scripts/build_src_deb.cmake
    COMMENT "Building Ubuntu/Debian source package (dpkg-source -b)"
    VERBATIM)
ENDIF()

# Install COPYING and README into the package as documentation.
IF(EXISTS "${PROJECT_SOURCE_DIR}/COPYING")
  INSTALL(FILES "${PROJECT_SOURCE_DIR}/COPYING"
    DESTINATION "share/doc/SoDaRadio")
ENDIF()
IF(EXISTS "${PROJECT_SOURCE_DIR}/README")
  INSTALL(FILES "${PROJECT_SOURCE_DIR}/README"
    DESTINATION "share/doc/SoDaRadio")
ENDIF()

# optimizations
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug)
ENDIF()

SET(CMAKE_CXX_FLAGS_DEBUG "-g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math")

MESSAGE("SoDaRadio_VERSION=[${SoDaRadio_VERSION}]")

# SoDaRadio's production sources no longer use Boost directly.  Any libboost
# entries that show up in 'ldd SoDaServer' arrive transitively through libuhd
# or hamlib, not through us.

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

INSTALL(FILES ${PROJECT_BINARY_DIR}/version.h DESTINATION "include/SoDaRadio")

CONFIGURE_FILE (
  "${PROJECT_SOURCE_DIR}/SoDaRadioConfigVersion.cmake.in"
  "${PROJECT_BINARY_DIR}/SoDaRadioConfigVersion.cmake"
  @ONLY
  )  

INSTALL(FILES ${PROJECT_BINARY_DIR}/SoDaRadioConfigVersion.cmake DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/SoDaRadio")
INSTALL(FILES ${PROJECT_SOURCE_DIR}/SoDaRadioConfig.cmake DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/SoDaRadio")

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")


OPTION(NO_USRP "Disable Ettus USRP / libuhd support" OFF)

IF(${NO_USRP})
  MESSAGE("libuhd / USRP support disabled.")
  SET(HAVE_UHD 0)
ELSE()
  FIND_PACKAGE(UHD "3.11" QUIET)
  IF(UHD_FOUND)
    SET(HAVE_UHD 1)
    MESSAGE("libuhd ${UHD_VERSION} found -- USRP support enabled.")
  ELSE()
    SET(HAVE_UHD 0)
    MESSAGE("libuhd NOT found -- USRP support disabled.")
  ENDIF()
ENDIF()



if(MACOSX)
  message("Looking for OPENGL")
  FIND_PACKAGE(OpenGL REQUIRED)
  message("looked for opengl")
  FIND_PACKAGE(GLUT REQUIRED)
  LIST(APPEND INCLUDE_DIRS ${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR})
  LIST(APPEND LIBRARIES ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})

  FIND_PACKAGE(Qt6 REQUIRED COMPONENTS core network printsupport multimedia)

  FIND_PACKAGE(Qwt REQUIRED)
else()
  FIND_PACKAGE(Qt6Widgets )
  IF(Qt6Widgets_FOUND)
    MESSAGE("Building qt gui")
    FIND_PACKAGE(Qt6Core REQUIRED )
    FIND_PACKAGE(Qt6Network REQUIRED )
    FIND_PACKAGE(Qt6PrintSupport REQUIRED )
    FIND_PACKAGE(Qt6Multimedia REQUIRED )                    
    FIND_PACKAGE(Qwt REQUIRED)    
    IF(Qt6Widgets_VERSION VERSION_LESS 5.7.0)
      MESSAGE(FATAL_ERROR "Minimum supported Qt6 version is 5.7")
    ENDIF()
  ELSE()
    MESSAGE(FATAL_ERROR "The Qt6Widgets library is required to build SoDaRadio")
  ENDIF()
endif()

#set(CMAKE_AUTOUIC ON)


## Need hamlib for just the include files to get hamlib codes for
## modulation scheme and other settings.
find_package(PkgConfig)
pkg_search_module(HAMLIB REQUIRED hamlib)
pkg_search_module(GPSLIB OPTIONAL libgps)

OPTION(NO_PLUTO "Disable ADALM-PLUTO / libiio support" OFF)

IF(${NO_PLUTO})
  MESSAGE("libiio / ADALM-PLUTO support disabled.")
  SET(HAVE_PLUTO 0)
ELSE()
  pkg_check_modules(LIBIIO QUIET libiio)
  IF(LIBIIO_FOUND)
    SET(HAVE_PLUTO 1)
    MESSAGE("libiio ${LIBIIO_VERSION} found -- ADALM-PLUTO support enabled.")
  ELSE()
    SET(HAVE_PLUTO 0)
    MESSAGE("libiio NOT found -- ADALM-PLUTO support disabled.")
  ENDIF()
ENDIF()

OPTION(NO_RTLSDR "Disable RTL-SDR / librtlsdr support" OFF)

IF(${NO_RTLSDR})
  MESSAGE("librtlsdr support disabled.")
  SET(HAVE_RTLSDR 0)
ELSE()
  pkg_check_modules(LIBRTLSDR QUIET librtlsdr)
  IF(LIBRTLSDR_FOUND)
    SET(HAVE_RTLSDR 1)
    MESSAGE("librtlsdr ${LIBRTLSDR_VERSION} found -- RTL-SDR support enabled.")
  ELSE()
    SET(HAVE_RTLSDR 0)
    MESSAGE("librtlsdr NOT found -- RTL-SDR support disabled.")
  ENDIF()
ENDIF()



IF(DISABLE_GPSD)
  SET(HAVE_GPSLIB 0)
  message("DISABLED GPSLIB")
ELSE()  
  IF(GPSLIB_FOUND)
    SET(HAVE_GPSLIB 1)
    message("FOUND GPSLIB version " ${GPSLIB_VERSION})
  ELSE()
    SET(HAVE_GPSLIB 0)
    message("DID NOT FIND GPSLIB")  
  ENDIF()
ENDIF()
UNSET(DISABLE_GPSD CACHE)

if(MACOSX)
  set(RT_LIB "")
else()
  set(RT_LIB "rt")  
endif()

# Find SoDaUtils and build it if we must. 
include(${PROJECT_SOURCE_DIR}/cmake/Modules/BuildSoDaUtils.cmake)

# This is the radio interface... 
ADD_SUBDIRECTORY(src)

## ADD_SUBDIRECTORY(exp)

ADD_SUBDIRECTORY(examples)

ADD_SUBDIRECTORY(qtgui)

ADD_SUBDIRECTORY(TestCLI)

ADD_SUBDIRECTORY(tools)

FIND_PACKAGE(Doxygen QUIET)

IF(DOXYGEN_FOUND)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  ADD_CUSTOM_TARGET(doc ALL
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile 
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
    )

  INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc)
ENDIF()


