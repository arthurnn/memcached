dnl ---------------------------------------------------------------------------
dnl Macro: ENABLE_MURMUR_HASH
dnl ---------------------------------------------------------------------------
AC_DEFUN([ENABLE_MURMUR_HASH],
  [AC_ARG_ENABLE([murmur_hash],
    [AS_HELP_STRING([--disable-murmur_hash],
      [build with support for murmur hashing. @<:@default=on@:>@])],
    [ac_cv_enable_murmur_hash=no],
    [ac_cv_enable_murmur_hash=yes])

  AS_IF([test "$ac_cv_enable_murmur_hash" = "yes"],
        [AC_DEFINE([HAVE_MURMUR_HASH], [1], [Enables murmur hashing support])])

  AM_CONDITIONAL([INCLUDE_MURMUR_SRC], [test "$ac_cv_enable_murmur_hash" = "yes"])
])
dnl ---------------------------------------------------------------------------
dnl End Macro: ENABLE_MURMUR_HASH
dnl ---------------------------------------------------------------------------
