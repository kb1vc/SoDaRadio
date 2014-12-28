AUTOMAKE_OPTIONS = subdir-objects
AM_CPPFLAGS = -I$(top_srcdir)/src
AM_BOOSTLIBFLAGS = ${BOOST_PROGRAM_OPTIONS_LIB} ${BOOST_THREAD_LIB} ${BOOST_DATE_TIME_LIB} ${BOOST_SYSTEM_LIB} 


bin_PROGRAMS = FFT_Test Sockets_Test MultiMBox_Test SoDa_ThreadTest OSFilter_Test  TestServer ReSampler_Test TSIP_Test Audio_test Hilbert_Test FindHome_Test RangeMap_test DDC_Test DDC_Post DDC_Analyze DCBlock_Test USRP2_Cordic_Model


Sockets_Test_SOURCES = Sockets_Test.cxx  ../src/IPSockets.cxx ../src/UDSockets.cxx ../src/IPSockets.hxx ../src/UDSockets.hxx
Sockets_Test_CPPFLAGS = $(AM_CPPFLAGS)
Sockets_Test_LDADD = $(AM_BOOSTLIBFLAGS)

SoDa_ThreadTest_SOURCES = SoDa_ThreadTest.cxx  ../src/SoDaBase.cxx ../src/Debug.cxx
SoDa_ThreadTest_CPPFLAGS = $(AM_CPPFLAGS)
SoDa_ThreadTest_LDADD = $(AM_BOOSTLIBFLAGS)



Audio_test_SOURCES = Audio_test.cxx  ../src/AudioPA.cxx ../src/AudioALSA.cxx ../src/SoDaBase.cxx ../src/Debug.cxx
Audio_test_CPPFLAGS = $(AM_CPPFLAGS)
Audio_test_LDADD = $(AM_BOOSTLIBFLAGS)


MultiMBox_Test_SOURCES = MultiMBox_Test.cxx  ../src/SoDaBase.cxx ../src/Debug.cxx
MultiMBox_Test_CPPFLAGS = $(AM_CPPFLAGS)
MultiMBox_Test_LDADD = $(AM_BOOSTLIBFLAGS)


OSFilter_Test_SOURCES = OSFilter_Test.cxx  ../src/OSFilter.cxx
OSFilter_Test_CPPFLAGS = $(AM_CPPFLAGS) -g -O1 
OSFilter_Test_LDADD = $(AM_BOOSTLIBFLAGS) 
OSFilter_Test_CXXFLAGS = -O1 -g
OSFilter_Test_LDFLAGS = -g -O1

Hilbert_Test_SOURCES = Hilbert_Test.cxx  ../src/HilbertTransformer.cxx  ../src/SoDaBase.cxx
Hilbert_Test_CPPFLAGS = $(AM_CPPFLAGS) -g -O1 
Hilbert_Test_LDADD = $(AM_BOOSTLIBFLAGS) 
Hilbert_Test_CXXFLAGS = -O1 -g
Hilbert_Test_LDFLAGS = -g -O1



FFT_Test_SOURCES = FFT_Test.cxx  
FFT_Test_CPPFLAGS = $(AM_CPPFLAGS) -g -O1 
FFT_Test_LDADD = $(AM_BOOSTLIBFLAGS) 
FFT_Test_CXXFLAGS = -O1 -g
FFT_Test_LDFLAGS = -g -O1


TSIP_Test_SOURCES = TSIP_Test.cxx  ../src/TSIP.cxx
TSIP_Test_CPPFLAGS = $(AM_CPPFLAGS)
TSIP_Test_LDADD = $(AM_BOOSTLIBFLAGS) 



TestServer_SOURCES = TestServer.cxx  ../src/Command.cxx ../src/UI.cxx ../src/Params.cxx ../src/UDSockets.cxx ../src/Spectrogram.cxx ../src/SoDaBase.cxx ../src/Debug.cxx
TestServer_CPPFLAGS = $(AM_CPPFLAGS)
TestServer_LDADD = $(AM_BOOSTLIBFLAGS) -lrt


ReSampler_Test_SOURCES = ReSampler_Test.cxx  ../src/ReSampler.cxx ../src/ReSamplers625x48.cxx ../src/SoDaBase.cxx
ReSampler_Test_CPPFLAGS = $(AM_CPPFLAGS) -g -O3 -ffast-math 
ReSampler_Test_LDADD = $(AM_BOOSTLIBFLAGS) 
ReSampler_Test_CXXFLAGS = -g -O3 -ffast-math
ReSampler_Test_LDFLAGS = -g -O3 


FindHome_Test_SOURCES = FindHome_Test.cxx  ../gui/FindHome.cxx ../gui/FindHome.hxx
FindHome_Test_CPPFLAGS = $(AM_CPPFLAGS) -g -O1 
FindHome_Test_LDADD = $(AM_BOOSTLIBFLAGS) 
FindHome_Test_CXXFLAGS = -g -O1
FindHome_Test_LDFLAGS = -g -O1


RangeMap_test_SOURCES = RangeMap_test.cxx  ../src/RangeMap.hxx
RangeMap_test_CPPFLAGS = $(AM_CPPFLAGS) -g -O1 
RangeMap_test_LDADD = $(AM_BOOSTLIBFLAGS) 
RangeMap_test_CXXFLAGS = -g -O1
RangeMap_test_LDFLAGS = -g -O1

DDC_Test_SOURCES = DDC_Test.cxx  ../src/Command.cxx ../src/Params.cxx ../src/USRPCtrl.cxx ../src/USRPRX.cxx ../src/SoDaBase.cxx ../src/Debug.cxx ../src/Spectrogram.cxx
DDC_Test_CPPFLAGS = $(AM_CPPFLAGS)
DDC_Test_LDADD = $(AM_BOOSTLIBFLAGS)


DDC_Post_SOURCES = DDC_Post.cxx 
DDC_Post_CPPFLAGS = $(AM_CPPFLAGS)
DDC_Post_LDADD = $(AM_BOOSTLIBFLAGS)


DDC_Analyze_SOURCES = DDC_Analyze.cxx 
DDC_Analyze_CPPFLAGS = $(AM_CPPFLAGS)
DDC_Analyze_LDADD = $(AM_BOOSTLIBFLAGS)

DCBlock_Test_SOURCES = DCBlock_Test.cxx  ../src/DCBlock.hxx
DCBlock_Test_CPPFLAGS = $(AM_CPPFLAGS) 
DCBlock_Test_LDADD = $(AM_BOOSTLIBFLAGS) 

USRP2_Cordic_Model_SOURCES = USRP2_Cordic_Model.cxx  ../src/Spectrogram.cxx ../src/Spectrogram.hxx ../src/DCBlock.hxx 
USRP2_Cordic_Model_CPPFLAGS = $(AM_CPPFLAGS) 
USRP2_Cordic_Model_LDADD = $(AM_BOOSTLIBFLAGS) 
