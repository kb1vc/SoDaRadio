#
# SYNOPSIS
#
#  AX_FFTW3
#
# DESCRIPTION
#
#   Test for ilbuhd library for the USRP Hardware Device
#
#   This macro sets
#
#     HAV_LIBFFTW3
#

AC_DEFUN([AX_FFTW3],
[
  have_fftw3=no
  AC_SEARCH_LIBS([fftw_malloc],[fftw3],[have_fftw3=yes])

  if test "x${have_fftw3}" = xyes; then
    AC_CHECK_HEADERS([fftw3.h],[], [have_fftw3=no])
  fi

  if test "x${have_fftw3}" = xno; then
    AC_MSG_WARN([
    ------------------------------------------
     Unable to find libfftw3 and header
     fftw3.h on this system. That's not good.
    ------------------------------------------
    ])
  fi
])