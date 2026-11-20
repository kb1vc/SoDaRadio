# Substantial parts written or modified by Claude Sonnet 4.6 (claude-sonnet-4-6)
CMAKE_MINIMUM_REQUIRED(VERSION 3.11.0) # this lets out really old distros like CentOS 6

include(CMakePrintHelpers)

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
OPTION(BUILD_SODALIBS
  "Fetch and build SoDaLibs from GitHub instead of using the installed copy" OFF)
OPTION(BUILD_FFTW
  "Fetch and build FFTW3 from source instead of using the system copy" OFF)
OPTION(BUILD_LIBIIO
  "Fetch and build libiio from GitHub instead of using the installed copy" OFF)
OPTION(BUILD_LIBRTLSDR
  "Fetch and build librtlsdr from GitHub instead of using the installed copy" OFF)
set(SODALIBS_TAG "v_U5.3.2_S4.4.0" CACHE STRING
  "SoDaLibs git tag to fetch when BUILD_SODALIBS=ON or SoDaLibs is not installed")

IF(CMAKE_VERSION VERSION_GREATER 3.0.0)
  CMAKE_POLICY(SET CMP0048 NEW)
  SET(CMAKE_CXX_STANDARD 17)
ENDIF()

IF(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
  message(FATAL_ERROR "Hey!  This must be a Mac! This build won't work. (yet)")
ELSE()
  set(MACOSX 0)
  ADD_DEFINITIONS(-DMACOSX=0)  
ENDIF()
  

  
PROJECT(SoDaRadio)
include(GNUInstallDirs)
# Huge bump in version -- replacing crufty stuff with new SoDaLibs
# implementations of filters, resamplers, mailboxes, ....
# version 10 moves TX audio from portaudio/whatever to Qt audio stream
# 10.1.0 Fixed NBMF demod and a mistake in LSB demod. (LSB filter was
# "on the wrong side" and nbmf demod had the squelch set wayyyyy too high.
# there must have been something that changed higher-up in the chain.
# 11.0 - Beginning of port to qt6 -- RX works now!
# 12.0.0 - All Qt audio. Support for USRP, Pluto, RTLSDR. 
# 12.1.0 - Cleanup, remove all gpsd gpsd-lib dependencies. Fixed logging bug.
# Improved README a little bit.
# 12.2.0 - Removed spurious build dependencies and made SoDaLibs,
# FFTW, libiio, and librtlsdr external projects - they can be
# (optionally) built as part of the SoDaRadio build.
# 12.3.0 - package support, refactoring of common gui components
# 12.3.1 - CW carrier offset, fix to initialization bug in RadioControl
# 12.3.2 - Moved buttons/controls to make them more convenient and accessible
# without adding too much visual noise to panels. Working on in-app
# documentation. Added hook to CMake that triggers version update with
# each new commit.
# 12.4.0 - Added more help documentation. Lots more.
# 12.5.0 - Fixed Qt deprecation warnings. Added power and VU meters. Added help key.
# Improved TX audio control. Improved restore settings operations. Fixed Qt complaint
# about audio formats for oddball virtual terminal (NX) devices.
# Big time improvement: Tuning offsets for LSB vs. USB were wrong. "Fixed" TX buffer
# under-run.
# 12.5.1 - It must have been something important. 
# 12.5.2 - Added help button. Updated README.md. Added instructions
# to create a new radio model. Updated AF recorder and RF recorder. Fixed
# a problem in the NBFM demod that modified its own input buffer (shared with
# everyone else sharing the mailbox.)
# 12.5.3 - Forgot to add a build script for FLAC when I pushed 12.5.2... sigh
# 12.5.4 - "Current" ubuntu build uses Qt 6.4 which had not yet introduced
# the QtAudio namespace. This caused the build to fail
# 12.5.5 - picked the wrong week to give up sniffing glue.
# 12.6.0 - Performance fiddles. Changed sleep length for RadioTX to 1ms when
# transmit is off. Big deal: changed QuadratureOscillator to produce vectors
# of NCO samples to allow downmixing to use vector instructions. Finally, PLUTO
# (and USRP N3xx) requires downsampling from the hardware sample rate to the
# SoDa preferred rate of 625 kS/s.  That was originally a normal SoDa::ReSampler
# but this isn't nearly as efficient as a well designed half-band sampler. Opus
# helped me add a cascaded half-band sampler to get the 4:1 reduction.
# 12.6.1 - AM demod was subject to really bad intermod.  Tweaking the AM
# demod prefilter
# 12.6.2 - Forgot to commit the new 4:1 resampler.  Things could be going
# better. 
SET(SoDaRadio_VERSION_MAJOR 12)
SET(SoDaRadio_VERSION_MINOR 6)
SET(SoDaRadio_VERSION_PATCH 2)
SET(SoDaRadio_VERSION "${SoDaRadio_VERSION_MAJOR}.${SoDaRadio_VERSION_MINOR}.${SoDaRadio_VERSION_PATCH}")

set(SoDaRadio_PACKAGE_DESCRIPTION 
  "SoDaRadio is a software defined radio implementation of "
  "a general purpose transmitter/receiver")

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
# Runtime dependencies on Fedora.  Let rpm's built-in dependency scanner
# derive the per-SONAME requirements directly from the installed binaries
# (libuhd, libssl, libcrypto, libxml2, libqwt-qt6, Qt6, ...).  Maintaining
# that list by hand is a losing battle — we ended up shipping a package
# that installed cleanly but failed at runtime with "not found" on
# openssl and libxml2 because the manual list omitted them.
#
# CPACK_RPM_PACKAGE_REQUIRES is additive when AUTOREQPROV is on, so the
# human-readable package pins below are merged with the auto-detected
# SONAME requires.  They serve as documentation of our minimum-supported
# versions and as a second line of defence if a dependency is ever loaded
# via dlopen() and thus invisible to the scanner.
set(CPACK_RPM_PACKAGE_AUTOREQPROV "yes")
set(CPACK_RPM_PACKAGE_REQUIRES
  "uhd >= 3.12.0, "
  "qt6-qtbase >= 6.2, "
  "qt6-qtmultimedia >= 6.2, "
  "qwt-qt6 >= 6.1.2")
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
set(CPACK_DEBIAN_PACKAGE_DEPENDS "uhd-host (>= 3.12)")
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

SET(CMAKE_CXX_FLAGS_DEBUG "-g -O3 -fno-omit-frame-pointer")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math")

MESSAGE("SoDaRadio_VERSION=[${SoDaRadio_VERSION}]")

# SoDaRadio's production sources no longer use Boost directly.  Any libboost
# entries that show up in 'ldd SoDaServer' arrive transitively through libuhd
# or hamlib, not through us.

# Capture git version now (needed for DOXY_PROJECT_VERSION etc.) and also
# schedule a build-time refresh so a new commit made after configuration
# still updates saved_git_version.txt and version.h on the next `make`.
SET(SRC_DIR      "${CMAKE_SOURCE_DIR}")
SET(BIN_DIR      "${CMAKE_BINARY_DIR}")
SET(VERSION_H_IN "${CMAKE_SOURCE_DIR}/version.h.in")
SET(SODA_VERSION "${SoDaRadio_VERSION}")
INCLUDE("${CMAKE_SOURCE_DIR}/gen_version.cmake")

ADD_CUSTOM_TARGET(sodaradio_version_gen ALL
  BYPRODUCTS
    ${CMAKE_BINARY_DIR}/saved_git_version.txt
    ${CMAKE_BINARY_DIR}/version.h
  COMMAND ${CMAKE_COMMAND}
    -DSRC_DIR=${CMAKE_SOURCE_DIR}
    -DBIN_DIR=${CMAKE_BINARY_DIR}
    -DVERSION_H_IN=${CMAKE_SOURCE_DIR}/version.h.in
    -DSODA_VERSION=${SoDaRadio_VERSION}
    -P ${CMAKE_SOURCE_DIR}/gen_version.cmake
  COMMENT "Refreshing git commit info into saved_git_version.txt / version.h"
  VERBATIM
)

SET(DOXY_PROJECT_VERSION "${SoDaRadio_VERSION}-${SoDaRadio_GIT_ID}")
SET(DOXY_EXCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/gui/Default.soda_cfg.h ${CMAKE_CURRENT_BINARY_DIR}/SoDaRadio_GUI.cxx")
CONFIGURE_FILE (
"${PROJECT_SOURCE_DIR}/help_path.h.in"
"${PROJECT_BINARY_DIR}/help_path.h"
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
  SET(HAVE_UHD 0)
ELSE()
  FIND_PACKAGE(UHD "4.8.0" QUIET)
  IF(UHD_FOUND)
    SET(HAVE_UHD 1)
  ELSE()
    SET(HAVE_UHD 0)
  ENDIF()
ENDIF()



FIND_PACKAGE(Qt6Widgets)
IF(Qt6Widgets_FOUND)
  FIND_PACKAGE(Qt6Core REQUIRED)
  FIND_PACKAGE(Qt6Network REQUIRED)
  FIND_PACKAGE(Qt6PrintSupport REQUIRED)
  FIND_PACKAGE(Qt6Multimedia REQUIRED)
  FIND_PACKAGE(Qt6Help QUIET)
  FIND_PACKAGE(Qt6WebEngineWidgets QUIET)
  FIND_PACKAGE(Qwt REQUIRED)
  IF(Qt6Widgets_VERSION VERSION_LESS 5.7.0)
    MESSAGE(FATAL_ERROR "Minimum supported Qt6 version is 5.7")
  ENDIF()
ELSE()
  MESSAGE(FATAL_ERROR "The Qt6Widgets library is required to build SoDaRadio")
ENDIF()

IF(Qt6Help_FOUND)
  MESSAGE(STATUS "Qt6Help found -- building in-app help browser")
  FIND_PROGRAM(QHELPGENERATOR_EXECUTABLE
    NAMES qhelpgenerator qhelpgenerator-qt6 qhelpgenerator6
    HINTS "${Qt6_DIR}/../../../bin" "${Qt6Core_DIR}/../../../bin"
    PATHS "/usr/lib64/qt6/libexec" "/usr/lib/qt6/libexec"
          "/usr/libexec/qt6" "/opt/qt6/libexec")
  IF(QHELPGENERATOR_EXECUTABLE)
    MESSAGE(STATUS "qhelpgenerator: ${QHELPGENERATOR_EXECUTABLE}")
    SET(DOXY_QHG_LOCATION "${QHELPGENERATOR_EXECUTABLE}")
  ELSE()
    MESSAGE(STATUS "qhelpgenerator not found -- .qch will not be compiled automatically")
    SET(DOXY_QHG_LOCATION "")
  ENDIF()
  SET(DOXY_QCH_FILE "SoDaRadio.qch")
ELSE()
  MESSAGE(STATUS "Qt6Help not found -- help button will open web browser")
  SET(DOXY_QHG_LOCATION "")
  SET(DOXY_QCH_FILE "")
ENDIF()

#set(CMAKE_AUTOUIC ON)


## Hamlib is no longer an external dependency: the handful of RIG_MODE_*,
## RIG_VFO_*, and RIG_ANT_* constants that HamlibHandler / HamlibServer need
## for the rigctld wire protocol are vendored in vendor/hamlib/rig.h.
find_package(PkgConfig)
OPTION(NO_PLUTO "Disable ADALM-PLUTO / libiio support" OFF)

IF(${NO_PLUTO})
  SET(HAVE_PLUTO 0)
ELSE()
  IF(BUILD_LIBIIO)
    include(BuildLibIIO)
  ELSE()
    pkg_check_modules(LIBIIO QUIET libiio)
  ENDIF()
  IF(LIBIIO_FOUND)
    SET(HAVE_PLUTO 1)
  ELSE()
    SET(HAVE_PLUTO 0)
  ENDIF()
ENDIF()

OPTION(ENABLE_PLUTO_TX "Enable ADALM-Pluto TX hardware output" OFF)

OPTION(NO_RTLSDR "Disable RTL-SDR / librtlsdr support" OFF)

IF(${NO_RTLSDR})
  SET(HAVE_RTLSDR 0)
ELSE()
  IF(BUILD_LIBRTLSDR)
    include(BuildLibRTLSDR)
  ELSE()
    pkg_check_modules(LIBRTLSDR QUIET librtlsdr)
  ENDIF()
  IF(LIBRTLSDR_FOUND)
    SET(HAVE_RTLSDR 1)
  ELSE()
    SET(HAVE_RTLSDR 0)
  ENDIF()
ENDIF()

add_definitions(-DHAVE_UHD=${HAVE_UHD})
add_definitions(-DHAVE_PLUTO=${HAVE_PLUTO})
add_definitions(-DHAVE_RTLSDR=${HAVE_RTLSDR})

# libFLAC -- required by the AudioRecorder in the GUI.  Try the system copy
# first; if that fails (or BUILD_FLAC is explicitly ON), fetch and build a
# static libFLAC from xiph/flac on GitHub.
OPTION(BUILD_FLAC "Force a from-source build of libFLAC even if the system has one" OFF)
IF(NOT BUILD_FLAC)
  pkg_check_modules(FLAC QUIET flac)
  IF(NOT FLAC_FOUND)
    # pkg-config missing on this host or no flac.pc installed -- try plain find_library.
    find_library(_flac_lib NAMES FLAC)
    find_path   (_flac_inc NAMES FLAC/stream_encoder.h)
    IF(_flac_lib AND _flac_inc)
      SET(FLAC_FOUND        TRUE)
      SET(FLAC_VERSION      "system")
      SET(FLAC_LIBRARIES    "${_flac_lib}")
      SET(FLAC_INCLUDE_DIRS "${_flac_inc}")
    ENDIF()
  ENDIF()
ENDIF()
IF(NOT FLAC_FOUND)
  IF(BUILD_FLAC)
    message(STATUS "BUILD_FLAC=ON -- fetching libFLAC from GitHub")
  ELSE()
    message(STATUS "System libFLAC not found -- fetching from GitHub")
  ENDIF()
  include(BuildFLAC)
ELSE()
  message(STATUS "Using libFLAC: ${FLAC_LIBRARIES} (${FLAC_VERSION})")
ENDIF()

set(RT_LIB "rt")  

# Find SoDaUtils and build it if we must. 
include(${PROJECT_SOURCE_DIR}/cmake/Modules/BuildSoDaUtils.cmake)

# This is the radio interface... 
ADD_SUBDIRECTORY(src)

## ADD_SUBDIRECTORY(exp)

ADD_SUBDIRECTORY(examples)

ADD_SUBDIRECTORY(qtgui)

ADD_SUBDIRECTORY(shortgui)

# Force the two GUI libraries (which #include "version.h") to wait for the
# build-time version refresh, so a new commit made after cmake configuration
# is reflected in the built binaries.
ADD_DEPENDENCIES(SoDaRadio_Lib sodaradio_version_gen)
ADD_DEPENDENCIES(ShortSoDa_Lib sodaradio_version_gen)

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

  INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc/SoDaRadio)
  IF(Qt6Help_FOUND AND QHELPGENERATOR_EXECUTABLE)
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/html/SoDaRadio.qch
      DESTINATION share/doc/SoDaRadio)
  ENDIF()
ENDIF()

# ---------------------------------------------------------------------------
# Build summary
# ---------------------------------------------------------------------------
set(_radios_enabled "")
IF(${HAVE_UHD})
  string(APPEND _radios_enabled "\n   USRP")
ENDIF()
IF(${HAVE_PLUTO})
  string(APPEND _radios_enabled "\n   ADALM-Pluto")
ENDIF()
IF(${HAVE_RTLSDR})
  string(APPEND _radios_enabled "\n   RTL-SDR")
ENDIF()

IF(_radios_enabled)
  message("\nRadios that will be built:${_radios_enabled}\n")
ELSE()
  message("\nWARNING: No radio hardware support enabled (no libuhd, libiio, or librtlsdr found)\n")
ENDIF()
