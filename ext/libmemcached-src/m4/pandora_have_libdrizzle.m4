dnl  Copyright (C) 2009 Sun Microsystems
dnl This file is free software; Sun Microsystems
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([_PANDORA_SEARCH_LIBDRIZZLE],[
  AC_REQUIRE([AC_LIB_PREFIX])

  dnl --------------------------------------------------------------------
  dnl  Check for libdrizzle
  dnl --------------------------------------------------------------------
  
  AC_LIB_HAVE_LINKFLAGS(drizzle,,[
    #include <libdrizzle/drizzle_client.h>
  ],[
    drizzle_st drizzle;
    drizzle_version();
  ])
  
  AM_CONDITIONAL(HAVE_LIBDRIZZLE, [test "x${ac_cv_libdrizzle}" = "xyes"])

])

AC_DEFUN([PANDORA_HAVE_LIBDRIZZLE],[
  AC_REQUIRE([_PANDORA_SEARCH_LIBDRIZZLE])
])

AC_DEFUN([PANDORA_REQUIRE_LIBDRIZZLE],[
  AC_REQUIRE([PANDORA_HAVE_LIBDRIZZLE])
  AS_IF([test x$ac_cv_libdrizzle = xno],
      AC_MSG_ERROR([libdrizzle is required for ${PACKAGE}]))
])

AC_DEFUN([PANDORA_LIBDRIZZLE_NOVCOL],[
  AC_CACHE_CHECK([if libdrizzle still has virtual columns],
    [pandora_cv_libdrizzle_vcol],
    [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <libdrizzle/drizzle.h>
int foo= DRIZZLE_COLUMN_TYPE_DRIZZLE_VIRTUAL;
    ]])],
    [pandora_cv_libdrizzle_vcol=yes],
    [pandora_cv_libdrizzle_vcol=no])])
  AS_IF([test "$pandora_cv_libdrizzle_vcol" = "yes"],[
    AC_MSG_ERROR([Your version of libdrizzle is too old. ${PACKAGE} requires at least version 0.4])
  ])
])
