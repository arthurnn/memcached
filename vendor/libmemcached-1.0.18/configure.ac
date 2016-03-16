# configure.ac -*- autoconf -*- vim: filetype=config
#
# LibmemcacheD
# Copyright (C) 2011-2013 Data Differential, http://datadifferential.com/
# Copyright (C) 2006-2010 Brian Aker, Monty Taylor, Trond Norbye
# All rights reserved.
#
# Use and distribution licensed under the BSD license.  See
# the COPYING file in this directory for full text.

m4_include([version.m4])

AC_PREREQ([2.61])
AC_INIT([libmemcached],VERSION_NUMBER,[http://libmemcached.org/])

# Setup the compilers early on
AC_PROG_CC([cc gcc clang])
AC_PROG_CXX([c++ g++ clang++])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_HOST
AC_ARG_PROGRAM
AC_USE_SYSTEM_EXTENSIONS

AM_INIT_AUTOMAKE([1.11 color-tests -Wno-portability subdir-objects foreign tar-ustar])

AC_CONFIG_HEADERS([mem_config.h:mem_config.in])dnl Keep filename to 8.3 for MS-DOS.
AC_SUBST([AUTOHEADER_FILE],[mem_config.h])
AC_CONFIG_SRCDIR([libmemcached/memcached.cc])

#shared library versioning
MEMCACHED_UTIL_LIBRARY_VERSION=2:0:0
MEMCACHED_PROTOCAL_LIBRARY_VERSION=0:0:0
MEMCACHED_LIBRARY_VERSION=11:0:0
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
AC_SUBST([MEMCACHED_UTIL_LIBRARY_VERSION])
AC_SUBST([MEMCACHED_PROTOCAL_LIBRARY_VERSION])
AC_SUBST([MEMCACHED_LIBRARY_VERSION])

AX_HEX_VERSION([LIBMEMCACHED],[$VERSION])
AX_HEX_VERSION([LIBHASHKIT],[1.0.0])
AC_SUBST([RPM_RELEASE],[1])

m4_include([libhashkit/include.m4])

LT_PREREQ([2.2])
LT_INIT([dlopen])
LT_LANG([C++])
LT_LIB_M

AC_SUBST([lt_cv_dlopen_libs])


AC_PROG_CC_C99
AS_IF([test "x${ac_cv_prog_cc_c99}" == "xno"],[AC_MSG_ERROR([No c99 compatible compiler found])])

AC_DEFUN([CHECK_CXX0X],[
          AC_LANG_PUSH([C++])
          AX_CHECK_COMPILE_FLAG([-std=c++0x],[
                                 CXX="$CXX -std=c++0x"])
          AC_LANG_POP([C++])
          ])
CHECK_CXX0X

AX_ASSERT

AX_PLATFORM
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_DEFUN([LIBMEMCACHED_PROTOCOL],
         [AC_ARG_ENABLE([libmemcachedprotocol],
                        [AS_HELP_STRING([--enable-libmemcachedprotocol],
                                        [Enable libmemcachedprotocol])],
                                        [ax_enable_libmemcachedprotocol=yes],
                                        [ax_enable_libmemcachedprotocol=no])

         AM_CONDITIONAL([BUILD_LIBMEMCACHED_PROTOCOL],[test "$ax_enable_libmemcachedprotocol" = "yes"])

         AC_MSG_CHECKING([for libmemcachedprotocol])
         AC_MSG_RESULT([$ax_enable_libmemcachedprotocol])
         ])
LIBMEMCACHED_PROTOCOL

# Adding support for libtest
m4_include([libtest/yatl.m4])

CHECK_FOR_GEARMAND
CHECK_FOR_LIBGEARMAND

AM_CONDITIONAL([BUILDING_LIBMEMCACHED],[true])
AM_CONDITIONAL([HAVE_LIBMEMCACHED],[false])
AM_CONDITIONAL([HAVE_LIBDRIZZLE],[false])
AC_DEFINE([HAVE_LIBMEMCACHED],[1],[Enables libmemcached Support])

AM_CONDITIONAL([BUILDING_GEARMAN],[false])

# Specialty checks
AX_CXX_CINTTYPES
CONFIG_EXTRA
DETECT_BYTEORDER
SETSOCKOPT_SANITY
ENABLE_HSIEH_HASH
ENABLE_MURMUR_HASH
ENABLE_FNV64_HASH
ENABLE_MEMASLAP
PROTOCOL_BINARY_TEST
ENABLE_DEPRECATED

# Checks for programs.
AC_CHECK_PROGS([DPKG_GENSYMBOLS],[dpkg-gensymbols])
AC_CHECK_PROGS([LEX],['flex'],[:])
AC_CHECK_PROGS([PERL],[perl])
AC_CHECK_PROGS([VALGRIND],['valgrind'])
AC_CHECK_PROGS([WINE],['wine'])
AC_CHECK_PROGS([YACC],['bison --warnings=all'],[:])
AC_CHECK_PROGS([RPMBUILD],[rpmbuild --nocheck])
AC_CHECK_PROGS([RPMDEV_SETUPTREE],[rpmdev-setuptree])
AC_CHECK_PROGS([RPM],[rpm])
AC_PROG_AWK
AC_PROG_MKDIR_P
AC_PROG_SED
AX_PROG_MEMCACHED
AX_PROG_SPHINX_BUILD(,[AC_MSG_WARN([sphinx-build version 1.0 or greater is required to build man pages])])
AX_WITH_PROG([LCOV],[lcov])
AX_WITH_PROG([LCOV_GENHTML],[genhtml])
AC_DEFINE([HAVE_MEMCACHED_SASL_BINARY],[0],[If Memcached binary with sasl support is available])

# Checks for libraries.
AC_SEARCH_LIBS([getopt_long],[gnugetopt])
AC_SEARCH_LIBS([gethostbyname],[nsl])

# Checks for header files.
AC_CHECK_HEADERS_ONCE([arpa/inet.h])
AC_CHECK_HEADERS_ONCE([errno.h])
AC_CHECK_HEADERS_ONCE([execinfo.h])
AC_CHECK_HEADERS_ONCE([fcntl.h])
AC_CHECK_HEADERS_ONCE([features.h])
AC_CHECK_HEADERS_ONCE([fnmatch.h])
AC_CHECK_HEADERS_ONCE([inttypes.h])
AC_CHECK_HEADERS_ONCE([libintl.h])
AC_CHECK_HEADERS_ONCE([limits.h])
AC_CHECK_HEADERS_ONCE([mach/mach.h])
AC_CHECK_HEADERS_ONCE([malloc.h])
AC_CHECK_HEADERS_ONCE([math.h])
AC_CHECK_HEADERS_ONCE([netdb.h])
AC_CHECK_HEADERS_ONCE([netinet/in.h])
AC_CHECK_HEADERS_ONCE([netinet/tcp.h])
AC_CHECK_HEADERS_ONCE([poll.h])
AC_CHECK_HEADERS_ONCE([pthread.h])
AC_CHECK_HEADERS_ONCE([spawn.h])
AC_CHECK_HEADERS_ONCE([stdarg.h])
AC_CHECK_HEADERS_ONCE([stddef.h])
AC_CHECK_HEADERS_ONCE([stdio.h])
AC_CHECK_HEADERS_ONCE([stdlib.h])
AC_CHECK_HEADERS_ONCE([sys/socket.h])
AC_CHECK_HEADERS_ONCE([sys/sysctl.h])
AC_CHECK_HEADERS_ONCE([sys/time.h])
AC_CHECK_HEADERS_ONCE([sys/types.h])
AC_CHECK_HEADERS_ONCE([sys/un.h])
AC_CHECK_HEADERS_ONCE([sys/wait.h])
AC_CHECK_HEADERS_ONCE([syslog.h])
AC_CHECK_HEADERS_ONCE([time.h])
AC_CHECK_HEADERS_ONCE([unistd.h])

# Windows only
AC_CHECK_HEADERS_ONCE([winsock2.h])
AC_CHECK_HEADERS_ONCE([ws2tcpip.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CHECK_FUNCS([alarm])
AC_CHECK_FUNCS([atexit])
AC_CHECK_FUNCS([clock_gettime])
AC_CHECK_FUNCS([dup2])
AC_CHECK_FUNCS([fcntl])
AC_CHECK_FUNCS([floor])
AC_CHECK_FUNCS([getcwd])
AC_CHECK_FUNCS([getline])
AC_CHECK_FUNCS([gettimeofday])
AC_CHECK_FUNCS([inet_ntoa])
AC_CHECK_FUNCS([memchr])
AC_CHECK_FUNCS([memmove])
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([pipe2])
AC_CHECK_FUNCS([putenv])
AC_CHECK_FUNCS([select])
AC_CHECK_FUNCS([setenv])
AC_CHECK_FUNCS([sigignore])
AC_CHECK_FUNCS([socket])
AC_CHECK_FUNCS([sqrt])
AC_CHECK_FUNCS([strcasecmp])
AC_CHECK_FUNCS([strchr])
AC_CHECK_FUNCS([strdup])
AC_CHECK_FUNCS([strerror])
AC_CHECK_FUNCS([strstr])
AC_CHECK_FUNCS([strtol])
AC_CHECK_FUNCS([strtoul])
AC_CHECK_FUNCS([strtoull])
AC_FUNC_ALLOCA
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_STRERROR_R

AC_CHECK_HEADER([umem.h],
                [AC_DEFINE([HAVE_UMEM_H],[1],[Define this if you have umem.h])
                build_cache=no],
                [build_cache=yes])

AM_CONDITIONAL([BUILD_CACHE],[test "x$build_cache" = "xyes"])

AX_COMPILER_VENDOR

AC_CHECK_TYPES([in_port_t],,,[[
                #ifdef HAVE_SYS_TYPES_H
                # include <sys/types.h>
                #endif
                #ifdef HAVE_NETINET_IN_H
                # include <netinet/in.h>
                #endif
                #ifdef HAVE_NETDB_H
                # include <netdb.h>
                #endif]])
AC_CHECK_TYPES([ptrdiff_t])
AC_HEADER_STDBOOL
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

AC_C_BIGENDIAN
AC_C_CONST
AC_C_INLINE
AC_C_VOLATILE
AC_C_RESTRICT

AX_CXX_GCC_ABI_DEMANGLE

AX_LIBEVENT

AC_CHECK_LIB([rt],[clock_gettime],
             [AC_SUBST([RT_LIB],[-lrt])
             AC_DEFINE([HAVE_CLOCK_GETTIME],[1],[Have clock_gettime])],
             [AC_DEFINE([HAVE_CLOCK_GETTIME],[0],[Have clock_gettime])])


AC_CHECK_HEADERS([atomic.h])
AS_IF([test "x$ac_cv_header_atomic_h" = "xyes"],
      [AC_CHECK_FUNCS([atomic_add_64])
      AC_CHECK_FUNCS([atomic_add_32])
      AS_IF([test "x$ac_cv_func_atomic_add_64" = "xyes" -a "x$ac_cv_func_atomic_add_32" = "xyes"],
            [AC_DEFINE([USE_ATOMIC_H],[1],[Define to true if you want to use functions from atomic.h])])])

AC_DEFINE([HAVE_LIBDRIZZLE],[0],[Support for libdrizzle])
AC_DEFINE([HAVE_DRIZZLED_BINARY],[0],[Support for DrizzleD])
AC_DEFINE([DRIZZLED_BINARY],[0],[Support for DrizzleD])
AC_DEFINE([GEARMAND_BLOBSLAP_WORKER],[0],[Support for Gearman Blobslap worker])
AC_DEFINE([HAVE_LIBPQ],[0],[Support for Postgres])
AC_DEFINE([HAVE_LIBCURL],[0],[Support for libcurl])

# Check for the requirements for running memcached with less privileges
# than the default privilege set. On Solaris we need setppriv and priv.h
# If you want to add support for other platforms you should check for
# your requirements, define HAVE_DROP_PRIVILEGES, and make sure you add
# the source file containing the implementation into memcached_SOURCE
# in Makefile.am
AC_CHECK_FUNCS([setppriv],
               [AC_CHECK_HEADER([priv.h],
                                [AC_DEFINE([HAVE_DROP_PRIVILEGES],
                                           [1],
                                           [Define this if you have an implementation of drop_privileges()])
                                build_solaris_privs=yes])
               ])

AM_CONDITIONAL([BUILD_POLL],[test "x$ac_cv_header_poll_h" = "xno"])
AM_CONDITIONAL([BUILD_WIN32_WRAPPERS],[test "x$ac_cv_header_winsock2_h" = "xyes"])
AS_IF([test "x$ac_cv_header_winsock2_h" = "xyes"],
      [AM_LDFLAGS="$AM_LDFLAGS -lws2_32"
       AM_CFLAGS="$AM_CFLAGS $NO_WERROR"
       AM_CXXFLAGS="$AM_CXXFLAGS $NO_WERROR"
       ])
SOCKET_SEND_FLAGS

AX_UUID_GENERATE_TIME_SAFE

AC_DEFUN([CHECK_FOR_PTHREAD_AND_SASL],
         [AC_LANG_PUSH([C++])
         AX_PTHREAD(, [AC_MSG_ERROR([could not find libpthread])])
         AX_PTHREAD_TIMEDJOIN_NP
         AC_LANG_POP([C++])

         # The sasl functions should only be visible if we build with sasl support
         #
         AX_SASL_OPTION
         AS_IF([test "x$ac_enable_sasl" = xyes],
               [LIBMEMCACHED_WITH_SASL_SUPPORT="#define LIBMEMCACHED_WITH_SASL_SUPPORT 1"],
               [LIBMEMCACHED_WITH_SASL_SUPPORT="#define LIBMEMCACHED_WITH_SASL_SUPPORT 0"])
         AC_SUBST([LIBMEMCACHED_WITH_SASL_SUPPORT])
         ])
CHECK_FOR_PTHREAD_AND_SASL

# backtrace(), others require shared builds
AC_DEFUN([__ENABLE_SHARED],
         [AC_REQUIRE([AX_PLATFORM])
         AS_IF([test "x${TARGET_WINDOWS}" = "xtrue"],[enable_shared=no])
         AS_IF([test "x$enable_shared" = "xyes"],
               [AC_DEFINE([HAVE_SHARED_ENABLED],[1],[Enable code which requires shared library support. Like backtrace().])])])
AM_CONDITIONAL([SHARED_ENABLED],[test "x$enable_shared" = "xyes"])

PANDORA_ENABLE_DTRACE
HAVE_GCC_ATOMICS
AX_ENDIAN
AX_ASSERT
AX_HARDEN_COMPILER_FLAGS

AX_AM_JOBSERVER([yes])

AC_CONFIG_FILES([Makefile
                 docs/conf.py
                 libmemcached-1.0/configure.h
                 support/libmemcached.pc
                 support/libmemcached.spec
                 ])
AC_OUTPUT

echo "---"
echo "Configuration summary for $PACKAGE_NAME version $VERSION"
echo ""
echo "   * Installation prefix:       $prefix"
echo "   * System type:               $host_vendor-$host_os"
echo "   * Host CPU:                  $host_cpu"
echo "   * C Compiler:                $CC_VERSION"
echo "   * C Flags:                   $CFLAGS"
echo "   * C++ Compiler:              $CXX_VERSION"
echo "   * C++ Flags:                 $CXXFLAGS"
echo "   * CPP Flags:                 $CPPFLAGS"
echo "   * LIB Flags:                 $LIB"
echo "   * Assertions enabled:        $ax_enable_assert"
echo "   * Debug enabled:             $ax_enable_debug"
echo "   * Shared:                    $enable_shared"
echo "   * Warnings as failure:       $ac_cv_warnings_as_errors"
echo "   * SASL support:              $ac_enable_sasl"
echo "   * make -j:                   $enable_jobserver"
echo "   * VCS checkout:              $ac_cv_vcs_checkout"
echo ""
echo "---"

case "$host_os" in
  *freebsd*)
    echo "*****"
    echo "*"
    echo "*  NOTE: You are on FreeBSD. BSD make will not work."
    echo "*"
    echo "*   use 'gmake' To build libmemcached"
    echo "*"
    echo "*****"
  ;;
esac

