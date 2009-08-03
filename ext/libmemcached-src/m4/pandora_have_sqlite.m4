dnl  Copyright (C) 2009 Sun Microsystems
dnl This file is free software; Sun Microsystems
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.


AC_DEFUN([_PANDORA_SEARCH_SQLITE],[
  AC_REQUIRE([AC_LIB_PREFIX])

  AC_LIB_HAVE_LINKFLAGS(sqlite3,,[
    #include <stdio.h>
    #include <sqlite3.h>
  ],[
    sqlite3 *db;
    sqlite3_open(NULL, &db);
  ])

  AM_CONDITIONAL(HAVE_LIBSQLITE3, [test "x${ac_cv_libsqlite3}" = "xyes"])

])

AC_DEFUN([PANDORA_HAVE_SQLITE],[
  AC_REQUIRE([_PANDORA_SEARCH_SQLITE])
])
