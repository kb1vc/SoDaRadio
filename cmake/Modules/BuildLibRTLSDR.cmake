# cmake/Modules/BuildLibRTLSDR.cmake
#
# Fetch librtlsdr from GitHub and build it as a static library.
# Sets the same variables that pkg_check_modules(LIBRTLSDR ...) would.
#
# Consumers:  LIBRTLSDR_FOUND, LIBRTLSDR_VERSION,
#             LIBRTLSDR_INCLUDE_DIRS, LIBRTLSDR_LIBRARIES
# Config:     LIBRTLSDR_TAG (default v2.0.1)
#
# librtlsdr still needs libusb-1.0 at compile time.

set(LIBRTLSDR_TAG "v2.0.1" CACHE STRING
  "librtlsdr git tag to fetch when BUILD_LIBRTLSDR=ON")

message(STATUS "Fetching librtlsdr ${LIBRTLSDR_TAG} from GitHub")

include(FetchContent)

FetchContent_Declare(
  librtlsdr
  GIT_REPOSITORY https://github.com/librtlsdr/librtlsdr
  GIT_TAG        ${LIBRTLSDR_TAG}
  GIT_SHALLOW    TRUE
  EXCLUDE_FROM_ALL
  # librtlsdr's CMakeLists.txt uses ${CMAKE_SOURCE_DIR} in several places
  # (module path, include path, uninstall configure_file) where it should
  # be ${CMAKE_CURRENT_SOURCE_DIR}.  Under a top-level build the two are
  # the same; under FetchContent the former resolves to SoDaRadio's source
  # tree and everything breaks.  Rewrite it via a cmake -P script -- doing
  # this with sed hits a two-level quoting problem because FetchContent
  # stores the command string and re-expands it in the subbuild.
  PATCH_COMMAND ${CMAKE_COMMAND}
    -DFILE=<SOURCE_DIR>/CMakeLists.txt
    -P ${CMAKE_CURRENT_LIST_DIR}/patch_librtlsdr.cmake
)

set(_bsl_save ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(INSTALL_UDEV_RULES OFF CACHE BOOL "" FORCE)
set(DETACH_KERNEL_DRIVER OFF CACHE BOOL "" FORCE)
set(WITH_RTL_TCP        OFF CACHE BOOL "" FORCE)

# librtlsdr's cmake_minimum_required is older than CMake 4.x will accept.
# Relaxing the check via this variable does not enable any new policies for
# it; it only satisfies the version-floor sanity check.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# If libiio was fetched too, its top-level CMakeLists calls
# pkg_check_modules(LIBUSB libusb-1.0), which sets LIBUSB_FOUND=TRUE and
# populates LIBUSB_INCLUDE_DIRS (plural).  librtlsdr's FindLibUSB.cmake
# short-circuits when LIBUSB_FOUND is already set, so it never populates
# its own singular LIBUSB_INCLUDE_DIR -- and the include_directories()
# line downstream ends up empty, so librtlsdr.c fails to find libusb.h.
# Clear the cross-project flag before librtlsdr's CMakeLists runs.
unset(LIBUSB_FOUND)
unset(LIBUSB_FOUND CACHE)

FetchContent_MakeAvailable(librtlsdr)

set(BUILD_SHARED_LIBS ${_bsl_save} CACHE BOOL "" FORCE)

# librtlsdr unconditionally creates both 'rtlsdr_shared' (SHARED) and
# 'rtlsdr_static' (STATIC).  BUILD_SHARED_LIBS is ignored -- both are
# always built.  Pin to the static one so SoDaServer doesn't pick up
# librtlsdr.so.2 from the system link path.
#
# LIBRTLSDR_INCLUDE_DIRS is set explicitly rather than relying on target
# propagation -- src/CMakeLists.txt drops it into a directory-level
# include_directories() call, which doesn't pick up interface propagation.
set(LIBRTLSDR_FOUND        TRUE)
set(LIBRTLSDR_VERSION      "${LIBRTLSDR_TAG} (fetched)")
set(LIBRTLSDR_LIBRARIES    rtlsdr_static)
set(LIBRTLSDR_INCLUDE_DIRS "${librtlsdr_SOURCE_DIR}/include")
