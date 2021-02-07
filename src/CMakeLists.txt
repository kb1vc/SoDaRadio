
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
)


find_package(FFTW3f REQUIRED QUIET)

set(SoDaServer_SRCS ${SoDaServer_SRCS} AudioQtRX.cxx)
IF(ALSA_FOUND)
  add_definitions(-DHAVE_LIBASOUND=1)
  set(SoDaServer_SRCS ${SoDaServer_SRCS} AudioQtRXTX.cxx)
  message("ALSA enabled -- TX is enabled in this configuration.")
ELSE()
  message("ALSA is disabled/not available -- TX is disabled in this configuration.")  
ENDIF()


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

include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${Radio_INCLUDE_DIRS} ${ALSA_INCLUDE_DIRS} ${FFTW3F_INCLUDE_DIRS} ${SoDaFormat_INCLUDE_DIR} )



add_executable(SoDaServer ${SoDaServer_SRCS})


target_link_libraries(SoDaServer ${RT_LIB}
  Threads::Threads
  sodaformat
    ${Boost_LIBRARIES}  ${Radio_LIBRARIES} ${ALSA_LIBRARIES} ${FFTW3F_LIBRARIES} ${CMAKE_DL_LIBS})


install(TARGETS SoDaServer DESTINATION bin)

set(simple_acc_SRCS
SimpleAccessory.cxx
SoDaThread.cxx
SoDaBase.cxx
Debug.cxx
SoDaThreadRegistry.cxx
)

add_library(accessory SHARED ${simple_acc_SRCS})
target_link_libraries(accessory ${Boost_LIBRARIES})


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


getListOfVarsStartingWith("[sS]" vars)
foreach (v IN LISTS vars)
  message("${v} = ${${v}}")
endforeach()
