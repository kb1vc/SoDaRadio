CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0) # this lets out really old distros like CentOS 6

OPTION(BUILD_RPM "Build an RPM package for this platform, or something like it." OFF)
OPTION(BUILD_DEB "Build a Debian package for this platform, or something like it." OFF)
OPTION(BUILD_UNIT_TESTS "Build the unit tests -- not normally useful" OFF)

IF(CMAKE_VERSION VERSION_GREATER 3.0.0)
  CMAKE_POLICY(SET CMP0048 NEW)
  SET(CMAKE_CXX_STANDARD 11)
ENDIF()

PROJECT(SoDaFormat)

SET(SoDaFormat_VERSION_MAJOR 1)
SET(SoDaFormat_VERSION_MINOR 0)
SET(SoDaFormat_VERSION_PATCH 0)
SET(SoDaFormat_VERSION "${SoDaFormat_VERSION_MAJOR}.${SoDaFormat_VERSION_MINOR}.${SoDaFormat_VERSION_PATCH}")

set(SoDaFormat_PACKAGE_DEFZRIPTION 
  "SoDaFormat is a library for building simple transceivers on the USRP platform."
)

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

# optimizations
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug)
ENDIF()

SET(CMAKE_CXX_FLAGS_DEBUG "-g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math")

MESSAGE("SoDaFormat_VERSION=[${SoDaFormat_VERSION}]")

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
  SET(SoDaFormat_GIT_ID "${GIT_BRANCH}:${GIT_COMMIT_HASH}")
  FILE(WRITE ${CMAKE_BINARY_DIR}/saved_git_version.txt "${GIT_BRANCH}\n")
  FILE(APPEND ${CMAKE_BINARY_DIR}/saved_git_version.txt "${GIT_COMMIT_HASH}\n")    
elseIF(EXISTS ${CMAKE_BINARY_DIR}/saved_git_version.txt)
  MESSAGE("Retrieving GIT version info from saved_git_version.txt")
  FILE(STRINGS ${CMAKE_BINARY_DIR}/saved_git_version.txt git_version_strings)
  LIST(GET git_version_strings 0 GIT_BRANCH)
  LIST(GET git_version_strings 1 GIT_COMMIT_HASH)
  SET(SoDaFormat_GIT_ID "${GIT_BRANCH}:${GIT_COMMIT_HASH}")
else()
  SET(SoDaFormat_GIT_ID "UNKNOWN")  
ENDIF()

SET(DOXY_PROJECT_VERSION "${SoDaFormat_VERSION}-${SoDaFormat_GIT_ID}")

CONFIGURE_FILE (
"${PROJECT_SOURCE_DIR}/version.h.in"
"${PROJECT_BINARY_DIR}/version.h"
  )
INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}")

INSTALL(FILES ${PROJECT_BINARY_DIR}/version.h DESTINATION "include/SoDaFormat")

CONFIGURE_FILE (
  "${PROJECT_SOURCE_DIR}/SoDaFormatConfigVersion.cmake.in"
  "${PROJECT_BINARY_DIR}/SoDaFormatConfigVersion.cmake"
  @ONLY
  )  

INSTALL(FILES ${PROJECT_BINARY_DIR}/SoDaFormatConfigVersion.cmake DESTINATION "lib/cmake/SoDaFormat")
INSTALL(FILES ${PROJECT_SOURCE_DIR}/SoDaFormatConfig.cmake DESTINATION "lib/cmake/SoDaFormat")

# This is the radio interface... 
ADD_SUBDIRECTORY(src)

FIND_PACKAGE(Doxygen QUIET)

IF(DOXYGEN_FOUND)
  MESSAGE("Found Doxygen!")	
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  ADD_CUSTOM_TARGET(doc ALL
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile 
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
    )

  INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc)
ENDIF()

