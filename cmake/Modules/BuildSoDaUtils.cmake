# cmake/Modules/BuildSoDaUtils.cmake
#
# Locate or fetch SoDaLibs (SoDaUtils + SoDaSignals) and FFTW3.
#
# User-settable options / cache variables:
#   BUILD_SODALIBS   OFF  Use the installed SoDaLibs; ON forces a fetch.
#   SODALIBS_TAG          Git tag to use when fetching (default v_U5.3.0_S4.4.0).
#
# When fetching, both SoDaLibs and FFTW3 (if absent) are built as static
# libraries and linked into the SoDaRadio binaries.

option(BUILD_SODALIBS
  "Fetch and build SoDaLibs from GitHub instead of using the installed copy" OFF)

set(SODALIBS_TAG "v_U5.3.0_S4.4.0" CACHE STRING
  "SoDaLibs git tag to fetch when BUILD_SODALIBS=ON or SoDaLibs is not installed")

# -----------------------------------------------------------------------
# Decide: use installed copy or fetch?
# -----------------------------------------------------------------------
if(NOT BUILD_SODALIBS)
  find_package(SoDaUtils QUIET CONFIG)
endif()

if(BUILD_SODALIBS OR NOT SoDaUtils_FOUND)

  if(BUILD_SODALIBS)
    message(STATUS "BUILD_SODALIBS=ON -- fetching SoDaLibs ${SODALIBS_TAG} from GitHub")
  else()
    message(STATUS "SoDaUtils not installed -- fetching SoDaLibs ${SODALIBS_TAG} from GitHub")
  endif()

  include(FetchContent)

  # Directory for stub package-config files (see below)
  set(_soda_stubs "${CMAKE_BINARY_DIR}/_sodalibs_stubs")

  # -----------------------------------------------------------------------
  # FFTW3 (float variant required by SoDaSignals)
  # -----------------------------------------------------------------------
  find_library(FFTW3F_LIB  NAMES fftw3f)
  find_path   (FFTW3_INCLUDE_DIR NAMES fftw3.h)

  if(FFTW3F_LIB AND FFTW3_INCLUDE_DIR)
    message(STATUS "Using system FFTW3f: ${FFTW3F_LIB}")
    set(_fftw_lib "${FFTW3F_LIB}")
    set(_fftw_inc "${FFTW3_INCLUDE_DIR}")
  else()
    message(STATUS "FFTW3f not found -- fetching fftw-3.3.11 from fftw.org")
    # Suppress CMake 3.24+ timestamp policy warning
    if(POLICY CMP0135)
      cmake_policy(SET CMP0135 NEW)
    endif()
    FetchContent_Declare(
      fftw3
      URL https://fftw.org/fftw-3.3.11.tar.gz
    )
    # Build only the float (fftw3f) variant, static
    set(ENABLE_FLOAT   ON  CACHE BOOL "" FORCE)
    set(ENABLE_SHARED  OFF CACHE BOOL "" FORCE)
    set(ENABLE_THREADS OFF CACHE BOOL "" FORCE)
    set(ENABLE_OPENMP  OFF CACHE BOOL "" FORCE)
    set(BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(fftw3)
    # fftw3's CMake creates a target named 'fftw3f' when ENABLE_FLOAT=ON
    set(_fftw_lib fftw3f)
    set(_fftw_inc "${fftw3_BINARY_DIR};${fftw3_SOURCE_DIR}/api")
  endif()

  # Create SoDa_FFTW::Float so sodasignals can link against it.
  # SoDaLibs' own find_package(SoDa_FFTW) would normally create this target;
  # we create it here so the stub config (below) can skip the real search.
  if(NOT TARGET SoDa_FFTW_Float)
    add_library(SoDa_FFTW_Float INTERFACE)
    target_include_directories(SoDa_FFTW_Float INTERFACE ${_fftw_inc})
    target_link_libraries     (SoDa_FFTW_Float INTERFACE ${_fftw_lib})
  endif()
  if(NOT TARGET SoDa_FFTW::Float)
    add_library(SoDa_FFTW::Float ALIAS SoDa_FFTW_Float)
  endif()

  # Stub config: SoDaLibs' find_package(SoDa_FFTW) finds this and stops searching.
  # The SoDa_FFTW::Float target is already live in the build, so no redefinition needed.
  file(MAKE_DIRECTORY "${_soda_stubs}/share/cmake/SoDa_FFTW")
  file(WRITE "${_soda_stubs}/share/cmake/SoDa_FFTW/SoDa_FFTWConfig.cmake"
    "set(SoDa_FFTW_FOUND TRUE)\nset(SoDa_FFTW_FLOAT_LIB_FOUND TRUE)\n")

  # Prepend stubs dir so find_package picks it up before real system locations
  list(PREPEND CMAKE_PREFIX_PATH "${_soda_stubs}")

  # -----------------------------------------------------------------------
  # SoDaLibs (SoDaUtils + SoDaSignals)
  # -----------------------------------------------------------------------
  FetchContent_Declare(
    SoDaLibs
    GIT_REPOSITORY https://github.com/kb1vc/SoDaLibs.git
    GIT_TAG        ${SODALIBS_TAG}
    GIT_SHALLOW    TRUE
  )

  # Force static libs inside SoDaLibs; suppress its tests and docs
  set(_bsl_save     ${BUILD_SHARED_LIBS})
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
  set(BUILD_TESTING     OFF CACHE BOOL "" FORCE)
  set(DISABLE_DOXYGEN   ON  CACHE BOOL "" FORCE)

  FetchContent_MakeAvailable(SoDaLibs)

  set(BUILD_SHARED_LIBS ${_bsl_save} CACHE BOOL "" FORCE)

  # SoDaLibs/Utils/src/CMakeLists.txt already creates:
  #   add_library(SoDaUtils::sodautils ALIAS sodautils)
  # SoDaLibs/Signals/src/CMakeLists.txt already creates:
  #   add_library(SoDaSignals::sodasignals ALIAS sodasignals)
  # Both targets are now available to SoDaRadio's subdirectories.

  # Write stub package configs so find_package(SoDaUtils/SoDaSignals) calls
  # in src/CMakeLists.txt succeed without looking for installed config files.
  foreach(_pkg SoDaUtils SoDaSignals)
    file(MAKE_DIRECTORY "${_soda_stubs}/share/cmake/${_pkg}")
    file(WRITE "${_soda_stubs}/share/cmake/${_pkg}/${_pkg}Config.cmake"
      "set(${_pkg}_FOUND TRUE)\n")
  endforeach()

else()
  # -----------------------------------------------------------------------
  # Installed path: verify SoDaUtils is static
  # -----------------------------------------------------------------------
  get_target_property(_t SoDaUtils::sodautils TYPE)
  if(NOT _t STREQUAL "STATIC_LIBRARY")
    message(FATAL_ERROR "SoDaUtils::sodautils: expected STATIC_LIBRARY, got ${_t}")
  endif()
  message(STATUS "Using installed SoDaUtils")
endif()
