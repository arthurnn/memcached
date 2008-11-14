dnl ---------------------------------------------------------------------------
dnl Macro: DEBUG_TEST
dnl ---------------------------------------------------------------------------
AC_ARG_ENABLE(debug,
    [  --enable-debug      Build with support for the DEBUG.],
    [ 
      AC_DEFINE([HAVE_DEBUG], [1], [Enables DEBUG Support])
      AC_CHECK_PROGS(DEBUG, debug)
      ENABLE_DEBUG="yes" 
      AC_SUBST(DEBUGFLAGS)
      AC_SUBST(HAVE_DEBUG)
    ],
    [
      ENABLE_DEBUG="no" 
    ]
    )
AM_CONDITIONAL([HAVE_DEBUG], [ test "$ENABLE_DEBUG" = "yes" ])
dnl ---------------------------------------------------------------------------
dnl End Macro: DEBUG_TEST
dnl ---------------------------------------------------------------------------
