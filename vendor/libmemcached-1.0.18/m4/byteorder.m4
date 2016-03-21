AC_DEFUN([DETECT_BYTEORDER],
    [
    AC_REQUIRE([AC_C_BIGENDIAN])
    AC_LANG_PUSH([C++])
    AC_CACHE_CHECK([for htonll], [ac_cv_have_htonll],
      [AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
          [#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
      ], [ return htonll(0) ])],
        [ ac_cv_have_htonll=yes ],
        [ ac_cv_have_htonll=no ])
      ])
    AC_LANG_POP()
    AS_IF([test "x$ac_cv_have_htonll" = "xyes"],[
      AC_DEFINE([HAVE_HTONLL], [1], [Have ntohll])])

    AM_CONDITIONAL([BUILD_BYTEORDER],[test "x$ac_cv_have_htonll" = "xno"])
    ])
