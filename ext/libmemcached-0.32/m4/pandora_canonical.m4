dnl  Copyright (C) 2009 Sun Microsystems
dnl This file is free software; Sun Microsystems
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl Which version of the canonical setup we're using
AC_DEFUN([PANDORA_CANONICAL_VERSION],[0.22])

AC_DEFUN([PANDORA_FORCE_DEPEND_TRACKING],[
  dnl Force dependency tracking on for Sun Studio builds
  AS_IF([test "x${enable_dependency_tracking}" = "x"],[
    enable_dependency_tracking=yes
  ])
])

dnl The standard setup for how we build Pandora projects
AC_DEFUN([PANDORA_CANONICAL_TARGET],[
  AC_REQUIRE([PANDORA_FORCE_DEPEND_TRACKING])
  ifdef([m4_define],,[define([m4_define],   defn([define]))])
  ifdef([m4_undefine],,[define([m4_undefine],   defn([undefine]))])
  m4_define([PCT_ALL_ARGS],[$*])
  m4_define([PCT_USE_GNULIB],[no])
  m4_define([PCT_REQUIRE_CXX],[no])
  m4_define([PCT_IGNORE_SHARED_PTR],[no])
  m4_define([PCT_FORCE_GCC42],[no])
  m4_foreach([pct_arg],[$*],[
    m4_case(pct_arg,
      [use-gnulib], [
        m4_undefine([PCT_USE_GNULIB])
        m4_define([PCT_USE_GNULIB],[yes])
      ],
      [require-cxx], [
        m4_undefine([PCT_REQUIRE_CXX])
        m4_define([PCT_REQUIRE_CXX],[yes])
      ],
      [ignore-shared-ptr], [
        m4_undefine([PCT_IGNORE_SHARED_PTR])
        m4_define([PCT_IGNORE_SHARED_PTR],[yes])
      ],
      [force-gcc42], [
        m4_undefine([PCT_FORCE_GCC42])
        m4_define([PCT_FORCE_GCC42],[yes])
    ])
  ])

  # We need to prevent canonical target
  # from injecting -O2 into CFLAGS - but we won't modify anything if we have
  # set CFLAGS on the command line, since that should take ultimate precedence
  AS_IF([test "x${ac_cv_env_CFLAGS_set}" = "x"],
        [CFLAGS=""])
  AS_IF([test "x${ac_cv_env_CXXFLAGS_set}" = "x"],
        [CXXFLAGS=""])
  
  AC_CANONICAL_TARGET
  
  AM_INIT_AUTOMAKE(-Wall -Werror nostdinc subdir-objects)

  m4_if(PCT_USE_GNULIB,yes,[ gl_EARLY ])
  
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([PANDORA_MAC_GCC42])
  AC_REQUIRE([PANDORA_64BIT])

  dnl Once we can use a modern autoconf, we can use this
  dnl AC_PROG_CC_C99
  AC_PROG_CXX
  AC_PROG_CPP
  AM_PROG_CC_C_O


  gl_USE_SYSTEM_EXTENSIONS
  m4_if(PCT_FORCE_GCC42, [yes], [
    AS_IF([test "$GCC" = "yes"], PANDORA_ENSURE_GCC_VERSION)
  ])

  AC_CHECK_DECL([__SUNPRO_C], [SUNCC="yes"], [SUNCC="no"])
  AC_CHECK_DECL([__ICC], [INTELCC="yes"], [INTELCC="no"])
  AS_IF([test "x$INTELCC" = "xyes"], [enable_rpath=no])

  PANDORA_LIBTOOL

  dnl autoconf doesn't automatically provide a fail-if-no-C++ macro
  dnl so we check c++98 features and fail if we don't have them, mainly
  dnl for that reason
  PANDORA_CHECK_CXX_STANDARD
  m4_if(PCT_REQUIRE_CXX, [yes], [
    AS_IF([test "$ac_cv_cxx_stdcxx_98" = "no"],[
      AC_MSG_ERROR([No working C++ Compiler has been found. ${PACKAGE} requires a C++ compiler that can handle C++98])
    ])
  ])
  
  PANDORA_SHARED_PTR
  m4_if(PCT_IGNORE_SHARED_PTR, [no], [
    AS_IF([test "$ac_cv_shared_ptr_namespace" = "missing"],[
      AC_MSG_WARN([a usable shared_ptr implementation was not found. Let someone know what your platform is.])
    ])
  ])
  
  m4_if(PCT_USE_GNULIB, [yes], [gl_INIT])

  AC_C_BIGENDIAN
  AC_C_CONST
  AC_C_INLINE
  AC_C_VOLATILE
  AC_C_RESTRICT

  AC_HEADER_TIME
  AC_TYPE_SIZE_T
  AC_SYS_LARGEFILE


  PANDORA_CHECK_C_VERSION
  PANDORA_CHECK_CXX_VERSION

  PANDORA_OPTIMIZE

  dnl We need to inject error into the cflags to test if visibility works or not
  save_CFLAGS="${CFLAGS}"
  CFLAGS="${CFLAGS} -Werror"
  gl_VISIBILITY
  CFLAGS="${save_CFLAGS}"

  PANDORA_HEADER_ASSERT

  PANDORA_WARNINGS(PCT_ALL_ARGS)

  PANDORA_ENABLE_DTRACE

  AC_LIB_PREFIX
  PANDORA_HAVE_BETTER_MALLOC

  AC_CHECK_PROGS([DOXYGEN], [doxygen])
  AC_CHECK_PROGS([PERL], [perl])

  AS_IF([test "x${gl_LIBOBJS}" != "x"],[
    AS_IF([test "$GCC" = "yes"],[
      AM_CPPFLAGS="-isystem \$(top_srcdir)/gnulib -isystem \$(top_builddir)/gnulib ${AM_CPPFLAGS}"
    ],[
    AM_CPPFLAGS="-I\$(top_srcdir)/gnulib -I\$(top_builddir)/gnulib ${AM_CPPFLAGS}"
    ])
  ])

  AM_CPPFLAGS="-I\${top_srcdir} -I\${top_builddir} ${AM_CPPFLAGS}"
  AM_CFLAGS="${AM_CFLAGS} ${CC_WARNINGS} ${CC_PROFILING} ${CC_COVERAGE}"
  AM_CXXFLAGS="${AM_CXXFLAGS} ${CXX_WARNINGS} ${CC_PROFILING} ${CC_COVERAGE}"

  AC_SUBST([AM_CFLAGS])
  AC_SUBST([AM_CXXFLAGS])
  AC_SUBST([AM_CPPFLAGS])

])
