
include_directories(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### next target ###############

set(FFT_Test_SRCS FFT_Test.cxx)

kde4_add_executable(FFT_Test ${FFT_Test_SRCS})

target_link_libraries(FFT_Test ${KDE4_KDECORE_LIBS})

install(TARGETS FFT_Test ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(Sockets_Test_SRCS
    Sockets_Test.cxx
    ../src/IPSockets.cxx
    ../src/UDSockets.cxx
    ../src/IPSockets.hxx
    ../src/UDSockets.hxx)

kde4_add_executable(Sockets_Test ${Sockets_Test_SRCS})

target_link_libraries(Sockets_Test ${KDE4_KDECORE_LIBS})

install(TARGETS Sockets_Test ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(MultiMBox_Test_SRCS MultiMBox_Test.cxx ../src/SoDaBase.cxx ../src/Debug.cxx)

kde4_add_executable(MultiMBox_Test ${MultiMBox_Test_SRCS})

target_link_libraries(MultiMBox_Test ${KDE4_KDECORE_LIBS})

install(TARGETS MultiMBox_Test ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(SoDa_ThreadTest_SRCS SoDa_ThreadTest.cxx ../src/SoDaBase.cxx ../src/Debug.cxx)

kde4_add_executable(SoDa_ThreadTest ${SoDa_ThreadTest_SRCS})

target_link_libraries(SoDa_ThreadTest ${KDE4_KDECORE_LIBS})

install(TARGETS SoDa_ThreadTest ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(OSFilter_Test_SRCS OSFilter_Test.cxx ../src/OSFilter.cxx)

kde4_add_executable(OSFilter_Test ${OSFilter_Test_SRCS})

target_link_libraries(OSFilter_Test ${KDE4_KDECORE_LIBS})

install(TARGETS OSFilter_Test ${INSTALL_TARGETS_DEFAULT_ARGS})


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

kde4_add_executable(TestServer ${TestServer_SRCS})

target_link_libraries(TestServer ${KDE4_KDECORE_LIBS} rt)

install(TARGETS TestServer ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(ReSampler_Test_SRCS
    ReSampler_Test.cxx
    ../src/ReSampler.cxx
    ../src/ReSamplers625x48.cxx
    ../src/SoDaBase.cxx)

kde4_add_executable(ReSampler_Test ${ReSampler_Test_SRCS})

target_link_libraries(ReSampler_Test ${KDE4_KDECORE_LIBS})

install(TARGETS ReSampler_Test ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(TSIP_Test_SRCS TSIP_Test.cxx ../src/TSIP.cxx)

kde4_add_executable(TSIP_Test ${TSIP_Test_SRCS})

target_link_libraries(TSIP_Test ${KDE4_KDECORE_LIBS})

install(TARGETS TSIP_Test ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(Audio_test_SRCS
    Audio_test.cxx
    ../src/AudioPA.cxx
    ../src/AudioALSA.cxx
    ../src/SoDaBase.cxx
    ../src/Debug.cxx)

kde4_add_executable(Audio_test ${Audio_test_SRCS})

target_link_libraries(Audio_test ${KDE4_KDECORE_LIBS})

install(TARGETS Audio_test ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(Hilbert_Test_SRCS Hilbert_Test.cxx ../src/HilbertTransformer.cxx ../src/SoDaBase.cxx)

kde4_add_executable(Hilbert_Test ${Hilbert_Test_SRCS})

target_link_libraries(Hilbert_Test ${KDE4_KDECORE_LIBS})

install(TARGETS Hilbert_Test ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(FindHome_Test_SRCS FindHome_Test.cxx ../src/FindHome.cxx ../src/FindHome.hxx)

kde4_add_executable(FindHome_Test ${FindHome_Test_SRCS})

target_link_libraries(FindHome_Test ${KDE4_KDECORE_LIBS})

install(TARGETS FindHome_Test ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(RangeMap_test_SRCS RangeMap_test.cxx ../src/RangeMap.hxx)

kde4_add_executable(RangeMap_test ${RangeMap_test_SRCS})

target_link_libraries(RangeMap_test ${KDE4_KDECORE_LIBS})

install(TARGETS RangeMap_test ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(SerialDev_Test_SRCS SerialDev_Test.cxx ../src/SerialDev.hxx ../src/SerialDev.cxx)

kde4_add_executable(SerialDev_Test ${SerialDev_Test_SRCS})

target_link_libraries(SerialDev_Test ${KDE4_KDECORE_LIBS})

install(TARGETS SerialDev_Test ${INSTALL_TARGETS_DEFAULT_ARGS})


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

kde4_add_executable(TRControl_Test ${TRControl_Test_SRCS})

target_link_libraries(TRControl_Test ${KDE4_KDECORE_LIBS})

install(TARGETS TRControl_Test ${INSTALL_TARGETS_DEFAULT_ARGS})


