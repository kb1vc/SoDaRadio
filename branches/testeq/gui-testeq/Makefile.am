AUTOMAKE_OPTIONS = subdir-objects
AM_CPPFLAGS = -I$(top_srcdir)/include
AM_BOOSTLIBFLAGS = ${BOOST_PROGRAM_OPTIONS_LIB} ${BOOST_THREAD_LIB} ${BOOST_DATE_TIME_LIB} ${BOOST_SYSTEM_LIB}

EXTRA_DIST = SoDaBench_GUI.cpp SoDaLogo.xpm SoDaLogo_Big.xpm adjust_GUI.sh SoDaLogo_Big.png 

SoDaBench_GUI.cxx : SoDaBench_GUI.cpp adjust_GUI.sh
	/bin/bash ./adjust_GUI.sh 


WX_CPPFLAGS = @WX_CPPFLAGS@
WX_CXXFLAGS = @WX_CXXFLAGS_ONLY@
WX_CFLAGS = @WX_CFLAGS_ONLY@
WX_LIBS = @WX_LIBS@

bin_PROGRAMS = SoDaBench 

SoDaBench_SOURCES = SoDaBench_App.cxx SoDaBench_GUI.cxx  SoDaBench_Top.cxx  ../src/UDSockets.cxx ../src/Command.cxx  ../gui/xyplot.cxx BenchListenerThread.cxx ../gui/waterfall.cxx  BenchListenerThread.hxx SoDaBench_App.hxx SoDaBench_GUI.h SoDaBench_Top.h ../gui/waterfall.hxx ../gui/xyplot.hxx ../gui/FindHome.hxx ../gui/FindHome.cxx  ../gui/GuiParams.hxx ../gui/GuiParams.cxx ../gui/GraphClient.hxx

SoDaBench_CPPFLAGS = $(AM_CPPFLAGS) $(WX_CPPFLAGS) 
SoDaBench_LDADD = $(AM_BOOSTLIBFLAGS) $(WX_LIBS)
SoDaBench_CXXFLAGS = $(WX_CXXFLAGS) 
SoDaBench_CFLAGS = -g
SoDaBench_LDFLAGS = $(WX_LIBS) -lm -lrt

