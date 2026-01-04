

SET(CMAKE_BUILD_TYPE "Debug")

SET(SoDaRadio_SRCS
  main_sodaradio.cpp
)
  
SET(SoDaRadio_Lib_SRCS
  mainwindow.cpp
  FreqLabel.cpp
  soda_logtable.cpp
  soda_wfall.cpp
  soda_spect.cpp
  soda_wfall_data.cpp
  soda_comboboxes.cpp
  RadioListener.cpp
  ../src/Command.cxx
  main_setup_top.cpp
  main_setup_mid.cpp
  main_setup_loggps.cpp
  main_setup_settings.cpp
  main_setup_status.cpp  
  main_setup_bandconfig.cpp
  main_setup_logeditor.cpp  
  main_setup_spect.cpp
  main_setup_hamlib.cpp  
  ../common/Navigation.hxx
  ../common/Navigation.cxx
  ../common/dem-gridlib.c
  ../common/dem-gridlib.h    
  ../common/GuiParams.cxx
  ../common/GuiParams.hxx
  HamlibServer.cpp
  HamlibHandler.cpp
  HamlibListener.cpp  
  AudioListener.cpp
  AudioRXListener.cpp
  AudioTXServer.cpp
  AudioRecorder.cpp
  Band.cpp
  BandMap.cpp
  )

SET(SoDaRadio_HDRS
  mainwindow.hpp
  FreqLabel.hpp
  soda_logtable.hpp
  soda_spect.hpp
  soda_wfall.hpp
  soda_comboboxes.hpp
  RadioListener.hpp
  HamlibServer.hpp
  HamlibHandler.hpp
  HamlibListener.hpp
  AudioRXListener.hpp
  AudioRecorder.hpp
  )

SET(SoDaRadio_NOMOC_HDRS
  soda_freq_scale_draw.hpp
  soda_plot_picker.hpp
  soda_wfall_data.hpp
  soda_wfall_picker.hpp
  ../common/GuiParams.hxx
  ../src/Command.hxx  
  Band.hpp
  BandMap.hpp
)

SET(SoDaRadio_UIS
  mainwindow.ui
  )

SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(SYSTEM 
  ${SNDFILE_INCLUDE_DIRS}  
  ${Qt5Widgets_INCLUDE_DIRS} 
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${QWT_INCLUDE_DIRS}
  ${SoDaUtils_INCLUDE_DIR}  
  ${CMAKE_CURRENT_BINARY_DIR})

ADD_DEFINITIONS(${Qt5Widgets_DEFINITIONS})

QT5_WRAP_CPP(SoDaRadio_hdr_moc ${SoDaRadio_HDRS})
QT5_WRAP_UI(SoDaRadio_form_moc ${SoDaRadio_UIS})

# ADD_LIBRARY(mainwindow ${SoDaRadio_hdr_moc} ${SoDaRadio_form_moc})

#QT5_USE_MODULES(mainwindow Widgets Core Gui)

ADD_LIBRARY(SoDaRadio_Lib STATIC 
  ${SoDaRadio_Lib_SRCS} 
  ${SoDaRadio_hdr_moc} 
  ${SoDaRadio_form_moc})

message("QWT_LIBRARIES = ${QWT_LIBRARIES}")
TARGET_LINK_LIBRARIES(SoDaRadio_Lib m ${RT_LIB} 
  Qt5::Widgets Qt5::PrintSupport
  Qt5::Core Qt5::Gui Qt5::Network Qt5::Multimedia
  Threads::Threads
  ${SoDaUtils_LIBRARIES}  
  ${SNDFILE_LIBRARIES}  
  ${QWT_LIBRARIES} )

ADD_EXECUTABLE(SoDaRadio ${SoDaRadio_SRCS})

TARGET_COMPILE_OPTIONS(SoDaRadio BEFORE PUBLIC -Wno-deprecated-declarations)

TARGET_LINK_LIBRARIES(SoDaRadio SoDaRadio_Lib
    m ${RT_LIB}
    Threads::Threads
    ${SNDFILE_LIBRARIES}
    sodautils
    )

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

# QT5_USE_MODULES(SoDaRadio Core Gui Widgets PrintSupport Network Multimedia)


INSTALL(TARGETS SoDaRadio DESTINATION bin)

