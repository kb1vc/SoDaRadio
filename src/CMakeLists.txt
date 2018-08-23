
########### next target ###############

set(SoDaServer_SRCS
    Params.cxx
    SoDaServer.cxx
    ProcInfo.cxx
    SoDaBase.cxx
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
    BaseBandRX.hxx
    BaseBandTX.hxx
    Command.hxx
    CWGenerator.hxx
    CWTX.hxx
    GPSmon.hxx
    HilbertTransformer.hxx
    IPSockets.hxx
    MultiMBox.hxx
    OSFilter.hxx
    Params.hxx
    QuadratureOscillator.hxx
    TDResamplers625x48.hxx
    TDResamplerTables625x48.hxx    
    ReSampler.hxx
    ReSamplers625x48.hxx
    SoDaBase.hxx
    SoDa_filter_tables.hxx
    SoDa_tx_filter_tables.hxx
    Spectrogram.hxx
    UDSockets.hxx
    UI.hxx
    AudioALSA.hxx
    AudioALSA.cxx
    AudioIfc.hxx
    MedianFilter.hxx
    AudioPA.hxx
    AudioPA.cxx
    Debug.hxx
    Debug.cxx
    SerialDev.cxx
    SerialDev.hxx
    TRControl.hxx
    TRControl.cxx
    N200Control.hxx
    N200Control.cxx
    IPSockets.hxx
    IPSockets.cxx
    B200Control.hxx
    B200Control.cxx
#    Demod.hxx
#    Demod.cxx
    PropTree.hxx
    )

find_package(Portaudio)
find_package(ALSA REQUIRED QUIET)
find_package(FFTW3f REQUIRED QUIET)

add_definitions(-DHAVE_LIBASOUND=1)
add_definitions(-DHAVE_UHD=${HAVE_UHD})

IF(GPSLIB_FOUND)
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
    ${Boost_LIBRARIES}  ${Radio_LIBRARIES} ${ALSA_LIBRARIES} ${FFTW3F_LIBRARIES})


install(TARGETS SoDaServer DESTINATION bin)

