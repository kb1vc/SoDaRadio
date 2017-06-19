
SET(SoDaRadioQT_SRCS
  main.cpp
  mainwindow.cpp
  freqlabel.cpp
  logtable.cpp
  )

SET(SoDaRadioQT_HDRS
  mainwindow.h
  freqlabel.h
  logtable.h
  )

SET(SoDaRadioQT_UIS
  mainwindow.ui
  )

SET(CMAKE_AUTOMOC ON)  
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
ADD_DEFINITIONS(${Qt5Widgets_DEFINITIONS})

QT5_WRAP_CPP(SoDaRadioQT_hdr_moc ${SoDaRadioQT_HDRS})
QT5_WRAP_UI(SoDaRadioQT_form_moc ${SoDaRadioQT_UIS})

ADD_LIBRARY(mainwindow ${SoDaRadioQT_hdr_moc} ${SoDaRadioQT_form_moc})
QT5_USE_MODULES(mainwindow Widgets)
  
ADD_EXECUTABLE(SoDaRadioQT ${SoDaRadioQT_SRCS} mainwindow)

TARGET_COMPILE_OPTIONS(SoDaRadioQT BEFORE PUBLIC -Wno-deprecated-declarations)

TARGET_LINK_LIBRARIES(SoDaRadioQT m rt ${Boost_LIBRARIES} Qt5::Widgets )

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

QT5_USE_MODULES(SoDaRadioQT Core Gui Widgets)


INSTALL(TARGETS SoDaRadioQT DESTINATION bin)

