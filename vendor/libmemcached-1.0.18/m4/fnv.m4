dnl ---------------------------------------------------------------------------
dnl Macro: ENABLE_FNV64_HASH
dnl ---------------------------------------------------------------------------
AC_DEFUN([ENABLE_FNV64_HASH],
  [AC_ARG_ENABLE([fnv64_hash],
    [AS_HELP_STRING([--disable-fnv64_hash],
      [build with support for fnv64 hashing. @<:@default=on@:>@])],
    [ac_cv_enable_fnv64_hash=no],
    [ac_cv_enable_fnv64_hash=yes])

  AS_IF([test "$ac_cv_enable_fnv64_hash" = "yes"],
        [AC_DEFINE([HAVE_FNV64_HASH], [1], [Enables fnv64 hashing support])])
])
dnl ---------------------------------------------------------------------------
dnl End Macro: ENABLE_FNV64_HASH
dnl ---------------------------------------------------------------------------
