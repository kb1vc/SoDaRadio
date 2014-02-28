# ===========================================================================
# based on  http://www.gnu.org/software/autoconf-archive/ax_check_zlib.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_UHD([action-if-found], [action-if-not-found])
#
# DESCRIPTION
#
#   This macro searches for an installed libuhd library. If nothing was
#   specified when calling configure, it searches first in /usr/local and
#   then in /usr, /opt/local and /sw. If the --with-uhd=DIR is specified,
#   it will try to find it in DIR/include/zlib.h and DIR/lib/libuhd.a. If
#   --without-uhd is specified, the library is not searched at all.
#
#   If either the header file (uhd/device.hpp) or the library (libuhd) is not found,
#   shell commands 'action-if-not-found' is run. If 'action-if-not-found' is
#   not specified, the configuration exits on error, asking for a valid uhd
#   installation directory.
#
#   If both header file and library are found, shell commands
#   'action-if-found' is run. If 'action-if-found' is not specified, the
#   default action appends '-I${UHD_HOME}/include' to CPFLAGS, appends
#   '-L${UHD_HOME}/lib' to LDFLAGS, prepends '-lz' to LIBS, and calls
#   AC_DEFINE(HAVE_UHD). You should use autoheader to include a definition
#   for this symbol in a config.h file. Sample usage in a C/C++ source is as
#   follows:
#
#     #ifdef HAVE_UHD
#     #include <uhd/device.hpp>
#     #endif /* HAVE_UHD */
#
# LICENSE
#
#   Copyright (c) 2008 Loic Dachary <loic@senga.org>
#   Copyright (c) 2010 Bastien Chevreux <bach@chevreux.org>
#
#   This program is free software; you can redistribute it and/or modify it
#   under the terms of the GNU General Public License as published by the
#   Free Software Foundation; either version 2 of the License, or (at your
#   option) any later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
#   Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program. If not, see <http://www.gnu.org/licenses/>.
#
#   As a special exception, the respective Autoconf Macro's copyright owner
#   gives unlimited permission to copy, distribute and modify the configure
#   scripts that are the output of Autoconf when processing the Macro. You
#   need not follow the terms of the GNU General Public License when using
#   or distributing such scripts, even though portions of the text of the
#   Macro appear in them. The GNU General Public License (GPL) does govern
#   all other use of the material that constitutes the Autoconf Macro.
#
#   This special exception to the GPL applies to versions of the Autoconf
#   Macro released by the Autoconf Archive. When you make and distribute a
#   modified version of the Autoconf Macro, you may extend this special
#   exception to the GPL to apply to your modified version as well.

#serial 14

AC_DEFUN([AX_UHD],
#
# Handle user hints
#
[AC_MSG_CHECKING(if uhd is wanted)
uhd_places="/usr/local /usr /opt/local /sw"
AC_ARG_WITH([uhd],
[  --with-uhd=DIR         root directory path of uhd installation @<:@defaults to
                          /usr/local or /usr if not found in /usr/local@:>@
  --without-uhd          to disable uhd usage completely],
[if test "$withval" != no ; then
  AC_MSG_RESULT(yes)
  if test -d "$withval"
  then
    uhd_places="$withval $uhd_places"
  else
    AC_MSG_WARN([Sorry, $withval does not exist, checking usual places])
  fi
else
  uhd_places=
  AC_MSG_RESULT(no)
fi],
[AC_MSG_RESULT(yes)])

#
# Locate uhd, if wanted
#
if test -n "${uhd_places}"
then
	# check the user supplied or any other more or less 'standard' place:
	#   Most UNIX systems      : /usr/local and /usr
	#   MacPorts / Fink on OSX : /opt/local respectively /sw
	for UHD_HOME in ${uhd_places} ; do
	  if test -f "${UHD_HOME}/include/uhd/device.hpp"; then break; fi
	  UHD_HOME=""
	done
  UHD_OLD_LDFLAGS=$LDFLAGS
  UHD_OLD_CPPFLAGS=$CPPFLAGS
  if test -n "${UHD_HOME}"; then
        LDFLAGS="$LDFLAGS -L${UHD_HOME}/lib"
        CPPFLAGS="$CPPFLAGS -I${UHD_HOME}/include"
  fi

  AC_CHECK_FILE([${UHD_HOME}/lib/libuhd.so], [uhd_cv_libuhd=yes], [uhd_cv_libuhd=no])
  AC_CHECK_FILE([${UHD_HOME}/include/uhd/device.hpp], [uhd_cv_uhd_h=yes], [uhd_cv_uhd_h=no])

  if test "$uhd_cv_libuhd" = "yes" && test "$uhd_cv_uhd_h" = "yes"
  then
    #
    # If both library and header were found, action-if-found
    #
    m4_ifblank([$1],[
                CPPFLAGS="$CPPFLAGS -I${UHD_HOME}/include"
                LDFLAGS="$LDFLAGS -L${UHD_HOME}/lib"
                LIBS="-luhd $LIBS"
                AC_DEFINE([HAVE_UHD], [1],
                          [Define to 1 if you have `uhd' library (-luhd)])
               ],[
                # Restore variables
                LDFLAGS="$UHD_OLD_LDFLAGS"
                CPPFLAGS="$UHD_OLD_CPPFLAGS"
                $1
               ])
  else
    #
    # If either header or library was not found, action-if-not-found
    #
    m4_default([$2],[
                AC_MSG_ERROR([either specify a valid uhd installation with --with-uhd=DIR or disable uhd usage with --without-uhd])
                ])
  fi
fi
])
