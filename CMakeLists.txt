cmake_minimum_required(VERSION 2.8)

cmake_policy(SET CMP0048 NEW)

project(SoDaRadio VERSION 4.0.0)

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
