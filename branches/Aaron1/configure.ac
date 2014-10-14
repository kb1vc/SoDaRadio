
AC_INIT([SoDa], [3.1])

AC_PREREQ(2.60)
AM_INIT_AUTOMAKE
AC_PROG_CXX
AC_PROG_CC

AM_PROG_CC_C_O

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_FILES([Makefile src/Makefile gui/Makefile gui/adjust_GUI.sh exp/Makefile])

AC_CONFIG_SRCDIR(src/SoDaServer.cxx)

AC_CHECK_FUNCS([sincos])

AC_CHECK_LIB(portaudio, Pa_GetVersion)

AC_CHECK_LIB(asound, snd_pcm_open)

AC_PATH_PROG(svnversioncommand, svnversion)
if test "X$svnversioncommand" = "X" || test `$svnversioncommand -n $srcdir` = "exported"; then
AC_DEFINE_UNQUOTED(SVN_VERSION, ["`cat ${srcdir}/saved_svn_version.txt`"], [repository svn version])
else
AC_DEFINE_UNQUOTED(SVN_VERSION, ["`svnversion -n $srcdir`"], [repository svn version])
`svnversion -n $srcdir > ${srcdir}/saved_svn_version.txt`
fi


DX_HTML_FEATURE(ON)
DX_INIT_DOXYGEN([$PACKAGE_NAME],[Doxyfile],[doc])

AX_UHD

AX_FFTW3F
AX_FFTW3



AX_BOOST_BASE([1.33])
AX_BOOST_DATE_TIME
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_THREAD
AX_BOOST_ASIO
AX_BOOST_SYSTEM
AX_BOOST_REGEX
AX_BOOST_FILESYSTEM
AX_BOOST_SERIALIZATION

AX_UHD

AC_LANG(C++)


AM_OPTIONS_WXCONFIG
reqwx=2.4.0
AM_PATH_WXCONFIG($reqwx, wxWin=1)
if test "$wxWin" != 1; then
	AC_MSG_ERROR([
		wxWidgets must be installed on your system.
 
		Please check that wx-config is in path, the directory
		where wxWidgets libraries are installed (returned by
		'wx-config --libs' or 'wx-config --static --libs' command)
		is in LD_LIBRARY_PATH or equivalent variable and
		wxWidgets version is $reqwx or above.
		])
fi
 
CPPFLAGS="$CPPFLAGS $WX_CPPFLAGS"
CXXFLAGS="$CXXFLAGS $WX_CXXFLAGS_ONLY"
CFLAGS="$CFLAGS $WX_CFLAGS_ONLY"
LIBS="$LIBS $WX_LIBS"

AC_OUTPUT


