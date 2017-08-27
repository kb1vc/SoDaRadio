SET(CMAKE_BUILD_TYPE "Debug")

SET(SoDaRadioQT_SRCS
  main.cpp
  mainwindow.cpp
  freqlabel.cpp
  soda_logtable.cpp
  soda_wfall.cpp
  soda_spect.cpp
  soda_wfall_data.cpp
  soda_comboboxes.cpp
  soda_listener.cpp
  ../src/Command.cxx
  main_setup_top.cpp
  main_setup_mid.cpp
  main_setup_loggps.cpp
  main_setup_settings.cpp
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
  soda_hamlib_server.cpp
  soda_hamlib_handler.cpp
  soda_hamlib_listener.cpp  
  )

SET(SoDaRadioQT_HDRS
  mainwindow.hpp
  freqlabel.hpp
  soda_logtable.hpp
  soda_spect.hpp
  soda_wfall.hpp
  soda_comboboxes.hpp
  soda_listener.hpp
  soda_hamlib_server.hpp
  soda_hamlib_handler.hpp
  soda_hamlib_listener.hpp  
  )

SET(SoDaRadioQT_NOMOC_HDRS
  soda_freq_scale_draw.hpp
  soda_list_spinbox.hpp
  soda_plot_picker.hpp
  soda_wfall_data.hpp
  soda_wfall_picker.hpp
  ../common/GuiParams.hxx
  ../src/Command.hxx  
  soda_band.hpp
)

SET(SoDaRadioQT_UIS
  mainwindow.ui
  )

SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

message("QWT_INCLUDE_DIRS = [${QWT_INCLUDE_DIRS}]")
message("QWT_LIBRARIES = [${QWT_LIBRARIES}]")
INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${QWT_INCLUDE_DIRS})
ADD_DEFINITIONS(${Qt5Widgets_DEFINITIONS})

QT5_WRAP_CPP(SoDaRadioQT_hdr_moc ${SoDaRadioQT_HDRS})
QT5_WRAP_UI(SoDaRadioQT_form_moc ${SoDaRadioQT_UIS})

ADD_LIBRARY(mainwindow ${SoDaRadioQT_hdr_moc} ${SoDaRadioQT_form_moc})
QT5_USE_MODULES(mainwindow Widgets)
  
ADD_EXECUTABLE(SoDaRadioQT ${SoDaRadioQT_SRCS} mainwindow)

TARGET_COMPILE_OPTIONS(SoDaRadioQT BEFORE PUBLIC -Wno-deprecated-declarations)

TARGET_LINK_LIBRARIES(SoDaRadioQT m rt ${Boost_LIBRARIES} Qt5::Widgets Qt5::PrintSupport  ${QWT_LIBRARIES})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

QT5_USE_MODULES(SoDaRadioQT Core Gui Widgets PrintSupport Network Multimedia)


INSTALL(TARGETS SoDaRadioQT DESTINATION bin)

