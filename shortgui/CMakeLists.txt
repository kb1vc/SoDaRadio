

SET(CMAKE_BUILD_TYPE "Debug")

SET(ShortSoDa_SRCS
  main_shortsoda.cpp
)

SET(ShortSoDa_Lib_SRCS
  mainwindow.cpp
  soda_spect.cpp
  main_setup_spect_controls.cpp
  ../gui_common/FreqLabel.cpp
  ../gui_common/soda_logtable.cpp
  ../gui_common/soda_wfall.cpp
  ../gui_common/soda_wfall_data.cpp
  ../gui_common/soda_comboboxes.cpp
  ../gui_common/RadioListener.cpp
  ../src/Command.cxx
  ../gui_common/main_setup_top.cpp
  ../gui_common/main_setup_mid.cpp
  ../gui_common/main_setup_loggps.cpp
  ../gui_common/main_setup_settings.cpp
  ../gui_common/main_setup_status.cpp
  ../gui_common/main_setup_bandconfig.cpp
  ../gui_common/main_setup_logeditor.cpp
  ../gui_common/main_setup_spect.cpp
  ../gui_common/main_setup_hamlib.cpp
  ../common/Navigation.hxx
  ../common/Navigation.cxx
  ../common/dem-gridlib.c
  ../common/dem-gridlib.h
  ../common/GuiParams.cxx
  ../common/GuiParams.hxx
  ../gui_common/HamlibServer.cpp
  ../gui_common/HamlibHandler.cpp
  ../gui_common/HamlibListener.cpp
  ../gui_common/AudioListener.cpp
  ../gui_common/AudioRXListener.cpp
  ../gui_common/AudioServer.cpp
  ../gui_common/AudioTXServer.cpp
  ../gui_common/AudioRecorder.cpp
  ../gui_common/Band.cpp
  ../gui_common/BandMap.cpp
  ../gui_common/VUMeter.cpp
  )

SET(ShortSoDa_HDRS
  mainwindow.hpp
  ../gui_common/FreqLabel.hpp
  ../gui_common/soda_logtable.hpp
  ../gui_common/soda_spect.hpp
  ../gui_common/soda_wfall.hpp
  ../gui_common/soda_comboboxes.hpp
  ../gui_common/RadioListener.hpp
  ../gui_common/HamlibServer.hpp
  ../gui_common/HamlibHandler.hpp
  ../gui_common/HamlibListener.hpp
  ../gui_common/AudioRXListener.hpp
  ../gui_common/AudioRecorder.hpp
  ../gui_common/VUMeter.hpp
  )

SET(ShortSoDa_NOMOC_HDRS
  ../gui_common/soda_freq_scale_draw.hpp
  ../gui_common/soda_plot_picker.hpp
  ../gui_common/soda_wfall_data.hpp
  ../gui_common/soda_wfall_picker.hpp
  ../common/GuiParams.hxx
  ../src/Command.hxx
  ../gui_common/Band.hpp
  ../gui_common/BandMap.hpp
)

SET(ShortSoDa_UIS
  ../gui_common/mainwindow.ui
  )

SET(ShortSoDa_QRC
  shortsoda_resources.qrc
  )

SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON)
SET(CMAKE_AUTORCC ON)
SET(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/../gui_common")
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(SYSTEM
  ${Qt6Widgets_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../gui_common
  ${QWT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}/vendor)

ADD_DEFINITIONS(${Qt6Widgets_DEFINITIONS})
IF(Qt6WebEngineWidgets_FOUND)
  ADD_DEFINITIONS(-DHAVE_QT6_WEBENGINE)
ENDIF()

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
  $<$<BOOL:${Qt6WebEngineWidgets_FOUND}>:Qt6::WebEngineWidgets>
  Threads::Threads
  SoDaUtils::sodautils
  ${QWT_LIBRARIES})

ADD_EXECUTABLE(SoDaRadio ${ShortSoDa_SRCS})

TARGET_COMPILE_OPTIONS(SoDaRadio BEFORE PUBLIC -Wno-deprecated-declarations)

TARGET_LINK_LIBRARIES(SoDaRadio ShortSoDa_Lib
  m ${RT_LIB}
  Threads::Threads
  SoDaUtils::sodautils
  )

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt6Widgets_EXECUTABLE_COMPILE_FLAGS}")

INSTALL(TARGETS SoDaRadio DESTINATION bin)
INSTALL(FILES ../gui_common/SoDaLogo_Big.png
        DESTINATION share/icons/hicolor/128x128/apps
        RENAME SoDaRadio.png)
INSTALL(FILES SoDaRadio.desktop
        DESTINATION share/applications)
