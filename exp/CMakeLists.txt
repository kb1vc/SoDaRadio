#find_package(Portaudio)
find_package(ALSA REQUIRED)
find_package(UHD REQUIRED)
find_package(FFTW3f REQUIRED)

add_definitions(-DHAVE_LIBASOUND=1)


find_package(Boost 1.45 REQUIRED COMPONENTS program_options thread date_time system filesystem serialization regex)

include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${UHD_INCLUDE_DIRS} ${ALSA_INCLUDE_DIRS} ${FFTW3F_INCLUDE_DIRS} ../src)

set(TEST_LIBS ${Boost_LIBRARIES} ${UHD_LIBRARIES} ${ALSA_LIBRARIES} ${FFTW3F_LIBRARIES})

########### next target ###############

set(FFT_Test_SRCS FFT_Test.cxx)

add_executable(FFT_Test ${FFT_Test_SRCS})

target_link_libraries(FFT_Test ${TEST_LIBS})

#install(TARGETS FFT_Test DESTINATION tests)


########### next target ###############

set(Sockets_Test_SRCS
    Sockets_Test.cxx
    ../src/IPSockets.cxx
    ../src/UDSockets.cxx
    ../src/IPSockets.hxx
    ../src/UDSockets.hxx)

add_executable(Sockets_Test ${Sockets_Test_SRCS})

target_link_libraries(Sockets_Test ${TEST_LIBS})

#install(TARGETS Sockets_Test DESTINATION tests)


########### next target ###############

set(MultiMBox_Test_SRCS MultiMBox_Test.cxx ../src/SoDaBase.cxx ../src/Debug.cxx)

add_executable(MultiMBox_Test ${MultiMBox_Test_SRCS})

target_link_libraries(MultiMBox_Test ${TEST_LIBS})

#install(TARGETS MultiMBox_Test DESTINATION tests)


########### next target ###############

set(SoDa_ThreadTest_SRCS SoDa_ThreadTest.cxx ../src/SoDaBase.cxx ../src/Debug.cxx)

add_executable(SoDa_ThreadTest ${SoDa_ThreadTest_SRCS})

target_link_libraries(SoDa_ThreadTest ${TEST_LIBS})

#install(TARGETS SoDa_ThreadTest DESTINATION tests)


########### next target ###############

set(OSFilter_Test_SRCS OSFilter_Test.cxx ../src/OSFilter.cxx)

add_executable(OSFilter_Test ${OSFilter_Test_SRCS})

target_link_libraries(OSFilter_Test ${TEST_LIBS})

#install(TARGETS OSFilter_Test DESTINATION tests)


########### next target ###############

set(TestServer_SRCS
    TestServer.cxx
    ../src/Command.cxx
    ../src/UI.cxx
    ../src/Params.cxx
    ../src/UDSockets.cxx
    ../src/Spectrogram.cxx
    ../src/SoDaBase.cxx
    ../src/Debug.cxx)

add_executable(TestServer ${TestServer_SRCS})

target_link_libraries(TestServer  rt ${TEST_LIBS})

#install(TARGETS TestServer DESTINATION tests)


########### next target ###############

set(ReSampler_Test_SRCS
    ReSampler_Test.cxx
    ../src/ReSampler.cxx
    ../src/ReSamplers625x48.cxx
    ../src/SoDaBase.cxx)

add_executable(ReSampler_Test ${ReSampler_Test_SRCS})

target_link_libraries(ReSampler_Test ${TEST_LIBS})

#install(TARGETS ReSampler_Test DESTINATION tests)


########### next target ###############

set(TSIP_Test_SRCS TSIP_Test.cxx ../src/TSIP.cxx)

add_executable(TSIP_Test ${TSIP_Test_SRCS})

target_link_libraries(TSIP_Test ${TEST_LIBS})

#install(TARGETS TSIP_Test DESTINATION tests)


########### next target ###############

