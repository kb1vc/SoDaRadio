#
# SYNOPSIS
#
#  AX_FFTW3F
#
# DESCRIPTION
#
#   Test for fftw3f library (fastest fourier transform in the west)
#
#   This macro sets
#
#     HAVE_LIBFFTW3F
#

AC_DEFUN([AX_FFTW3F],
[
  have_fftw3f=no
  AC_SEARCH_LIBS([fftwf_malloc],[fftw3f],[have_fftw3f=yes])

  if test "x${have_fftw3f}" = xyes; then
    AC_CHECK_HEADERS([fftw3.h],[], [have_fftw3f=no])
  fi

  if test "x${have_fftw3f}" = xno; then
    AC_MSG_WARN([
    ------------------------------------------
     Unable to find libfftw3f and header
     fftw3.h on this system. That's not good.
    ------------------------------------------
    ])
  fi
])