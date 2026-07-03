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
)

set(_bsl_save ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(INSTALL_UDEV_RULES OFF CACHE BOOL "" FORCE)
set(DETACH_KERNEL_DRIVER OFF CACHE BOOL "" FORCE)
set(WITH_RTL_TCP        OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(librtlsdr)

set(BUILD_SHARED_LIBS ${_bsl_save} CACHE BOOL "" FORCE)

# librtlsdr's CMake creates both 'rtlsdr' (respecting BUILD_SHARED_LIBS)
# and, in newer versions, a separate 'rtlsdr_static' target.  With
# BUILD_SHARED_LIBS=OFF the 'rtlsdr' target is built static; use it.
set(LIBRTLSDR_FOUND        TRUE)
set(LIBRTLSDR_VERSION      "${LIBRTLSDR_TAG} (fetched)")
set(LIBRTLSDR_LIBRARIES    rtlsdr)
set(LIBRTLSDR_INCLUDE_DIRS "")
