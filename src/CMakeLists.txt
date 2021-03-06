
########### next target ###############

set(SoDaServer_SRCS
    Params.cxx
    SoDaServer.cxx
    SoDaBase.cxx
    SoDaThread.cxx
    SoDaThreadRegistry.cxx    
    CWTX.cxx
    BaseBandRX.cxx
    BaseBandTX.cxx
    UI.cxx
    Command.cxx
    OSFilter.cxx
    HilbertTransformer.cxx
    TDResamplerTables625x48.cxx
    ReSampler.cxx
    ReSamplers625x48.cxx
    Spectrogram.cxx
    CWGenerator.cxx
    GPSmon.cxx
    UDSockets.cxx
    AudioQt.cxx
    Debug.cxx
    SerialDev.cxx
    TRControl.cxx
    N200Control.cxx
    IPSockets.cxx
    B200Control.cxx
    IFRecorder.cxx
    fix_gpsd_ugliness.cxx
    )

find_package(ALSA REQUIRED QUIET)
find_package(FFTW3f REQUIRED QUIET)

add_definitions(-DHAVE_LIBASOUND=1)
add_definitions(-DHAVE_UHD=${HAVE_UHD})

IF(HAVE_GPSLIB)
  ADD_DEFINITIONS(-DHAVE_GPSLIB=1)
  LIST(APPEND Radio_INCLUDE_DIRS ${GPSLIB_INCLUDE_DIRS})
  LIST(APPEND Radio_LIBRARIES ${GPSLIB_LIBRARIES})
ENDIF()

IF(PORTAUDIO_FOUND)
  ADD_DEFINITIONS(-DHAVE_LIBPORTAUDIO=1)
  LIST(APPEND Radio_INCLUDE_DIRS ${PORTAUDIO_INCLUDE_DIRS})
  LIST(APPEND Radio_LIBRARIES ${PORTAUDIO_LIBRARIES})
ENDIF()

ADD_DEFINITIONS(-DUSE_PORTAUDIO=1)

IF(${HAVE_UHD})
  LIST(APPEND SoDaServer_SRCS
    USRPCtrl.cxx
    USRPRX.cxx
    USRPTX.cxx
    USRPFrontEnd.cxx
    )
  LIST(APPEND Radio_INCLUDE_DIRS ${UHD_INCLUDE_DIRS})
  LIST(APPEND Radio_LIBRARIES ${UHD_LIBRARIES})
ENDIF()


include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${Radio_INCLUDE_DIRS} ${ALSA_INCLUDE_DIRS} ${FFTW3F_INCLUDE_DIRS})


add_executable(SoDaServer ${SoDaServer_SRCS})

target_link_libraries(SoDaServer rt
    Threads::Threads
    ${Boost_LIBRARIES}  ${Radio_LIBRARIES} ${ALSA_LIBRARIES} ${FFTW3F_LIBRARIES} ${CMAKE_DL_LIBS})


install(TARGETS SoDaServer DESTINATION bin)

add_library(accessory SHARED SimpleAccessory.cxx)

# Export header files that are required to build loadable modules
set(Loadable_INCLS
  SoDaBase.hxx
  SoDaThread.hxx
  UDSockets.hxx
  IPSockets.hxx  
  Command.hxx
  MultiMBox.hxx
  Debug.hxx
  )

install(FILES ${Loadable_INCLS} DESTINATION "include/SoDaRadio")

set(SoDaSockets_SRCS
  UDSockets.cxx
  IPSockets.cxx)

add_library(SoDaSockets STATIC ${SoDaSockets_SRCS})
install(TARGETS SoDaSockets DESTINATION lib)
