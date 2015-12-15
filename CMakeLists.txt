cmake_minimum_required(VERSION 3.0)

cmake_policy(SET CMP0048 NEW)

project(SoDaRadio)

option(BUILD_UNIT_TESTS "Build the unit tests -- not normally useful" OFF)

set(SoDaRadio_VERSION_MAJOR 4)
set(SoDaRadio_VERSION_MINOR 0)
set(SoDaRadio_VERSION_PATCH "pre")

# Package generation for the source tarball
set(CPACK_PACKAGE_VERSION_MAJOR ${SoDaRadio_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${SoDaRadio_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${SoDaRadio_VERSION_PATCH})
set(CPACK_GENERATOR TGZ)
set(CPACK_SOURCE_GENERATOR TGZ)
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_SOURCE_IGNORE_FILES "/build/" "~$" "SoDa.soda_cfg$" "SoDa.soda_log$" "ckp$" "/.git/")
include(CPack)

set(SoDaRadio_VERSION "${SoDaRadio_VERSION_MAJOR}.${SoDaRadio_VERSION_MINOR}.${SoDaRadio_VERSION_PATCH}")

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

