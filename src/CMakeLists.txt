# Substantial parts written or modified by Claude Sonnet 4.6 (claude-sonnet-4-6)
# If SoDaLibs was fetched via BuildSoDaUtils.cmake the target already exists;
# otherwise pull it in from the installed package.
if(NOT TARGET SoDaSignals::sodasignals)
  find_package(SoDaSignals REQUIRED CONFIG)
endif()
get_target_property(_sig_type SoDaSignals::sodasignals TYPE)
if(NOT _sig_type STREQUAL "STATIC_LIBRARY")
  message(FATAL_ERROR "SoDaSignals::sodasignals: expected STATIC_LIBRARY, got ${_sig_type}")
endif()

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
    HilbertTransformer.cxx
    QuadratureOscillator.cxx
    Spectrogram.cxx
    CWGenerator.cxx
    UDSockets.cxx
    Debug.cxx
    SerialDev.cxx
    IPSockets.cxx
    IFRecorder.cxx
    RadioModels.cxx
    RadioControl.cxx
    RadioRX.cxx
    RadioTX.cxx
)


# If BuildSoDaUtils.cmake already set up SoDa_FFTW::Float (either from a
# fetched fftw3 build or from a discovered system copy), route the legacy
# FFTW3F_* variables through it instead of doing a second find_package that
# would rediscover the system library and cause a rpath conflict.
if(TARGET SoDa_FFTW::Float)
  get_target_property(FFTW3F_INCLUDE_DIRS SoDa_FFTW::Float INTERFACE_INCLUDE_DIRECTORIES)
  get_target_property(FFTW3F_LIBRARIES    SoDa_FFTW::Float INTERFACE_LINK_LIBRARIES)
else()
  find_package(FFTW3f REQUIRED QUIET)
endif()

set(SoDaServer_SRCS ${SoDaServer_SRCS} AudioQt.cxx)

IF(${HAVE_UHD})
  LIST(APPEND SoDaServer_SRCS
    USRPTRControl.cxx
    N200Control.cxx
    B200Control.cxx
    USRPRadio.cxx
    USRPCtrl.cxx
    USRPRX.cxx
    USRPTX.cxx
    USRPFrontEnd.cxx
    )
  LIST(APPEND Radio_INCLUDE_DIRS ${UHD_INCLUDE_DIRS})
  LIST(APPEND Radio_LIBRARIES ${UHD_LIBRARIES})
ENDIF()

add_definitions(-DHAVE_UHD=${HAVE_UHD})

IF(${HAVE_PLUTO})
  LIST(APPEND SoDaServer_SRCS PlutoCtrl.cxx PlutoRX.cxx PlutoTX.cxx PlutoRadio.cxx)
  LIST(APPEND Radio_INCLUDE_DIRS ${LIBIIO_INCLUDE_DIRS})
  LIST(APPEND Radio_LIBRARIES ${LIBIIO_LIBRARIES})
ENDIF()

add_definitions(-DHAVE_PLUTO=${HAVE_PLUTO})

IF(ENABLE_PLUTO_TX)
  add_definitions(-DPLUTO_HAS_TX)
ENDIF()

IF(${HAVE_RTLSDR})
  LIST(APPEND SoDaServer_SRCS RTLSDRCtrl.cxx RTLSDRRX.cxx RTLSDRTX.cxx RTLSDRRadio.cxx)
  LIST(APPEND Radio_INCLUDE_DIRS ${LIBRTLSDR_INCLUDE_DIRS})
  LIST(APPEND Radio_LIBRARIES ${LIBRTLSDR_LIBRARIES})
ENDIF()

add_definitions(-DHAVE_RTLSDR=${HAVE_RTLSDR})

include_directories(SYSTEM
  ${Radio_INCLUDE_DIRS}
  ${FFTW3F_INCLUDE_DIRS})
##  ${SoDaUtils_INCLUDE_DIR} )



add_executable(SoDaServer ${SoDaServer_SRCS})


target_link_libraries(SoDaServer ${RT_LIB}
  Threads::Threads
  SoDaSignals::sodasignals SoDaUtils::sodautils
  ${Radio_LIBRARIES} ${FFTW3F_LIBRARIES} ${CMAKE_DL_LIBS})


install(TARGETS SoDaServer DESTINATION bin)

set(simple_acc_SRCS
SimpleAccessory.cxx
SoDaThread.cxx
SoDaBase.cxx
Debug.cxx
SoDaThreadRegistry.cxx
)

add_library(accessory SHARED ${simple_acc_SRCS})
# accessory's sources #include <SoDa/*.hxx> via SoDaBase.hxx.  In the
# installed-SoDaLibs case the headers are already on the system include
# path; in the fetched case they live under the SoDaUtils target's
# INTERFACE_INCLUDE_DIRECTORIES.  Pull them in without adding a link
# dependency -- accessory is dlopened by SoDaServer with RTLD_GLOBAL,
# so the sodautils symbols it needs come from the main executable at
# runtime (and installed static libsodautils.a is often not -fPIC).
target_include_directories(accessory PRIVATE
  $<TARGET_PROPERTY:SoDaUtils::sodautils,INTERFACE_INCLUDE_DIRECTORIES>)

set(comm_mon_SRCS
CommandMonitor.cxx
SoDaThread.cxx
SoDaBase.cxx
Debug.cxx
SoDaThreadRegistry.cxx
)

add_library(CommandMonitor SHARED ${comm_mon_SRCS})
# Same rationale as accessory above: headers only, no link.
target_include_directories(CommandMonitor PRIVATE
  $<TARGET_PROPERTY:SoDaUtils::sodautils,INTERFACE_INCLUDE_DIRECTORIES>)



# Export header files that are required to build loadable modules
set(Loadable_INCLS
  SoDaBase.hxx
  SoDaThread.hxx
  UDSockets.hxx
  IPSockets.hxx  
  Command.hxx
  Debug.hxx
  )

install(FILES ${Loadable_INCLS} DESTINATION "include/SoDaRadio")

set(SoDaSockets_SRCS
  UDSockets.cxx
  IPSockets.cxx)

add_library(SoDaSockets STATIC ${SoDaSockets_SRCS})
# Same rationale as accessory above: headers only, no link.  Consumers
# of SoDaSockets (currently only SoDaServer) link SoDaUtils::sodautils
# themselves.
target_include_directories(SoDaSockets PRIVATE
  $<TARGET_PROPERTY:SoDaUtils::sodautils,INTERFACE_INCLUDE_DIRECTORIES>)
install(TARGETS SoDaSockets DESTINATION lib)


