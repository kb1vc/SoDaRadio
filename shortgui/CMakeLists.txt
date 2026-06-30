

SET(CMAKE_BUILD_TYPE "Debug")

SET(ShortSoDa_SRCS
  main_shortsoda.cpp
)

SET(ShortSoDa_Lib_SRCS
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
  AudioServer.cpp
  AudioTXServer.cpp
  AudioRecorder.cpp
  Band.cpp
  BandMap.cpp
  )

SET(ShortSoDa_HDRS
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

SET(ShortSoDa_NOMOC_HDRS
  soda_freq_scale_draw.hpp
  soda_plot_picker.hpp
  soda_wfall_data.hpp
  soda_wfall_picker.hpp
  ../common/GuiParams.hxx
  ../src/Command.hxx
  Band.hpp
  BandMap.hpp
)

SET(ShortSoDa_UIS
  mainwindow.ui
  )

SET(ShortSoDa_QRC
  shortsoda_resources.qrc
  )

SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON)
SET(CMAKE_AUTORCC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(SYSTEM
  ${Qt6Widgets_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${QWT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR})

ADD_DEFINITIONS(${Qt6Widgets_DEFINITIONS})

QT6_WRAP_CPP(ShortSoDa_hdr_moc ${ShortSoDa_HDRS})
QT6_WRAP_UI(ShortSoDa_form_moc ${ShortSoDa_UIS})

ADD_LIBRARY(ShortSoDa_Lib STATIC
  ${ShortSoDa_Lib_SRCS}
  ${ShortSoDa_hdr_moc}
  ${ShortSoDa_form_moc}
  ${ShortSoDa_QRC})

TARGET_LINK_LIBRARIES(ShortSoDa_Lib m ${RT_LIB}
  Qt6::Widgets Qt6::PrintSupport
  Qt6::Core Qt6::Gui Qt6::Network Qt6::Multimedia
  Threads::Threads
  SoDaUtils::sodautils
  ${QWT_LIBRARIES})

ADD_EXECUTABLE(ShortSoDa ${ShortSoDa_SRCS})

TARGET_COMPILE_OPTIONS(ShortSoDa BEFORE PUBLIC -Wno-deprecated-declarations)

TARGET_LINK_LIBRARIES(ShortSoDa ShortSoDa_Lib
  m ${RT_LIB}
  Threads::Threads
  SoDaUtils::sodautils
  )

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt6Widgets_EXECUTABLE_COMPILE_FLAGS}")

INSTALL(TARGETS ShortSoDa DESTINATION bin)
INSTALL(FILES SoDaLogo_Big.png
        DESTINATION share/icons/hicolor/128x128/apps
        RENAME ShortSoDa.png)
INSTALL(FILES ShortSoDa.desktop
        DESTINATION share/applications)
