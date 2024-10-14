
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
    Debug.cxx
    SerialDev.cxx
    TRControl.cxx
    N200Control.cxx
    IPSockets.cxx
    B200Control.cxx
    IFRecorder.cxx
    fix_gpsd_ugliness.cxx
    AudioQtRX.cxx
    AudioQtTX.cxx
)


find_package(FFTW3f REQUIRED QUIET)

IF(HAVE_GPSLIB)
  ADD_DEFINITIONS(-DHAVE_GPSLIB=1)
  LIST(APPEND Radio_INCLUDE_DIRS ${GPSLIB_INCLUDE_DIRS})
  LIST(APPEND Radio_LIBRARIES ${GPSLIB_LIBRARIES})
ENDIF()

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

add_definitions(-DHAVE_UHD=${HAVE_UHD})

include_directories(SYSTEM 
  ${Radio_INCLUDE_DIRS} 
  ${FFTW3F_INCLUDE_DIRS} 
  ${SoDaUtils_INCLUDE_DIR} )



add_executable(SoDaServer ${SoDaServer_SRCS})


target_link_libraries(SoDaServer ${RT_LIB}
  Threads::Threads
  ${SoDaUtils_LIBRARIES}
    ${Radio_LIBRARIES}  ${FFTW3F_LIBRARIES} ${CMAKE_DL_LIBS})


install(TARGETS SoDaServer DESTINATION bin)

set(simple_acc_SRCS
SimpleAccessory.cxx
SoDaThread.cxx
SoDaBase.cxx
Debug.cxx
SoDaThreadRegistry.cxx
)

add_library(accessory SHARED ${simple_acc_SRCS})
#target_link_libraries(accessory ${SoDaUtils_LIBRARIES})


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


