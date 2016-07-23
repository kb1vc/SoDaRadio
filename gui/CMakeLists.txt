

########### next target ###############

#build the GUI from the generated sources from wxFormBuilder
#  SoDaRadio_GUI.cpp
#execute_process(COMMAND /bin/bash ${PROJECT_SOURCE_DIR}/gui/adjust_GUI.sh ${PROJECT_SOURCE_DIR}/gui ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR})

ADD_CUSTOM_COMMAND(OUTPUT
  ${PROJECT_BINARY_DIR}/SoDaRadio_GUI.cxx
  COMMAND /bin/bash ${PROJECT_SOURCE_DIR}/gui/adjust_GUI.sh ${PROJECT_SOURCE_DIR}/gui ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}  
  DEPENDS ${PROJECT_SOURCE_DIR}/gui/SoDaRadio_GUI.cpp ${PROJECT_SOURCE_DIR}/gui/adjust_GUI.sh )

set(SoDaRadio_SRCS
    SoDaRadio_App.cxx
    ${PROJECT_BINARY_DIR}/SoDaRadio_GUI.cxx
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
    ../src/Debug.cxx
    )


find_package(wxWidgets COMPONENTS core base richtext aui xrc qa html adv  )
include(${wxWidgets_USE_FILE})
  
add_executable(SoDaRadio ${SoDaRadio_SRCS})

include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${wxWidgets_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/gui)

target_compile_options(SoDaRadio BEFORE PUBLIC -Wno-deprecated-declarations)

target_link_libraries(SoDaRadio m rt ${Boost_LIBRARIES} ${wxWidgets_LIBRARIES} )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

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
