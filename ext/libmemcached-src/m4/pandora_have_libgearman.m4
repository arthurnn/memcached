dnl  Copyright (C) 2009 Sun Microsystems
dnl This file is free software; Sun Microsystems
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([_PANDORA_SEARCH_LIBGEARMAN],[
  AC_REQUIRE([AC_LIB_PREFIX])

  dnl --------------------------------------------------------------------
  dnl  Check for libgearman
  dnl --------------------------------------------------------------------

  AC_LIB_HAVE_LINKFLAGS(gearman,, 
    [#include <libgearman/gearman.h>],[ 
      gearman_client_st gearman_client; 
      gearman_version(); 
    ]) 

  AM_CONDITIONAL(HAVE_LIBGEARMAN, [test "x${ac_cv_libgearman}" = "xyes"])
  
])

AC_DEFUN([PANDORA_HAVE_LIBGEARMAN],[
  AC_REQUIRE([_PANDORA_SEARCH_LIBGEARMAN])
])

AC_DEFUN([PANDORA_REQUIRE_LIBGEARMAN],[
  AC_REQUIRE([PANDORA_HAVE_LIBGEARMAN])
  AS_IF([test x$ac_cv_libgearman = xno],
      AC_MSG_ERROR([libgearman is required for ${PACKAGE}]))
])
