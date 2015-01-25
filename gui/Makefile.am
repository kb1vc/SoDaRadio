AUTOMAKE_OPTIONS = subdir-objects
AM_CPPFLAGS = -I$(top_srcdir)/include
AM_BOOSTLIBFLAGS = ${BOOST_PROGRAM_OPTIONS_LIB} ${BOOST_THREAD_LIB} ${BOOST_DATE_TIME_LIB} ${BOOST_SYSTEM_LIB}

EXTRA_DIST = SoDaRadio_GUI.cpp uparrow.xpm downarrow.xpm SoDaLogo.xpm SoDaLogo_Big.xpm adjust_GUI.sh SoDaLogo_Big.png  SoDaSNA_GUI.cpp

SoDaRadio_GUI.cxx : SoDaRadio_GUI.cpp adjust_GUI.sh
	/bin/bash ./adjust_GUI.sh

SoDaSNA_GUI.cxx  : SoDaSNA_GUI.cpp adjust_GUI.sh
	/bin/bash ./adjust_GUI.sh

WX_CPPFLAGS = @WX_CPPFLAGS@
WX_CXXFLAGS = @WX_CXXFLAGS_ONLY@
WX_CFLAGS = @WX_CFLAGS_ONLY@
WX_LIBS = @WX_LIBS@

bin_PROGRAMS = SoDaRadio SoDaSNA

SoDaRadio_SOURCES = SoDaRadio_App.cxx SoDaRadio_GUI.cxx  SoDaRadio_Top.cxx SoDaRadio_Top_Events.cxx SoDaRadio_Top_Config.cxx ../src/UDSockets.cxx ../src/Command.cxx Navigation.cxx dem-gridlib.c xyplot.cxx RadioListenerThread.cxx GuiParams.cxx waterfall.cxx dem-gridlib.h GuiParams.hxx Navigation.hxx RadioListenerThread.hxx SoDaRadio_App.hxx SoDaRadio_GUI.h SoDaRadio_Top.h TunerDigit.hxx waterfall.hxx xyplot.hxx FindHome.hxx FindHome.cxx Default.soda_cfg.h SoDaRadio_Band.hxx ../src/Debug.hxx ../src/Debug.cxx 

SoDaRadio_CPPFLAGS = $(AM_CPPFLAGS) $(WX_CPPFLAGS) 
SoDaRadio_LDADD = $(AM_BOOSTLIBFLAGS) $(WX_LIBS)
SoDaRadio_CXXFLAGS = $(WX_CXXFLAGS) 
SoDaRadio_CFLAGS = -g
SoDaRadio_LDFLAGS = $(WX_LIBS) -lm -lrt


SoDaSNA_SOURCES = SoDaSNA_App.cxx SoDaSNA_GUI.cxx  SoDaSNA_Top.cxx SoDaSNA_Top_Events.cxx SoDaSNA_Top_Config.cxx ../src/UDSockets.cxx ../src/Command.cxx  xyplot.cxx SNAListenerThread.cxx GuiParams.cxx GuiParams.hxx  SNAListenerThread.hxx SoDaSNA_App.hxx SoDaSNA_GUI.h SoDaSNA_Top.h  xyplot.hxx FindHome.hxx FindHome.cxx ../src/Debug.hxx ../src/Debug.cxx SoDaSNA_Top_Calibrate.cxx

SoDaSNA_CPPFLAGS = $(AM_CPPFLAGS) $(WX_CPPFLAGS) 
SoDaSNA_LDADD = $(AM_BOOSTLIBFLAGS) $(WX_LIBS)
SoDaSNA_CXXFLAGS = $(WX_CXXFLAGS) 
SoDaSNA_CFLAGS = -g
SoDaSNA_LDFLAGS = $(WX_LIBS) -lm -lrt

