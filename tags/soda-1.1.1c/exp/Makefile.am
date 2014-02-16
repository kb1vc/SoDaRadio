AUTOMAKE_OPTIONS = subdir-objects
AM_CPPFLAGS = -I$(top_srcdir)/src
AM_BOOSTLIBFLAGS = ${BOOST_PROGRAM_OPTIONS_LIB} ${BOOST_THREAD_LIB} ${BOOST_DATE_TIME_LIB} ${BOOST_SYSTEM_LIB} 


bin_PROGRAMS = FFT_Test Sockets_Test MultiMBox_Test SoDa_ThreadTest OSFilter_Test  TestServer ReSampler_Test TSIP_Test Audio_test Hilbert_Test


Sockets_Test_SOURCES = Sockets_Test.cxx  ../src/IPSockets.cxx ../src/UDSockets.cxx ../src/IPSockets.hxx ../src/UDSockets.hxx
Sockets_Test_CPPFLAGS = $(AM_CPPFLAGS)
Sockets_Test_LDADD = $(AM_BOOSTLIBFLAGS)

SoDa_ThreadTest_SOURCES = SoDa_ThreadTest.cxx  ../src/SoDaBase.cxx
SoDa_ThreadTest_CPPFLAGS = $(AM_CPPFLAGS)
SoDa_ThreadTest_LDADD = $(AM_BOOSTLIBFLAGS)



Audio_test_SOURCES = Audio_test.cxx  ../src/AudioPA.cxx ../src/AudioALSA.cxx ../src/SoDaBase.cxx
Audio_test_CPPFLAGS = $(AM_CPPFLAGS)
Audio_test_LDADD = $(AM_BOOSTLIBFLAGS)


MultiMBox_Test_SOURCES = MultiMBox_Test.cxx  ../src/SoDaBase.cxx
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



TestServer_SOURCES = TestServer.cxx  ../src/Command.cxx ../src/UI.cxx ../src/Params.cxx ../src/UDSockets.cxx ../src/Spectrogram.cxx ../src/SoDaBase.cxx
TestServer_CPPFLAGS = $(AM_CPPFLAGS)
TestServer_LDADD = $(AM_BOOSTLIBFLAGS) -lrt


ReSampler_Test_SOURCES = ReSampler_Test.cxx  ../src/ReSampler.cxx ../src/ReSamplers625x48.cxx ../src/SoDaBase.cxx
ReSampler_Test_CPPFLAGS = $(AM_CPPFLAGS) -g -O3 -ffast-math 
ReSampler_Test_LDADD = $(AM_BOOSTLIBFLAGS) 
ReSampler_Test_CXXFLAGS = -g -O3 -ffast-math
ReSampler_Test_LDFLAGS = -g -O3 
