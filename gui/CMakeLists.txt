

########### next target ###############

set(SoDaRadio_SRCS
    SoDaRadio_App.cxx
    SoDaRadio_GUI.cxx
    SoDaRadio_Top.cxx
    SoDaRadio_Top_Events.cxx
    SoDaRadio_Top_Config.cxx
    ../src/UDSockets.cxx
    ../src/Command.cxx
    Navigation.cxx
    dem-gridlib.c
    xyplot.cxx
    RadioListenerThread.cxx
    GuiParams.cxx
    waterfall.cxx
    dem-gridlib.h
    GuiParams.hxx
    Navigation.hxx
    RadioListenerThread.hxx
    SoDaRadio_App.hxx
    SoDaRadio_GUI.h
    SoDaRadio_Top.h
    TunerDigit.hxx
    waterfall.hxx
    xyplot.hxx
    FindHome.hxx
    FindHome.cxx
    Default.soda_cfg.h
    SoDaRadio_Band.hxx
    ../src/Debug.hxx
    ../src/Debug.cxx)

find_package(Boost 1.45 REQUIRED COMPONENTS program_options thread date_time system filesystem serialization regex)

find_package(wxWidgets COMPONENTS core base richtext aui xrc qa html adv  )
include(${wxWidgets_USE_FILE})
  
add_executable(SoDaRadio ${SoDaRadio_SRCS})

include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${wxWidgets_INCLUDE_DIRS})

target_link_libraries(SoDaRadio m rt ${Boost_LIBRARIES} ${wxWidgets_LIBRARIES} )

install(TARGETS SoDaRadio DESTINATION bin)


########### install files ###############




#original Makefile.am contents follow:

#AUTOMAKE_OPTIONS = subdir-objects
#AM_CPPFLAGS = -I$(top_srcdir)/include -Wno-deprecated-declarations
#AM_BOOSTLIBFLAGS = ${BOOST_PROGRAM_OPTIONS_LIB} ${BOOST_THREAD_LIB} ${BOOST_DATE_TIME_LIB} ${BOOST_SYSTEM_LIB}
#
#EXTRA_DIST = SoDaRadio_GUI.cpp uparrow.xpm downarrow.xpm SoDaLogo.xpm SoDaLogo_Big.xpm adjust_GUI.sh SoDaLogo_Big.png 
#
#SoDaRadio_GUI.cxx : SoDaRadio_GUI.cpp adjust_GUI.sh
#	/bin/bash ./adjust_GUI.sh 
#
#
#WX_CPPFLAGS = @WX_CPPFLAGS@
#WX_CXXFLAGS = @WX_CXXFLAGS_ONLY@
#WX_CFLAGS = @WX_CFLAGS_ONLY@
#WX_LIBS = @WX_LIBS@
#
#bin_PROGRAMS = SoDaRadio
#
#SoDaRadio_SOURCES = SoDaRadio_App.cxx SoDaRadio_GUI.cxx  SoDaRadio_Top.cxx SoDaRadio_Top_Events.cxx SoDaRadio_Top_Config.cxx ../src/UDSockets.cxx ../src/Command.cxx Navigation.cxx dem-gridlib.c xyplot.cxx RadioListenerThread.cxx GuiParams.cxx waterfall.cxx dem-gridlib.h GuiParams.hxx Navigation.hxx RadioListenerThread.hxx SoDaRadio_App.hxx SoDaRadio_GUI.h SoDaRadio_Top.h TunerDigit.hxx waterfall.hxx xyplot.hxx FindHome.hxx FindHome.cxx Default.soda_cfg.h SoDaRadio_Band.hxx ../src/Debug.hxx ../src/Debug.cxx
#
#SoDaRadio_CPPFLAGS = $(AM_CPPFLAGS) $(WX_CPPFLAGS) 
#SoDaRadio_LDADD = $(AM_BOOSTLIBFLAGS) $(WX_LIBS)
#SoDaRadio_CXXFLAGS = $(WX_CXXFLAGS) 
#SoDaRadio_CFLAGS = -g
#SoDaRadio_LDFLAGS = $(WX_LIBS) -lm -lrt
#
