

########### next target ###############

#build the GUI from the generated sources from wxFormBuilder
#  SoDaRadio_GUI.cpp

ADD_CUSTOM_COMMAND(OUTPUT
  ${PROJECT_BINARY_DIR}/SoDaRadio_GUI.cxx
  COMMAND /bin/bash ${CMAKE_CURRENT_SOURCE_DIR}/adjust_GUI.sh ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}  
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/SoDaRadio_GUI.cpp ${CMAKE_CURRENT_SOURCE_DIR}/adjust_GUI.sh )

set(SoDaRadio_SRCS
    SoDaRadio_App.cxx
    ${PROJECT_BINARY_DIR}/SoDaRadio_GUI.cxx
    SoDaRadio_Top.cxx
    SoDaRadio_Top_Events.cxx
    SoDaRadio_Top_Config.cxx
    ../src/UDSockets.cxx
    ../src/Command.cxx
    xyplot.cxx
    RadioListenerThread.cxx
    waterfall.cxx
    WXGuiParams.hxx    
    RadioListenerThread.hxx
    SoDaRadio_App.hxx
    SoDaRadio_GUI.h
    SoDaRadio_Top.h
    TunerDigit.hxx
    waterfall.hxx
    xyplot.hxx
    ../src/FindHome.hxx
    ../src/FindHome.cxx
    Default.soda_cfg.h
    ../src/Debug.hxx
    ../src/Debug.cxx
    ../common/Navigation.hxx
    ../common/Navigation.cxx
    ../common/dem-gridlib.c
    ../common/dem-gridlib.h    
    ../common/GuiParams.cxx
    ../common/GuiParams.hxx    
    ../common/SoDaRadio_Band.hxx
    )

  
add_executable(SoDaRadioWX ${SoDaRadio_SRCS})

include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${wxWidgets_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(SoDaRadioWX BEFORE PUBLIC -Wno-deprecated-declarations)

target_link_libraries(SoDaRadioWX m rt ${Boost_LIBRARIES} ${wxWidgets_LIBRARIES} )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

install(TARGETS SoDaRadioWX DESTINATION bin)

