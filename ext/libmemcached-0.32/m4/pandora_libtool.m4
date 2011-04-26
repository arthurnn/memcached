dnl  Copyright (C) 2009 Sun Microsystems
dnl This file is free software; Sun Microsystems
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([PANDORA_LIBTOOL],[
  AC_REQUIRE([AC_PROG_LIBTOOL])
  dnl By requiring AC_PROG_LIBTOOL, we should force the macro system to read
  dnl libtool.m4, where in 2.2 AC_PROG_LIBTOOL is an alias for LT_INIT
  dnl Then, if we're on 2.2, we should have LT_LANG, so we'll call it.
  m4_ifdef([LT_LANG],[
    LT_LANG(C)
    LT_LANG(C++)
  ])
])
