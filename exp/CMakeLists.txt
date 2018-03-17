#find_package(Portaudio)
find_package(ALSA REQUIRED QUIET)
find_package(FFTW3f REQUIRED QUIET)

add_definitions(-DHAVE_LIBASOUND=1)

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


set(LineSockets_Test_SRCS
    LineSockets_Test.cxx
    ../src/IPSockets.cxx
    ../src/IPSockets.hxx
    ../src/LineSocket.hxx)

add_executable(LineSockets_Test ${LineSockets_Test_SRCS})

target_link_libraries(LineSockets_Test ${TEST_LIBS})

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

set(OSFILTER_LIBS ${Boost_LIBRARIES}  ${FFTW3F_LIBRARIES})

add_executable(OSFilter_Test ${OSFilter_Test_SRCS})

target_link_libraries(OSFilter_Test ${OSFILTER_LIBS})

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

set(FindHome_Test_SRCS FindHome_Test.cxx ../src/FindHome.cxx ../src/FindHome.hxx)

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
IF(${HAVE_UHD})
  set(USRPFrontEnd_test_SRCS USRPFrontEnd_test.cxx ../src/USRPFrontEnd.cxx ../src/PropTree.hxx)

  add_executable(USRPFrontEnd_test ${USRPFrontEnd_test_SRCS})

  target_link_libraries(USRPFrontEnd_test ${TEST_LIBS})
ENDIF()

#install(TARGETS USRPFrontEnd_test DESTINATION tests)


set(OneFilterTest_SRCS OneFilterTest.cxx ../src/OSFilter.cxx)

set(OSFILTER_LIBS ${Boost_LIBRARIES}  ${FFTW3F_LIBRARIES})

add_executable(OneFilterTest ${OneFilterTest_SRCS})

target_link_libraries(OneFilterTest ${OSFILTER_LIBS})



set(ProcInfo_test_SRCS ProcInfo_test.cxx ../src/ProcInfo.hxx ../src/ProcInfo.cxx)

add_executable(ProcInfo_test ${ProcInfo_test_SRCS})

target_link_libraries(ProcInfo_test ${Boost_LIBRARIES})

add_executable(NCOexp NCOexp.cxx)

target_link_libraries(NCOexp ${Boost_LIBRARIES})

set(TDResampler_Test_SRCS
    TDResampler_Test.cxx
    ../src/TDResamplerTables625x48.cxx
    ../src/SoDaBase.cxx)

add_executable(TDResampler_Test ${TDResampler_Test_SRCS})

target_link_libraries(TDResampler_Test ${TEST_LIBS})

set(ResamplerSpeed_SRCS
  ResamplerSpeed.cxx
  ../src/ReSampler.cxx
  ../src/ReSamplers625x48.cxx
  ../src/TDResamplerTables625x48.cxx
  ../src/SoDaBase.cxx)

add_executable(ResamplerSpeed ${ResamplerSpeed_SRCS})
target_link_libraries(ResamplerSpeed ${TEST_LIBS})

