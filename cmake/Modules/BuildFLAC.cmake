# cmake/Modules/BuildFLAC.cmake
#
# Fetch libFLAC from xiph/flac on GitHub and build it as a static library.
# Sets the same variables that pkg_check_modules(FLAC flac) would produce
# so the rest of the build tree doesn't have to know where libFLAC came
# from.
#
# Consumers:  FLAC_FOUND, FLAC_VERSION, FLAC_INCLUDE_DIRS, FLAC_LIBRARIES
# Config:     FLAC_TAG (default 1.5.0)
#
# AudioRecorder uses only the libFLAC C stream_encoder API, so we skip
# the C++ bindings, OGG support, CLI programs, examples, docs, and tests.

set(FLAC_TAG "1.5.0" CACHE STRING
  "libFLAC git tag to fetch when system FLAC is missing")

message(STATUS "Fetching libFLAC ${FLAC_TAG} from GitHub")

include(FetchContent)

FetchContent_Declare(
  flac
  GIT_REPOSITORY https://github.com/xiph/flac
  GIT_TAG        ${FLAC_TAG}
  GIT_SHALLOW    TRUE
  EXCLUDE_FROM_ALL
)

# Force a static-only, trimmed-down libFLAC build.
set(_bsl_save ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS      OFF CACHE BOOL "" FORCE)
set(BUILD_CXXLIBS          OFF CACHE BOOL "" FORCE)
set(BUILD_PROGRAMS         OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES         OFF CACHE BOOL "" FORCE)
set(BUILD_DOCS             OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING          OFF CACHE BOOL "" FORCE)
set(WITH_OGG               OFF CACHE BOOL "" FORCE)
set(INSTALL_MANPAGES       OFF CACHE BOOL "" FORCE)
set(INSTALL_PKGCONFIG_MODULES   OFF CACHE BOOL "" FORCE)
set(INSTALL_CMAKE_CONFIG_MODULE OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(flac)

set(BUILD_SHARED_LIBS ${_bsl_save} CACHE BOOL "" FORCE)

# FLAC's CMakeLists adds -D_FORTIFY_SOURCE=2 (via WITH_FORTIFY_SOURCE=ON by
# default), but glibc's features.h rejects that at -O0 and emits a #warning
# on every FLAC .c file when SoDaRadio is built in Debug (our default).
# Single-config generators bake CMAKE_BUILD_TYPE globally, so we can't flip
# just FLAC's build type to Release -- apply optimization directly to the
# FLAC target instead.  FLAC is a leaf dep we don't debug ourselves, so
# forcing -O2 on it is harmless.
IF(TARGET FLAC)
  target_compile_options(FLAC PRIVATE -O2)
ENDIF()

# Publish the same variables a system pkg_check_modules(FLAC flac) would set.
# qtgui/CMakeLists.txt and shortgui/CMakeLists.txt drop FLAC_INCLUDE_DIRS into
# directory-level include_directories() calls, which don't participate in
# target-level INTERFACE_INCLUDE_DIRECTORIES propagation -- set the source
# include path explicitly.  FLAC headers live under include/FLAC/ in the
# source tree.
set(FLAC_FOUND        TRUE)
set(FLAC_VERSION      "${FLAC_TAG} (fetched)")
set(FLAC_LIBRARIES    FLAC)
set(FLAC_INCLUDE_DIRS "${flac_SOURCE_DIR}/include")
