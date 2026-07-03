# cmake/Modules/BuildSoDaUtils.cmake
#
# Locate or fetch SoDaLibs (SoDaUtils + SoDaSignals) and FFTW3.
#
# User-visible options (declared in the top-level CMakeLists.txt):
#   BUILD_SODALIBS  OFF  Use installed SoDaLibs; ON forces a fetch from GitHub.
#   BUILD_FFTW      OFF  Use system FFTW3;    ON forces a build from fftw.org.
#   SODALIBS_TAG         SoDaLibs git tag (default v_U5.3.0_S4.4.0).
#
# Behavior:
#   * If BUILD_SODALIBS is OFF and SoDaUtils is installed, use the installed
#     copy.  BUILD_FFTW is ignored in this case (the installed SoDaSignals is
#     already linked against whatever FFTW it was built with).
#   * Otherwise SoDaLibs is cloned from GitHub at SODALIBS_TAG and built as
#     a static library via FetchContent.  FFTW3 is either found on the host
#     or, when BUILD_FFTW=ON or fftw3f is not installed, fetched from
#     https://fftw.org/fftw-3.3.11.tar.gz and built as a static library.

# -----------------------------------------------------------------------
# Should we use an installed SoDaLibs, or fetch and build one?
# -----------------------------------------------------------------------
if(NOT BUILD_SODALIBS)
  find_package(SoDaUtils QUIET CONFIG)
endif()

if(NOT (BUILD_SODALIBS OR NOT SoDaUtils_FOUND))
  # Installed path: verify SoDaUtils is static and we're done.
  get_target_property(_t SoDaUtils::sodautils TYPE)
  if(NOT _t STREQUAL "STATIC_LIBRARY")
    message(FATAL_ERROR
      "SoDaUtils::sodautils: expected STATIC_LIBRARY, got ${_t}")
  endif()
  message(STATUS "Using installed SoDaUtils")
  return()
endif()

# -----------------------------------------------------------------------
# Fetch path
# -----------------------------------------------------------------------
if(BUILD_SODALIBS)
  message(STATUS "BUILD_SODALIBS=ON -- fetching SoDaLibs ${SODALIBS_TAG} from GitHub")
else()
  message(STATUS "SoDaUtils not installed -- fetching SoDaLibs ${SODALIBS_TAG} from GitHub")
endif()

include(FetchContent)

# -----------------------------------------------------------------------
# FFTW3 (float variant required by SoDaSignals)
# -----------------------------------------------------------------------
set(_have_system_fftw FALSE)
if(NOT BUILD_FFTW)
  find_library(FFTW3F_LIB NAMES fftw3f)
  find_path   (FFTW3F_INC NAMES fftw3.h)
  if(FFTW3F_LIB AND FFTW3F_INC)
    set(_have_system_fftw TRUE)
  endif()
endif()

if(_have_system_fftw)
  message(STATUS "Using system FFTW3f: ${FFTW3F_LIB}")
  set(_fftw_lib "${FFTW3F_LIB}")
  set(_fftw_inc "${FFTW3F_INC}")
else()
  if(BUILD_FFTW)
    message(STATUS "BUILD_FFTW=ON -- fetching fftw-3.3.11 from fftw.org")
  else()
    message(STATUS "FFTW3f not installed -- fetching fftw-3.3.11 from fftw.org")
  endif()
  # Silence CMake 3.24+ policy warning about tarball timestamps
  if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
  endif()
  FetchContent_Declare(
    fftw3
    URL https://fftw.org/fftw-3.3.11.tar.gz
    EXCLUDE_FROM_ALL
  )
  # Build float variant only, static
  set(ENABLE_FLOAT   ON  CACHE BOOL "" FORCE)
  set(ENABLE_SHARED  OFF CACHE BOOL "" FORCE)
  set(ENABLE_THREADS OFF CACHE BOOL "" FORCE)
  set(ENABLE_OPENMP  OFF CACHE BOOL "" FORCE)
  set(BUILD_TESTS    OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(fftw3)
  # FFTW3's CMake creates a target named 'fftw3f' when ENABLE_FLOAT=ON.
  set(_fftw_lib fftw3f)
  set(_fftw_inc "${fftw3_BINARY_DIR};${fftw3_SOURCE_DIR}/api")
endif()

# Create SoDa_FFTW::Float -- SoDaSignals links against this exact name.
# INTERFACE IMPORTED GLOBAL is visible to FetchContent's sub-add_subdirectory.
if(NOT TARGET SoDa_FFTW::Float)
  add_library(SoDa_FFTW::Float INTERFACE IMPORTED GLOBAL)
  set_target_properties(SoDa_FFTW::Float PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${_fftw_inc}"
    INTERFACE_LINK_LIBRARIES      "${_fftw_lib}")
endif()

# SoDaLibs' top-level CMakeLists does FIND_PACKAGE(SoDa_FFTW) to decide whether
# to build Signals.  Write a stub config that just sets _FOUND=TRUE so the
# find_package succeeds without redefining our SoDa_FFTW::Float target.
set(_soda_stubs "${CMAKE_BINARY_DIR}/_sodalibs_stubs")
file(MAKE_DIRECTORY "${_soda_stubs}/share/cmake/SoDa_FFTW")
file(WRITE "${_soda_stubs}/share/cmake/SoDa_FFTW/SoDa_FFTWConfig.cmake"
  "set(SoDa_FFTW_FOUND TRUE)\nset(SoDa_FFTW_FLOAT_LIB_FOUND TRUE)\n")
list(PREPEND CMAKE_PREFIX_PATH "${_soda_stubs}")

# -----------------------------------------------------------------------
# SoDaLibs (Utils + Signals)
# -----------------------------------------------------------------------
FetchContent_Declare(
  SoDaLibs
  GIT_REPOSITORY https://github.com/kb1vc/SoDaLibs
  GIT_TAG        ${SODALIBS_TAG}
  GIT_SHALLOW    TRUE
  EXCLUDE_FROM_ALL
)

# Force static libs and suppress SoDaLibs' tests / docs during the sub-build.
set(_bsl_save ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING     OFF CACHE BOOL "" FORCE)
set(DISABLE_DOXYGEN   ON  CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(SoDaLibs)

set(BUILD_SHARED_LIBS ${_bsl_save} CACHE BOOL "" FORCE)

# After the fetch, SoDaLibs has created:
#   SoDaUtils::sodautils    (ALIAS in Utils/src/CMakeLists.txt)
#   SoDaSignals::sodasignals (ALIAS in Signals/src/CMakeLists.txt)
# Both are visible to SoDaRadio's subdirectories via their aliases.
