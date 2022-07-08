
########### next target ###############
include(GNUInstallDirs)

set(SoDaServer_SRCS
  SoDaServer.cxx
  )

set(SoDaLib_SRCS  
    Params.cxx
    Buffer.cxx
    SoDaBase.cxx
    SoDaThread.cxx
    SoDaThreadRegistry.cxx    
    MailBoxRegistry.cxx
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

LIST(APPEND SoDaLib_SRCS AudioQtRX.cxx)
IF(ALSA_FOUND)
  add_definitions(-DHAVE_LIBASOUND=1)
  LIST(APPEND SoDaLib_SRCS AudioQtRXTX.cxx)
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
  LIST(APPEND SoDaLib_SRCS
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
  ${ALSA_INCLUDE_DIRS} 
  ${FFTW3F_INCLUDE_DIRS} 
  ${SoDaUtils_INCLUDE_DIR} )


add_library(SoDaLib ${SoDaLib_SRCS})

add_executable(SoDaServer ${SoDaServer_SRCS})
# Do this to allow plugins to see things like the thread and
# mailbox registrars.  (Otherwise they create their own and
# nothing good comes of it.)
set_property(TARGET SoDaServer PROPERTY ENABLE_EXPORTS 1)


target_link_libraries(SoDaServer ${RT_LIB}
  Threads::Threads
  SoDaLib
  ${SoDaUtils_LIBRARIES}
  ${Radio_LIBRARIES} ${ALSA_LIBRARIES} ${FFTW3F_LIBRARIES} ${CMAKE_DL_LIBS})


install(TARGETS SoDaServer DESTINATION bin)
install(TARGETS SoDaLib DESTINATION ${LIBDIR})
set(simple_acc_SRCS
SimpleAccessory.cxx
)

set(logmsg_SRCS
LogMsgPlugin.cxx  
)

add_library(logmsgplugin SHARED ${logmsg_SRCS})

#add_library(accessory SHARED ${simple_acc_SRCS})
#target_link_libraries(accessory ${SoDaUtils_LIBRARIES})


# Export header files that are required to build loadable modules
set(Loadable_INCLS
  SoDaBase.hxx
  SoDaThread.hxx
  MailBoxRegistry.cxx  
  UDSockets.hxx
  IPSockets.hxx  
  Command.hxx
  Buffer.hxx
  Debug.hxx
  )

install(FILES ${Loadable_INCLS} DESTINATION "include/SoDaRadio")

set(SoDaSockets_SRCS
  UDSockets.cxx
  IPSockets.cxx)

add_library(SoDaSockets STATIC ${SoDaSockets_SRCS})
install(TARGETS SoDaSockets DESTINATION lib)


