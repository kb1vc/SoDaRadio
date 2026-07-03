# cmake/Modules/BuildLibIIO.cmake
#
# Fetch libiio from GitHub and build the float-free, USB+network backends
# as a static library.  Sets the same variables that pkg_check_modules
# would produce so the rest of the build tree doesn't have to know that
# the copy of libiio came from source rather than the host system.
#
# Consumers:  LIBIIO_FOUND, LIBIIO_VERSION, LIBIIO_INCLUDE_DIRS, LIBIIO_LIBRARIES
# Config:     LIBIIO_TAG (default v0.26)
#
# Note: libiio still needs libusb-1.0 and libxml2 at compile time -- we're
# only building libiio itself from source, not its transitive C dependencies.

set(LIBIIO_TAG "v0.26" CACHE STRING
  "libiio git tag to fetch when BUILD_LIBIIO=ON")

message(STATUS "Fetching libiio ${LIBIIO_TAG} from GitHub")

include(FetchContent)

FetchContent_Declare(
  libiio
  GIT_REPOSITORY https://github.com/analogdevicesinc/libiio
  GIT_TAG        ${LIBIIO_TAG}
  GIT_SHALLOW    TRUE
  EXCLUDE_FROM_ALL
)

# Force static + trim libiio down to what SoDaRadio uses (Pluto over USB).
set(_bsl_save ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(WITH_TESTS         OFF CACHE BOOL "" FORCE)
set(WITH_DOC           OFF CACHE BOOL "" FORCE)
set(WITH_MAN           OFF CACHE BOOL "" FORCE)
set(WITH_ZSTD          OFF CACHE BOOL "" FORCE)
set(HAVE_DNS_SD        OFF CACHE BOOL "" FORCE)
set(WITH_HWMON         OFF CACHE BOOL "" FORCE)
set(WITH_LOCAL_BACKEND OFF CACHE BOOL "" FORCE)
set(WITH_USB_BACKEND   ON  CACHE BOOL "" FORCE)
set(WITH_NETWORK_BACKEND ON  CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(libiio)

set(BUILD_SHARED_LIBS ${_bsl_save} CACHE BOOL "" FORCE)

# Publish the same variables pkg_check_modules(LIBIIO ...) would set.
# LIBIIO_LIBRARIES uses the target name; cmake propagates its
# INTERFACE_INCLUDE_DIRECTORIES automatically to consumers, so
# LIBIIO_INCLUDE_DIRS can be left empty.
set(LIBIIO_FOUND        TRUE)
set(LIBIIO_VERSION      "${LIBIIO_TAG} (fetched)")
set(LIBIIO_LIBRARIES    iio)
set(LIBIIO_INCLUDE_DIRS "")
