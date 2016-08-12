

########### next target ###############

set(SoDaServer_SRCS
    Params.cxx
    SoDaServer.cxx
    SoDaBase.cxx
    USRPCtrl.cxx
    USRPRX.cxx
    USRPTX.cxx
    CWTX.cxx
    BaseBandRX.cxx
    BaseBandTX.cxx
    UI.cxx
    Command.cxx
    OSFilter.cxx
    HilbertTransformer.cxx
    ReSampler.cxx
    ReSamplers625x48.cxx
    Spectrogram.cxx
    CWGenerator.cxx
    TSIP.cxx
    GPS_TSIPmon.cxx
    UDSockets.cxx
    BaseBandRX.hxx
    BaseBandTX.hxx
    Command.hxx
    CWGenerator.hxx
    CWTX.hxx
    GPS_TSIPmon.hxx
    HilbertTransformer.hxx
    IPSockets.hxx
    MultiMBox.hxx
    OSFilter.hxx
    Params.hxx
    QuadratureOscillator.hxx
    ReSampler.hxx
    ReSamplers625x48.hxx
    SoDaBase.hxx
    SoDa_filter_tables.hxx
    SoDa_tx_filter_tables.hxx
    Spectrogram.hxx
    TSIP.hxx
    UDSockets.hxx
    UI.hxx
    USRPCtrl.hxx
    USRPRX.hxx
    USRPTX.hxx
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
    Demod.hxx
    Demod.cxx
    PropTree.hxx
    FrontEnd.hxx)

#find_package(Portaudio)
find_package(ALSA REQUIRED)
find_package(UHD REQUIRED)
find_package(FFTW3f REQUIRED)

add_definitions(-DHAVE_LIBASOUND=1)

include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${UHD_INCLUDE_DIRS} ${ALSA_INCLUDE_DIRS} ${FFTW3F_INCLUDE_DIRS})

add_executable(SoDaServer ${SoDaServer_SRCS})

target_link_libraries(SoDaServer rt ${Boost_LIBRARIES} ${UHD_LIBRARIES} ${ALSA_LIBRARIES} ${FFTW3F_LIBRARIES})
  #Boost::program_options Boost::thread Boost::date_time Boost::system Boost::filesystem Boost::serialization Boost::regex)

install(TARGETS SoDaServer DESTINATION bin)


########### install files ###############




#original Makefile.am contents follow:

#AUTOMAKE_OPTIONS = subdir-objects
#AM_CPPFLAGS = -I$(top_srcdir)/src ${BOOST_CPPFLAGS}
#AM_BOOSTLIBFLAGS = ${BOOST_LDFLAGS} ${BOOST_PROGRAM_OPTIONS_LIB} ${BOOST_THREAD_LIB} ${BOOST_DATE_TIME_LIB} ${BOOST_SYSTEM_LIB} ${BOOST_FILESYSTEM_LIB} ${BOOST_SERIALIZATION_LIB} ${BOOST_REGEX_LIB}
#
#bin_PROGRAMS = SoDaServer
#
#SoDaServer_SOURCES = Params.cxx  SoDaServer.cxx SoDaBase.cxx USRPCtrl.cxx USRPRX.cxx USRPTX.cxx CWTX.cxx BaseBandRX.cxx BaseBandTX.cxx UI.cxx Command.cxx OSFilter.cxx HilbertTransformer.cxx  ReSampler.cxx ReSamplers625x48.cxx Spectrogram.cxx CWGenerator.cxx TSIP.cxx GPS_TSIPmon.cxx UDSockets.cxx BaseBandRX.hxx BaseBandTX.hxx Command.hxx CWGenerator.hxx CWTX.hxx GPS_TSIPmon.hxx HilbertTransformer.hxx IPSockets.hxx  MultiMBox.hxx OSFilter.hxx Params.hxx QuadratureOscillator.hxx ReSampler.hxx ReSamplers625x48.hxx SoDaBase.hxx SoDa_filter_tables.hxx SoDa_tx_filter_tables.hxx Spectrogram.hxx TSIP.hxx UDSockets.hxx UI.hxx USRPCtrl.hxx USRPRX.hxx USRPTX.hxx  AudioALSA.hxx AudioALSA.cxx AudioIfc.hxx MedianFilter.hxx AudioPA.hxx AudioPA.cxx Debug.hxx Debug.cxx SerialDev.cxx SerialDev.hxx TRControl.hxx TRControl.cxx N200Control.hxx N200Control.cxx IPSockets.hxx IPSockets.cxx B200Control.hxx B200Control.cxx Demod.hxx Demod.cxx PropTree.hxx FrontEnd.hxx
#
#SoDaServer_CPPFLAGS = $(AM_CPPFLAGS)  -g
#SoDaServer_LDADD = $(AM_BOOSTLIBFLAGS)  -lrt
#SoDaServer_CXXFLAGS = -g
#SoDaServer_LDFLAGS = -g 
#
