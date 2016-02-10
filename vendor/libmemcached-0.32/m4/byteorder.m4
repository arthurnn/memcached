AC_DEFUN([DETECT_BYTEORDER],
[
    AC_MSG_CHECKING([for htonll])
    have_htoll="no"
    AC_RUN_IFELSE([
       AC_LANG_PROGRAM([
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
       ], [
          return htonll(0);
       ])            
    ], [
      have_htoll="yes"
      AC_DEFINE([HAVE_HTONLL], [1], [Have ntohll])
    ])

    AC_MSG_RESULT([$have_htoll])
    AM_CONDITIONAL([BUILD_BYTEORDER],[test "x$have_htoll" = "xno"])
    AC_MSG_CHECKING([byteorder])
    have_htoll="no"
    AC_RUN_IFELSE([
       AC_LANG_PROGRAM([
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
       ], [
if (htonl(5) != 5) {
   return 1;
}
       ])            
    ], [
       AC_MSG_RESULT([big endian])
       AC_DEFINE([BYTEORDER_BIG_ENDIAN], [1], [Enable big endian byteorder])
    ], [
       AC_MSG_RESULT([little endian])
       AC_DEFINE([BYTEORDER_LITTLE_ENDIAN], [1], [Enable little endian byteorder])
    ])
])

