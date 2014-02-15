#
# SYNOPSIS
#
#  AX_UHD
#
# DESCRIPTION
#
#   Test for ilbuhd library for the USRP Hardware Device
#
#   This macro sets
#
#     HAVE_LIBUHD
#

AC_DEFUN([AX_UHD],
[
m4_ifval([], , [AH_CHECK_LIB([uhd])])dnl
AS_VAR_PUSHDEF([ac_Lib], [ac_cv_lib_uhd_multi_usrp])dnl
AC_CACHE_CHECK([whether -luhd exports uhd::get_version_string(void)],[ac_Lib], 
[ac_check_lib_save_LIBS=$LIBS
LIBS="-luhd $LIBS"
AC_LINK_IFELSE([AC_LANG_PROGRAM(
[[#include <string>
namespace uhd { 
  std::string get_version_string(); 
}
]],
[[  std::string foo = uhd::get_version_string();]])],
  [AS_VAR_SET([ac_Lib], [yes])],
  [AS_VAR_SET([ac_Lib], [no])])
LIBS=$ac_check_lib_save_LIBS])
AS_VAR_IF([ac_Lib], [yes],
  [AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_LIBUHD))
  LIBS="-luhd $LIBS"
],
[])dnl
AS_VAR_POPDEF([ac_Lib])dnl
])

