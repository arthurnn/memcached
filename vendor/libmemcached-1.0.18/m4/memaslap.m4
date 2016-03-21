AC_DEFUN([ENABLE_MEMASLAP],
  [AC_ARG_ENABLE([memaslap],
    [AS_HELP_STRING([--enable-memaslap],
      [build with memaslap tool. @<:@default=off@:>@])],
    [ac_cv_enable_memaslap=yes],
    [ac_cv_enable_memaslap=no])

  AM_CONDITIONAL([BUILD_MEMASLAP], [test "$ac_cv_enable_memaslap" = "yes"])
])
