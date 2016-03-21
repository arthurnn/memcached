# serial 1
AC_DEFUN([CHECK_FOR_GEARMAND],
    [AX_WITH_PROG([GEARMAND_BINARY],[gearmand])
    AS_IF([test -f "$ac_cv_path_GEARMAND_BINARY"],
      [AC_DEFINE([HAVE_GEARMAND_BINARY],[1],[If Gearmand binary is available])
      AC_DEFINE_UNQUOTED([GEARMAND_BINARY], "$ac_cv_path_GEARMAND_BINARY", [Name of the gearmand binary used in make test])],
      [AC_DEFINE([HAVE_GEARMAND_BINARY],[0],[If Gearmand binary is available])
      AC_DEFINE([GEARMAND_BINARY],[0],[Name of the gearmand binary used in make test])])
    ])
