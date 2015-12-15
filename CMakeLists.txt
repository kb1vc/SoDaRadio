cmake_minimum_required(VERSION 3.0)

cmake_policy(SET CMP0048 NEW)

project(SoDaRadio)
set(SoDaRadio_VERSION_MAJOR 4)
set(SoDaRadio_VERSION_MINOR 0)
set(SoDaRadio_VERSION_PATCH "pre")

set(CPACK_PACKAGE_VERSION_MAJOR ${SoDaRadio_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${SoDaRadio_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${SoDaRadio_VERSION_PATCH})
set(CPACK_GENERATOR TGZ)
set(CPACK_SOURCE_GENERATOR TGZ)
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_SOURCE_IGNORE_FILES "/build/" "~$" "SoDa.soda_cfg$" "SoDa.soda_log$" "ckp$")

message("CPACK_PACKAGE_VERSION_MAJOR = ${CPACK_PACKAGE_VERSION_MAJOR}")
include(CPack)

add_definitions(-DGIT_VERSION="UNKNOWN")
add_definitions(-DSoDaRadio_VERSION="${SoDaRadio_VERSION}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

#include(MacroLibrary)

#include(ConvenienceLibs.cmake)

#include(ManualStuff.cmake)

#include(ConfigureChecks.cmake)


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
