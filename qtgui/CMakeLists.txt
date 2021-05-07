

SET(CMAKE_BUILD_TYPE "Debug")

SET(SoDaRadio_SRCS
  main_sodaradio.cpp
)
  
SET(SoDaRadio_Lib_SRCS
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
  soda_hamlib_server.cpp
  soda_hamlib_handler.cpp
  soda_hamlib_listener.cpp  
  soda_audio_listener.cpp
  )

SET(SoDaRadio_HDRS
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
  soda_audio_listener.hpp  
  )

SET(SoDaRadio_NOMOC_HDRS
  soda_freq_scale_draw.hpp
  soda_plot_picker.hpp
  soda_wfall_data.hpp
  soda_wfall_picker.hpp
  ../common/GuiParams.hxx
  ../src/Command.hxx  
  soda_band.hpp
)

SET(SoDaRadio_UIS
  mainwindow.ui
  )

SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIRS}
  ${SoDaUtils_INCLUDE_DIRS}}  
  ${SNDFILE_INCLUDE_DIRS}  
  ${Qt5Widgets_INCLUDE_DIRS} 
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${QWT_INCLUDE_DIRS} 
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

message("******\n\nGUI *****\nSoDaUtils_INCLUDE_DIRS = ${SoDaUtils_INCLUDE_DIRS}")
message("QWT_LIBRARIES = ${QWT_LIBRARIES}")
TARGET_LINK_LIBRARIES(SoDaRadio_Lib m ${RT_LIB} ${Boost_LIBRARIES}
    ${SoDaUtils_LIBRARIES}      
  Qt5::Widgets Qt5::PrintSupport
  Qt5::Core Qt5::Gui Qt5::Network Qt5::Multimedia
  Threads::Threads
  ${SNDFILE_LIBRARIES}  
  ${QWT_LIBRARIES} )

TARGET_INCLUDE_DIRECTORIES(SoDaRadio_Lib PUBLIC ${SoDaUtils_INCLUDE_DIRS})

ADD_EXECUTABLE(SoDaRadio ${SoDaRadio_SRCS})

TARGET_COMPILE_OPTIONS(SoDaRadio BEFORE PUBLIC -Wno-deprecated-declarations)

TARGET_LINK_LIBRARIES(SoDaRadio SoDaRadio_Lib
    m ${RT_LIB}
    Threads::Threads
    ${SoDaUtils_LIBRARIES}    
    ${SNDFILE_LIBRARIES}
    ${Boost_LIBRARIES}  )

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

# QT5_USE_MODULES(SoDaRadio Core Gui Widgets PrintSupport Network Multimedia)


INSTALL(TARGETS SoDaRadio DESTINATION bin)

