cmake_minimum_required(VERSION 3.0)

cmake_policy(SET CMP0048 NEW)

project(SoDaRadio)

option(BUILD_UNIT_TESTS "Build the unit tests -- not normally useful" OFF)

set(SoDaRadio_VERSION_MAJOR 4)
set(SoDaRadio_VERSION_MINOR 0)
set(SoDaRadio_VERSION_PATCH "pre")
set(SoDaRadio_VERSION "${SoDaRadio_VERSION_MAJOR}.${SoDaRadio_VERSION_MINOR}.${SoDaRadio_VERSION_PATCH}")

# Package generation for the source tarball
set(CPACK_PACKAGE_VERSION_MAJOR ${SoDaRadio_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${SoDaRadio_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${SoDaRadio_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${SoDaRadio_VERSION})
set(CPACK_PACKAGE_NAME "SoDaRadio")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "Matt Reilly kb1vc")
set(CPACK_PACKAGE_VENDOR "Matt Reilly -- radiogeek381@gmail.com")
# RPM stuff doesn't work and it is screwing up the dest dir for the source package
# set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
# set(CPACK_GENERATOR TGZ RPM)
set(CPACK_SOURCE_GENERATOR TGZ)
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_SOURCE_IGNORE_FILES "/build/" "~$" "SoDa.soda_cfg$" "SoDa.soda_log$" "ckp$" "/.git/")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/project_rpm.spec.in"
  "${CMAKE_CURRENT_BINARY_DIR}/project_rpm.spec" @ONLY)
set(CPACK_RPM_USER_BINARY_SPECFILE "${CMAKE_CURRENT_BINARY_DIR}/project_rpm.spec")
include(CPack)




message("SoDaRadio_VERSION=[${SoDaRadio_VERSION}]")


find_package(Git)

if((EXISTS "${CMAKE_SOURCE_DIR}/.git/") AND GIT_FOUND)
  message("Recording GIT version info in ${PROJECT_SOURCE_DIR}/saved_git_version.txt")
  execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  set(SoDaRadio_GIT_ID "${GIT_BRANCH}:${GIT_COMMIT_HASH}")
  file(WRITE ${PROJECT_SOURCE_DIR}/saved_git_version.txt "${GIT_BRANCH}\n")
  file(APPEND ${PROJECT_SOURCE_DIR}/saved_git_version.txt "${GIT_COMMIT_HASH}\n")    
elseif(EXISTS ${PROJECT_SOURCE_DIR}/saved_git_version.txt)
  message("Retrieving GIT version info from saved_git_version.txt")
  file(STRINGS ${PROJECT_SOURCE_DIR}/saved_git_version.txt git_version_strings)
  list(GET git_version_strings 0 GIT_BRANCH)
  list(GET git_version_strings 1 GIT_COMMIT_HASH)
  set(SoDaRadio_GIT_ID "${GIT_BRANCH}:${GIT_COMMIT_HASH}")
else()
  set(SoDaRadio_GIT_ID "UNKNOWN")  
endif()

set(DOXY_PROJECT_VERSION "${SoDaRadio_VERSION}-${SoDaRadio_GIT_ID}")
set(DOXY_EXCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/gui/Default.soda_cfg.h ${CMAKE_CURRENT_BINARY_DIR}/SoDaRadio_GUI.cxx")

configure_file (
"${PROJECT_SOURCE_DIR}/version.h.in"
"${PROJECT_BINARY_DIR}/version.h"
  )
include_directories("${PROJECT_BINARY_DIR}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")


find_package(Boost 1.45 REQUIRED COMPONENTS program_options thread date_time system filesystem serialization regex)

add_subdirectory(src)
add_subdirectory(gui)
if(BUILD_UNIT_TESTS)
  add_subdirectory(exp)
endif()

find_package(Doxygen)

if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile 
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif()
