AUTOMAKE_OPTIONS = subdir-objects
AM_CPPFLAGS = -I$(top_srcdir)/src ${BOOST_CPPFLAGS}
AM_BOOSTLIBFLAGS = ${BOOST_LDFLAGS} ${BOOST_PROGRAM_OPTIONS_LIB} ${BOOST_THREAD_LIB} ${BOOST_DATE_TIME_LIB} ${BOOST_SYSTEM_LIB} ${BOOST_FILESYSTEM_LIB} ${BOOST_SERIALIZATION_LIB} ${BOOST_REGEX_LIB}

bin_PROGRAMS = SoDaServer SoDaCan SoDaBenchServer SoDaNFSurvey

SoDaCan_SOURCES = SoDaCan.cxx UDSockets.cxx UDSockets.hxx IPSockets.cxx IPSockets.hxx Command.cxx Command.hxx ../gui/FindHome.cxx ../gui/FindHome.hxx

SoDaServer_SOURCES = Params.cxx  SoDaServer.cxx SoDaBase.cxx USRPCtrl.cxx USRPRX.cxx USRPTX.cxx CWTX.cxx AudioRX.cxx AudioTX.cxx UI.cxx Command.cxx OSFilter.cxx HilbertTransformer.cxx  ReSampler.cxx ReSamplers625x48.cxx Spectrogram.cxx CWGenerator.cxx TSIP.cxx GPS_TSIPmon.cxx UDSockets.cxx AudioRX.hxx AudioTX.hxx Command.hxx CWGenerator.hxx CWTX.hxx GPS_TSIPmon.hxx HilbertTransformer.hxx IPSockets.hxx  MultiMBox.hxx OSFilter.hxx Params.hxx QuadratureOscillator.hxx ReSampler.hxx ReSamplers625x48.hxx SoDaBase.hxx SoDa_filter_tables.hxx SoDa_tx_filter_tables.hxx Spectrogram.hxx TSIP.hxx UDSockets.hxx UI.hxx USRPCtrl.hxx USRPRX.hxx USRPTX.hxx  AudioALSA.hxx AudioALSA.cxx AudioIfc.hxx MedianFilter.hxx AudioPA.hxx AudioPA.cxx

SoDaBenchServer_SOURCES = Params.cxx  SoDaBenchServer.cxx SoDaBase.cxx USRPCtrl.cxx USRPRX.cxx USRPTX.cxx BenchUI.cxx Command.cxx OSFilter.cxx HilbertTransformer.cxx  ReSampler.cxx ReSamplers625x48.cxx Spectrogram.cxx UDSockets.cxx Command.hxx HilbertTransformer.hxx IPSockets.hxx  MultiMBox.hxx OSFilter.hxx Params.hxx QuadratureOscillator.hxx ReSampler.hxx ReSamplers625x48.hxx SoDaBase.hxx SoDa_filter_tables.hxx SoDa_tx_filter_tables.hxx Spectrogram.hxx TSIP.hxx UDSockets.hxx BenchUI.hxx USRPCtrl.hxx USRPRX.hxx USRPTX.hxx MedianFilter.hxx

SoDaNFSurvey_SOURCES = Params.cxx  SoDaNFSurvey.cxx SoDaBase.cxx USRPCtrl.cxx USRPRX.cxx USRPTX.cxx BenchUI.cxx Command.cxx OSFilter.cxx HilbertTransformer.cxx  ReSampler.cxx ReSamplers625x48.cxx Spectrogram.cxx UDSockets.cxx Command.hxx HilbertTransformer.hxx IPSockets.hxx  MultiMBox.hxx OSFilter.hxx Params.hxx QuadratureOscillator.hxx ReSampler.hxx ReSamplers625x48.hxx SoDaBase.hxx SoDa_filter_tables.hxx SoDa_tx_filter_tables.hxx Spectrogram.hxx TSIP.hxx UDSockets.hxx BenchUI.hxx USRPCtrl.hxx USRPRX.hxx USRPTX.hxx MedianFilter.hxx NFThermometer.hxx NFThermometer.cxx AudioRX.hxx AudioRX.cxx

SoDaServer_CPPFLAGS = $(AM_CPPFLAGS)  -g
SoDaServer_LDADD = $(AM_BOOSTLIBFLAGS)  -lrt
SoDaServer_CXXFLAGS = -g
SoDaServer_LDFLAGS = -g 

SoDaCan_CPPFLAGS = $(AM_CPPFLAGS)  -g
SoDaCan_LDADD = $(AM_BOOSTLIBFLAGS)  -lrt
SoDaCan_CXXFLAGS = -g
SoDaCan_LDFLAGS = -g 

SoDaBenchServer_CPPFLAGS = $(AM_CPPFLAGS)  -g
SoDaBenchServer_LDADD = $(AM_BOOSTLIBFLAGS)  -lrt
SoDaBenchServer_CXXFLAGS = -g
SoDaBenchServer_LDFLAGS = -g 

SoDaNFSurvey_CPPFLAGS = $(AM_CPPFLAGS)  -g
SoDaNFSurvey_LDADD = $(AM_BOOSTLIBFLAGS)  -lrt
SoDaNFSurvey_CXXFLAGS = -g
SoDaNFSurvey_LDFLAGS = -g 

