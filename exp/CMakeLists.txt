#find_package(Portaudio)

find_package(FFTW3f REQUIRED QUIET)

add_definitions(-DHAVE_LIBASOUND=1)

include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${UHD_INCLUDE_DIRS} ${ALSA_INCLUDE_DIRS} ${FFTW3F_INCLUDE_DIRS}
  ${SoDaUtils_INCLUDE_DIR}
  ../src ../common)

set(TEST_LIBS ${Boost_LIBRARIES} ${UHD_LIBRARIES} ${ALSA_LIBRARIES} ${FFTW3F_LIBRARIES} ${SoDaUtils_LIBRARIES})
set(SIMPLE_TEST_LIBS ${FFTW3F_LIBRARIES} ${SoDaUtils_LIBRARIES})

########### next target ###############

set(FFT_Test_SRCS FFT_Test.cxx)

add_executable(FFT_Test EXCLUDE_FROM_ALL ${FFT_Test_SRCS})

target_link_libraries(FFT_Test ${TEST_LIBS})

#install(TARGETS FFT_Test DESTINATION tests)


########### next target ###############

set(Sockets_Test_SRCS
    Sockets_Test.cxx
    ../src/IPSockets.cxx
    ../src/UDSockets.cxx
    ../src/IPSockets.hxx
    ../src/UDSockets.hxx)

add_executable(Sockets_Test  EXCLUDE_FROM_ALL ${Sockets_Test_SRCS})

target_link_libraries(Sockets_Test ${TEST_LIBS})


set(LineSockets_Test_SRCS
    LineSockets_Test.cxx
    ../src/IPSockets.cxx
    ../src/IPSockets.hxx
    ../src/LineSocket.hxx)

add_executable(LineSockets_Test EXCLUDE_FROM_ALL ${LineSockets_Test_SRCS})

target_link_libraries(LineSockets_Test ${TEST_LIBS})

#install(TARGETS Sockets_Test DESTINATION tests)


########### next target ###############

set(MultiMBox_Test_SRCS MultiMBox_Test.cxx ../src/SoDaBase.cxx ../src/Debug.cxx)

add_executable(MultiMBox_Test EXCLUDE_FROM_ALL ${MultiMBox_Test_SRCS})

target_link_libraries(MultiMBox_Test ${TEST_LIBS})

#install(TARGETS MultiMBox_Test DESTINATION tests)


########### next target ###############

set(SoDa_ThreadTest_SRCS SoDa_ThreadTest.cxx ../src/SoDaBase.cxx ../src/Debug.cxx ../src/Thread.cxx ../src/ThreadRegistry.cxx)

add_executable(SoDa_ThreadTest EXCLUDE_FROM_ALL ${SoDa_ThreadTest_SRCS})

target_link_libraries(SoDa_ThreadTest ${TEST_LIBS})

#install(TARGETS SoDa_ThreadTest DESTINATION tests)


########### next target ###############

set(TestServer_SRCS
    TestServer.cxx
    ../src/Command.cxx
    ../src/UI.cxx
    ../src/Params.cxx
    ../src/UDSockets.cxx
    ../src/SoDaBase.cxx
    ../src/Debug.cxx)

add_executable(TestServer EXCLUDE_FROM_ALL ${TestServer_SRCS})

target_link_libraries(TestServer  rt ${TEST_LIBS})



########### next target ###############

#set(Audio_test_SRCS
#    Audio_test.cxx
#    ../src/AudioALSA.cxx
#    ../src/SoDaBase.cxx
#    ../src/Debug.cxx)

#add_executable(Audio_test EXCLUDE_FROM_ALL ${Audio_test_SRCS})

#target_link_libraries(Audio_test ${TEST_LIBS})

#install(TARGETS Audio_test DESTINATION tests)


########### next target ###############

########### next target ###############

set(FindHome_Test_SRCS FindHome_Test.cxx ../src/FindHome.cxx ../src/FindHome.hxx)

add_executable(FindHome_Test EXCLUDE_FROM_ALL ${FindHome_Test_SRCS})

target_link_libraries(FindHome_Test ${TEST_LIBS})

#install(TARGETS FindHome_Test DESTINATION tests)


########### next target ###############

set(RangeMap_test_SRCS RangeMap_test.cxx RangeMap.hxx)

add_executable(RangeMap_test EXCLUDE_FROM_ALL ${RangeMap_test_SRCS})

target_link_libraries(RangeMap_test ${TEST_LIBS})

#install(TARGETS RangeMap_test DESTINATION tests)


########### next target ###############

set(SerialDev_Test_SRCS SerialDev_Test.cxx ../src/SerialDev.hxx ../src/SerialDev.cxx)

add_executable(SerialDev_Test EXCLUDE_FROM_ALL ${SerialDev_Test_SRCS})

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

add_executable(TRControl_Test EXCLUDE_FROM_ALL ${TRControl_Test_SRCS})

target_link_libraries(TRControl_Test ${TEST_LIBS})

#install(TARGETS TRControl_Test DESTINATION tests)


########### next target ###############

set(IntN_FreqTable_SRCS IntN_FreqTable.cxx)

add_executable(IntN_FreqTable EXCLUDE_FROM_ALL ${IntN_FreqTable_SRCS})

target_link_libraries(IntN_FreqTable ${TEST_LIBS})

#install(TARGETS IntN_FreqTable DESTINATION tests)


########### next target ###############
IF(${HAVE_UHD})
  set(USRPFrontEnd_test_SRCS USRPFrontEnd_test.cxx ../src/USRPFrontEnd.cxx ../src/USRPPropTree.hxx)

  add_executable(USRPFrontEnd_test EXCLUDE_FROM_ALL ${USRPFrontEnd_test_SRCS})

  target_link_libraries(USRPFrontEnd_test ${TEST_LIBS})
ENDIF()

#install(TARGETS USRPFrontEnd_test DESTINATION tests)


add_executable(NCOexp EXCLUDE_FROM_ALL NCOexp.cxx)

target_link_libraries(NCOexp ${Boost_LIBRARIES})


