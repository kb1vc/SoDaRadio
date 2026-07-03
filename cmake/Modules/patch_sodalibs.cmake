# cmake/Modules/patch_sodalibs.cmake
#
# Standalone script invoked from BuildSoDaUtils.cmake's PATCH_COMMAND as
#   cmake -DSOURCE_DIR=<path-to-sodalibs-src> -P patch_sodalibs.cmake
#
# SoDaLibs' CMakeLists uses ${CMAKE_SOURCE_DIR} in five places (module
# path, .git presence check, git working dir, and the SoDa/ header
# symlink target in Signals).  In a top-level build CMAKE_SOURCE_DIR and
# PROJECT_SOURCE_DIR are the same; under FetchContent the former
# resolves to SoDaRadio's source tree, so the Signals SoDa/ symlink
# points at a directory that doesn't exist and sodasignals fails to
# find <SoDa/Exception.hxx> during compile.  Rewrite in place.

# Only Signals/CMakeLists.txt is patched.  The other uses of
# ${CMAKE_SOURCE_DIR} in SoDaLibs (top-level CMakeLists.txt: module path,
# .git presence check, git working dir) all fail *safely* in a subproject
# context and don't need to be fixed.  Patching them would inadvertently
# expose SoDaLibs' own FindSoDa_FFTW.cmake, whose add_library would then
# collide with the SoDa_FFTW::Float target BuildSoDaUtils.cmake creates.

set(_file "${SOURCE_DIR}/Signals/CMakeLists.txt")
if(EXISTS "${_file}")
  file(READ "${_file}" _content)
  string(REPLACE
    [=[${CMAKE_SOURCE_DIR}]=]
    [=[${PROJECT_SOURCE_DIR}]=]
    _content "${_content}")
  file(WRITE "${_file}" "${_content}")
endif()
