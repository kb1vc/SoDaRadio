AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -I$(top_srcdir)/include
AM_BOOSTLIBFLAGS = ${BOOST_PROGRAM_OPTIONS_LIB} ${BOOST_THREAD_LIB} ${BOOST_DATE_TIME_LIB} ${BOOST_SYSTEM_LIB}

EXTRA_DIST = SoDaRadio_GUI.cpp uparrow.xpm downarrow.xpm SoDaLogo.xpm SoDaLogo_Big.xpm adjust_GUI.sh SoDaLogo_Big.png

SoDaRadio_GUI.cxx : SoDaRadio_GUI.cpp adjust_GUI.sh
	/bin/bash ./adjust_GUI.sh 

WX_CPPFLAGS = @WX_CPPFLAGS@
WX_CXXFLAGS = @WX_CXXFLAGS_ONLY@
WX_CFLAGS = @WX_CFLAGS_ONLY@
WX_LIBS = @WX_LIBS@

bin_PROGRAMS = SoDaRadio

SoDaRadio_SOURCES = SoDaRadio_App.cxx SoDaRadio_GUI.cxx  SoDaRadio_Top.cxx SoDaRadio_Top_Events.cxx SoDaRadio_Top_Config.cxx ../src/UDSockets.cxx ../src/Command.cxx Navigation.cxx dem-gridlib.c xyplot.cxx RadioListenerThread.cxx GuiParams.cxx waterfall.cxx dem-gridlib.h GuiParams.hxx Navigation.hxx RadioListenerThread.hxx SoDaRadio_App.hxx SoDaRadio_GUI.h SoDaRadio_Top.h TunerDigit.hxx waterfall.hxx xyplot.hxx 

SoDaRadio_CPPFLAGS = $(AM_CPPFLAGS) $(WX_CPPFLAGS) 
SoDaRadio_LDADD = $(AM_BOOSTLIBFLAGS) $(WX_LIBS)
SoDaRadio_CXXFLAGS = $(WX_CXXFLAGS) 
SoDaRadio_CFLAGS = -g
SoDaRadio_LDFLAGS = $(WX_LIBS) -lm -lrt