set(Audio_test_SRCS
    Audio_test.cxx
    ../src/AudioPA.cxx
    ../src/AudioALSA.cxx
    ../src/SoDaBase.cxx
    ../src/Debug.cxx)

add_executable(Audio_test ${Audio_test_SRCS})

target_link_libraries(Audio_test ${TEST_LIBS})

#install(TARGETS Audio_test DESTINATION tests)


########### next target ###############

set(Hilbert_Test_SRCS Hilbert_Test.cxx ../src/HilbertTransformer.cxx ../src/SoDaBase.cxx)

add_executable(Hilbert_Test ${Hilbert_Test_SRCS})

target_link_libraries(Hilbert_Test ${TEST_LIBS})

#install(TARGETS Hilbert_Test DESTINATION tests)


########### next target ###############

set(FindHome_Test_SRCS FindHome_Test.cxx ../gui/FindHome.cxx ../gui/FindHome.hxx)

add_executable(FindHome_Test ${FindHome_Test_SRCS})

target_link_libraries(FindHome_Test ${TEST_LIBS})

#install(TARGETS FindHome_Test DESTINATION tests)


########### next target ###############

set(RangeMap_test_SRCS RangeMap_test.cxx ../src/RangeMap.hxx)

add_executable(RangeMap_test ${RangeMap_test_SRCS})

target_link_libraries(RangeMap_test ${TEST_LIBS})

#install(TARGETS RangeMap_test DESTINATION tests)


########### next target ###############

set(SerialDev_Test_SRCS SerialDev_Test.cxx ../src/SerialDev.hxx ../src/SerialDev.cxx)

add_executable(SerialDev_Test ${SerialDev_Test_SRCS})

target_link_libraries(SerialDev_Test ${TEST_LIBS})

#install(TARGETS SerialDev_Test DESTINATION tests)


########### next target ###############

set(TRControl_Test_SRCS
    TRControl_Test.cxx
    ../src/N200Control.hxx
    ../src/N200Control.cxx
    ../src/TRControl.hxx
    ../src/TRControl.cxx
    ../src/IPSockets.hxx
    ../src/IPSockets.cxx
    ../src/B200Control.hxx
    ../src/B200Control.cxx)

add_executable(TRControl_Test ${TRControl_Test_SRCS})

target_link_libraries(TRControl_Test ${TEST_LIBS})

#install(TARGETS TRControl_Test DESTINATION tests)


########### next target ###############

set(IntN_FreqTable_SRCS IntN_FreqTable.cxx)

add_executable(IntN_FreqTable ${IntN_FreqTable_SRCS})

target_link_libraries(IntN_FreqTable ${TEST_LIBS})

#install(TARGETS IntN_FreqTable DESTINATION tests)


########### next target ###############

set(Demod_test_SRCS
    Demod_test.cxx
    ../src/Demod.hxx
    ../src/Demod.cxx
    ../src/SoDaBase.hxx
    ../src/Debug.hxx
    ../src/Debug.cxx
    ../src/SoDaBase.cxx)

add_executable(Demod_test ${Demod_test_SRCS})

target_link_libraries(Demod_test ${TEST_LIBS})

#install(TARGETS Demod_test DESTINATION tests)


########### next target ###############

set(FrontEnd_test_SRCS FrontEnd_test.cxx ../src/FrontEnd.hxx ../src/PropTree.hxx)

add_executable(FrontEnd_test ${FrontEnd_test_SRCS})

target_link_libraries(FrontEnd_test ${TEST_LIBS})

#install(TARGETS FrontEnd_test DESTINATION tests)


########### install files ###############




#original Makefile.am contents follow:

