

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

  ### wxWidgets 3.0 is a trainwreck
set(wxWidgets_CONFIG_OPTIONS --version=2.8)
find_package(wxWidgets COMPONENTS core base richtext aui xrc qa html adv  QUIET)
include(${wxWidgets_USE_FILE})
  
add_executable(SoDaRadio ${SoDaRadio_SRCS})

include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${wxWidgets_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/gui)

target_compile_options(SoDaRadio BEFORE PUBLIC -Wno-deprecated-declarations)

target_link_libraries(SoDaRadio m rt ${Boost_LIBRARIES} ${wxWidgets_LIBRARIES} )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

install(TARGETS SoDaRadio DESTINATION bin)

