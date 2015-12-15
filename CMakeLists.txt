cmake_minimum_required(VERSION 3.0)

cmake_policy(SET CMP0048 NEW)

project(SoDaRadio)
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
add_subdirectory(exp)


########### install files ###############




#original Makefile.am contents follow:

#AUTOMAKE_OPTIONS = subdir-objects
## Doxygen support
#include $(top_srcdir)/doxygen_aminclude.am
#
#EXTRA_DIST = Doxyfile saved_git_version.txt  \
#	doc/figs/Resample_RX_625to48.svg \
#	doc/figs/SoDa_Radio_RX_Audio.svg \
#	doc/figs/SoDa_Radio_RX_Signal_Path.svg \
#	doc/figs/SoDa_Radio_System.svg \
#	doc/figs/SoDa_Radio_Toplevel.svg \
#	doc/figs/SoDa_Radio_TX_Signal_Path.svg \
#	doc/figs/USRP_N200_HW.svg \
#	doc/pix/kw2t_de_kb1vc.png \
#	doc/pix/prod_un200-kit_01_md_crop.jpg \
#	doc/pix/prod_un200-kit_01_md.jpg \
#	doc/pix/Tuning_Dialog.png 
#
#
#ACLOCAL_AMFLAGS = -I ./m4
#SUBDIRS = src gui exp 
