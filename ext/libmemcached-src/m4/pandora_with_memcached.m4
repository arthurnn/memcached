dnl  Copyright (C) 2009 Sun Microsystems
dnl This file is free software; Sun Microsystems
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([PANDORA_WITH_MEMCACHED],[

  AC_ARG_WITH([memcached],
    [AS_HELP_STRING([--with-memcached],
      [Memcached binary to use for make test])],
    [ac_cv_with_memcached="$withval"],
    [ac_cv_with_memcached=memcached])

  # just ignore the user if --without-memcached is passed.. it is
  # only used by make test
  AS_IF([test "x$withval" = "xno"],[
    ac_cv_with_memcached=memcached
    MEMCACHED_BINARY=memcached
  ],[
    AS_IF([test -f "$withval"],[
      ac_cv_with_memcached=$withval
      MEMCACHED_BINARY=$withval
    ],[
      AC_PATH_PROG([MEMCACHED_BINARY], [$ac_cv_with_memcached], "no")
      AS_IF([test "x$MEMCACHED_BINARY" = "xno"],[
        AC_MSG_ERROR(["could not find memcached binary"])
      ])
    ])
  ])
  AC_DEFINE_UNQUOTED([MEMCACHED_BINARY], "$MEMCACHED_BINARY", 
            [Name of the memcached binary used in make test])

])
