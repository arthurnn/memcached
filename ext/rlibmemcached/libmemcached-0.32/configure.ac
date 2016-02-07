# libmemcached
# Copyright (C) 2008 Brian Aker, Monty Taylor
# All rights reserved.
#
# Use and distribution licensed under the BSD license.  See
# the COPYING file in this directory for full text.

AC_PREREQ(2.59)
AC_INIT([libmemcached],[0.32],[http://tangent.org/552/libmemcached.html])
AC_CONFIG_SRCDIR([libmemcached/memcached.c])
AC_CONFIG_AUX_DIR(config)
AM_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

PANDORA_CANONICAL_TARGET

#shared library versioning
MEMCACHED_LIBRARY_VERSION=2:0:0
#                         | | |
#                  +------+ | +---+
#                  |        |     |
#                 current:revision:age
#                  |        |     |
#                  |        |     +- increment if interfaces have been added
#                  |        |        set to zero if interfaces have been removed or changed
#                  |        +- increment if source code has changed
#                  |           set to zero if current is incremented
#                  +- increment if interfaces have been added, removed or changed
AC_SUBST(MEMCACHED_LIBRARY_VERSION)
MEMCACHEDUTIL_LIBRARY_VERSION=0:0:0
AC_SUBST(MEMCACHEDUTIL_LIBRARY_VERSION)


# libmemcached versioning when linked with GNU ld.
if test "$lt_cv_prog_gnu_ld" = "yes"
then
    LD_VERSION_SCRIPT="-Wl,--version-script=\$(top_srcdir)/libmemcached/libmemcached.ver"
    LD_UTIL_VERSION_SCRIPT="-Wl,--version-script=\$(top_srcdir)/libmemcachedutil/libmemcachedutil.ver"
fi
AC_SUBST(LD_VERSION_SCRIPT)
AC_SUBST(LD_UTIL_VERSION_SCRIPT)


#--------------------------------------------------------------------
# Check for libpthread
#--------------------------------------------------------------------

ACX_PTHREAD(,AC_MSG_ERROR(could not find libpthread))
LIBS="${PTHREAD_LIBS} ${LIBS}"
CFLAGS="${PTHREAD_CFLAGS} ${CFLAGS}"
CC="$PTHREAD_CC"


AC_SEARCH_LIBS(getopt_long, gnugetopt)
AC_SEARCH_LIBS(socket, socket)
AC_SEARCH_LIBS(gethostbyname, nsl)

save_LIBS="$LIBS"
LIBS="$LIBS -lm"
AC_LINK_IFELSE(
  [AC_LANG_PROGRAM(
    [[
#include <stdlib.h>
    ]],[[
      float f= floorf((float) 1.0);
    ]],
    [LIBM="-lm"],[LIBM=""])])

AC_SUBST(LIBM)
LIBS="$save_LIBS"

AC_SEARCH_LIBS(floorf, m)

dnl Specialty checks
DETECT_BYTEORDER
ENABLE_UTILLIB
SETSOCKOPT_SANITY
ENABLE_HSIEH_HASH
PROTOCOL_BINARY_TEST
WITH_MEMCACHED
ENABLE_DEPRECATED
PANDORA_HAVE_SASL

dnl The sasl functions should only be visible if we build with sasl support
AS_IF([test "x$ac_cv_sasl" = "xyes"],
       [LIBMEMCACHED_WITH_SASL_SUPPORT="#define LIBMEMCACHED_WITH_SASL_SUPPORT 1"])
AC_SUBST(LIBMEMCACHED_WITH_SASL_SUPPORT)

AC_CONFIG_FILES([
  Makefile
  clients/Makefile
  tests/Makefile
  libmemcached/Makefile
  libmemcached/memcached_configure.h
  libmemcachedutil/Makefile
  support/Makefile
  support/libmemcached.pc
  support/libmemcached.spec
  support/libmemcached-fc.spec
  ])
AC_OUTPUT

echo "---"
echo "Configuration summary for $PACKAGE_NAME version $VERSION"
echo ""
echo "   * Installation prefix:       $prefix"
echo "   * System type:               $host_vendor-$host_os"
echo "   * Host CPU:                  $host_cpu"
echo "   * C Compiler:                $CC_VERSION"
echo "   * Assertions enabled:        $ac_cv_assert"
echo "   * Debug enabled:             $with_debug"
echo "   * Warnings as failure:       $ac_cv_warnings_as_errors"
AS_IF([ test "x${ac_cv_sasl}" = "xyes" ],[
echo "   * With SASL support:         $LIBSASL"
], [
echo "   * Without SASL support"
])
echo ""
echo "---"