#AUTOMAKE_OPTIONS = subdir-objects
#AM_CPPFLAGS = -I$(top_srcdir)/src
#AM_BOOSTLIBFLAGS = ${BOOST_PROGRAM_OPTIONS_LIB} ${BOOST_THREAD_LIB} ${BOOST_DATE_TIME_LIB} ${BOOST_SYSTEM_LIB} 
#
#
#bin_PROGRAMS = FFT_Test Sockets_Test MultiMBox_Test SoDa_ThreadTest OSFilter_Test  TestServer ReSampler_Test TSIP_Test Audio_test Hilbert_Test FindHome_Test RangeMap_test SerialDev_Test TRControl_Test IntN_FreqTable Demod_test FrontEnd_test
#
#
#Sockets_Test_SOURCES = Sockets_Test.cxx  ../src/IPSockets.cxx ../src/UDSockets.cxx ../src/IPSockets.hxx ../src/UDSockets.hxx
#Sockets_Test_CPPFLAGS = $(AM_CPPFLAGS)
#Sockets_Test_LDADD = $(AM_BOOSTLIBFLAGS)
#
#SoDa_ThreadTest_SOURCES = SoDa_ThreadTest.cxx  ../src/SoDaBase.cxx ../src/Debug.cxx
#SoDa_ThreadTest_CPPFLAGS = $(AM_CPPFLAGS)
#SoDa_ThreadTest_LDADD = $(AM_BOOSTLIBFLAGS)
#
#
#
#Audio_test_SOURCES = Audio_test.cxx  ../src/AudioPA.cxx ../src/AudioALSA.cxx ../src/SoDaBase.cxx ../src/Debug.cxx
#Audio_test_CPPFLAGS = $(AM_CPPFLAGS)
#Audio_test_LDADD = $(AM_BOOSTLIBFLAGS)
#
#
#MultiMBox_Test_SOURCES = MultiMBox_Test.cxx  ../src/SoDaBase.cxx ../src/Debug.cxx
#MultiMBox_Test_CPPFLAGS = $(AM_CPPFLAGS)
#MultiMBox_Test_LDADD = $(AM_BOOSTLIBFLAGS)
#
#
#OSFilter_Test_SOURCES = OSFilter_Test.cxx  ../src/OSFilter.cxx
#OSFilter_Test_CPPFLAGS = $(AM_CPPFLAGS) -g -O1 
#OSFilter_Test_LDADD = $(AM_BOOSTLIBFLAGS) 
#OSFilter_Test_CXXFLAGS = -O1 -g
#OSFilter_Test_LDFLAGS = -g -O1
#
#Hilbert_Test_SOURCES = Hilbert_Test.cxx  ../src/HilbertTransformer.cxx  ../src/SoDaBase.cxx
#Hilbert_Test_CPPFLAGS = $(AM_CPPFLAGS) -g -O1 
#Hilbert_Test_LDADD = $(AM_BOOSTLIBFLAGS) 
#Hilbert_Test_CXXFLAGS = -O1 -g
#Hilbert_Test_LDFLAGS = -g -O1
#
#
#
#FFT_Test_SOURCES = FFT_Test.cxx  
#FFT_Test_CPPFLAGS = $(AM_CPPFLAGS) -g -O1 
#FFT_Test_LDADD = $(AM_BOOSTLIBFLAGS) 
#FFT_Test_CXXFLAGS = -O1 -g
#FFT_Test_LDFLAGS = -g -O1
#
#
#TSIP_Test_SOURCES = TSIP_Test.cxx  ../src/TSIP.cxx
#TSIP_Test_CPPFLAGS = $(AM_CPPFLAGS)
#TSIP_Test_LDADD = $(AM_BOOSTLIBFLAGS) 
#
#
#
#TestServer_SOURCES = TestServer.cxx  ../src/Command.cxx ../src/UI.cxx ../src/Params.cxx ../src/UDSockets.cxx ../src/Spectrogram.cxx ../src/SoDaBase.cxx ../src/Debug.cxx
#TestServer_CPPFLAGS = $(AM_CPPFLAGS)
#TestServer_LDADD = $(AM_BOOSTLIBFLAGS) -lrt
#
#
#ReSampler_Test_SOURCES = ReSampler_Test.cxx  ../src/ReSampler.cxx ../src/ReSamplers625x48.cxx ../src/SoDaBase.cxx
#ReSampler_Test_CPPFLAGS = $(AM_CPPFLAGS) -g -O3 -ffast-math 
#ReSampler_Test_LDADD = $(AM_BOOSTLIBFLAGS) 
#ReSampler_Test_CXXFLAGS = -g -O3 -ffast-math
#ReSampler_Test_LDFLAGS = -g -O3 
#
#
#FindHome_Test_SOURCES = FindHome_Test.cxx  ../gui/FindHome.cxx ../gui/FindHome.hxx
#FindHome_Test_CPPFLAGS = $(AM_CPPFLAGS) -g -O1 
#FindHome_Test_LDADD = $(AM_BOOSTLIBFLAGS) 
#FindHome_Test_CXXFLAGS = -g -O1
#FindHome_Test_LDFLAGS = -g -O1
#
#
#RangeMap_test_SOURCES = RangeMap_test.cxx  ../src/RangeMap.hxx
#RangeMap_test_CPPFLAGS = $(AM_CPPFLAGS) -g -O1 
#RangeMap_test_LDADD = $(AM_BOOSTLIBFLAGS) 
#RangeMap_test_CXXFLAGS = -g -O1
#RangeMap_test_LDFLAGS = -g -O1
#
#SerialDev_Test_SOURCES = SerialDev_Test.cxx  ../src/SerialDev.hxx ../src/SerialDev.cxx
#SerialDev_Test_CPPFLAGS = $(AM_CPPFLAGS) -g -O1 
#SerialDev_Test_LDADD = $(AM_BOOSTLIBFLAGS) 
#SerialDev_Test_CXXFLAGS = -g -O1
#SerialDev_Test_LDFLAGS = -g -O1
#
#TRControl_Test_SOURCES = TRControl_Test.cxx  ../src/N200Control.hxx ../src/N200Control.cxx ../src/TRControl.hxx ../src/TRControl.cxx ../src/IPSockets.hxx ../src/IPSockets.cxx ../src/B200Control.hxx ../src/B200Control.cxx
#TRControl_Test_CPPFLAGS = $(AM_CPPFLAGS) -g -O1 
#TRControl_Test_LDADD = $(AM_BOOSTLIBFLAGS) 
#TRControl_Test_CXXFLAGS = -g -O1
#TRControl_Test_LDFLAGS = -g -O1
#
#IntN_FreqTable_SOURCES = IntN_FreqTable.cxx 
#IntN_FreqTable_CPPFLAGS = $(AM_CPPFLAGS) -g -O1 
#IntN_FreqTable_LDADD = $(AM_BOOSTLIBFLAGS) 
#IntN_FreqTable_CXXFLAGS = -g -O1
#IntN_FreqTable_LDFLAGS = -g -O1
#
#Demod_test_SOURCES = Demod_test.cxx  ../src/Demod.hxx ../src/Demod.cxx ../src/SoDaBase.hxx ../src/Debug.hxx ../src/Debug.cxx ../src/SoDaBase.cxx
#Demod_test_CPPFLAGS = $(AM_CPPFLAGS) -g -O1 
#Demod_test_LDADD = $(AM_BOOSTLIBFLAGS) 
#Demod_test_CXXFLAGS = -g -O1
#Demod_test_LDFLAGS = -g -O1
#
#
#FrontEnd_test_SOURCES = FrontEnd_test.cxx  ../src/FrontEnd.hxx ../src/PropTree.hxx
#FrontEnd_test_CPPFLAGS = $(AM_CPPFLAGS) -g -O1 
#FrontEnd_test_LDADD = $(AM_BOOSTLIBFLAGS) 
#FrontEnd_test_CXXFLAGS = -g -O1
#FrontEnd_test_LDFLAGS = -g -O1
#
