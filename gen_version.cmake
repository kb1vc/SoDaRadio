#
# Refresh saved_git_version.txt and version.h from the current git state.
# Runs both at configure time (INCLUDE()'d from CMakeLists.txt) and at
# build time (via `cmake -P` from a custom target) so a new commit made
# after configuration is still picked up by the next `make` build.
#
# Required variables:
#   SRC_DIR       — top-level source directory
#   BIN_DIR       — top-level build directory
#   VERSION_H_IN  — path to version.h.in
#   SODA_VERSION  — the SoDaRadio semver (e.g. "12.3.1")
#
# Writes:
#   ${BIN_DIR}/saved_git_version.txt   (only when the content actually changed
#                                       so downstream targets don't rebuild
#                                       unnecessarily)
#   ${BIN_DIR}/version.h               (via configure_file)
#

find_package(Git QUIET)

if(GIT_FOUND AND EXISTS "${SRC_DIR}/.git")
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY "${SRC_DIR}"
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} log -1 --format=%h
    WORKING_DIRECTORY "${SRC_DIR}"
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE)
elseif(EXISTS "${BIN_DIR}/saved_git_version.txt")
  file(STRINGS "${BIN_DIR}/saved_git_version.txt" _saved_lines)
  list(GET _saved_lines 0 GIT_BRANCH)
  list(GET _saved_lines 1 GIT_COMMIT_HASH)
else()
  set(GIT_BRANCH "UNKNOWN")
  set(GIT_COMMIT_HASH "UNKNOWN")
endif()

set(SoDaRadio_GIT_ID  "${GIT_BRANCH}:${GIT_COMMIT_HASH}")
set(SoDaRadio_VERSION "${SODA_VERSION}")

set(_new_saved "${GIT_BRANCH}\n${GIT_COMMIT_HASH}\n")
set(_old_saved "")
if(EXISTS "${BIN_DIR}/saved_git_version.txt")
  file(READ "${BIN_DIR}/saved_git_version.txt" _old_saved)
endif()
if(NOT "${_new_saved}" STREQUAL "${_old_saved}")
  file(WRITE "${BIN_DIR}/saved_git_version.txt" "${_new_saved}")
endif()

configure_file("${VERSION_H_IN}" "${BIN_DIR}/version.h" @ONLY)
