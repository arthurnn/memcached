AC_INIT(clients/memcat.c)
AC_CONFIG_AUX_DIR(config)
AM_CONFIG_HEADER(libmemcached/libmemcached_config.h)
AC_CANONICAL_SYSTEM

MEMCACHED_LIBRARY_NAME=libmemcached 

#release versioning
MEMCACHED_MAJOR_VERSION=0
MEMCACHED_MINOR_VERSION=25
MEMCACHED_MICRO_VERSION=0

#API version
MEMCACHED_API_VERSION=1.0
AC_SUBST(MEMCACHED_API_VERSION)

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

PACKAGE=$MEMCACHED_LIBRARY_NAME
AC_SUBST(MEMCACHED_LIBRARY_NAME)

MEMCACHED_VERSION=$MEMCACHED_MAJOR_VERSION.$MEMCACHED_MINOR_VERSION.$MEMCACHED_MICRO_VERSION
MEMCACHED_RELEASE=$MEMCACHED_MAJOR_VERSION.$MEMCACHED_MINOR_VERSION
AC_SUBST(MEMCACHED_RELEASE)
AC_SUBST(MEMCACHED_VERSION)

VERSION=$MEMCACHED_RELEASE

AM_INIT_AUTOMAKE($PACKAGE, $VERSION, no-define)

AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL
AM_SANITY_CHECK
LIBTOOL="$LIBTOOL --preserve-dup-deps"
AC_SUBST(LIBTOOL)dnl
AC_LANG_CPLUSPLUS

sinclude(config/pod2man.m4)
sinclude(config/debug.m4)
sinclude(config/dtrace.m4)
sinclude(config/byteorder.m4)
sinclude(config/64bit.m4)
sinclude(config/protocol_binary.m4)

# We only support GCC and Sun's forte at the moment
CFLAGS="-DMEMCACHED_INTERNAL $CFLAGS"
if test "$GCC" = "yes"
then
  #CFLAGS="-W -std=iso9899:1999 -Wall -Wextra -Wstrict-aliasing -pedantic -Wundef -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -O3 $CFLAGS"
  CFLAGS="-Wall -O3 $CFLAGS"

  if test "$ENABLE_DEBUG" = "yes"
  then
    CFLAGS="$CFLAGS -ggdb -DHAVE_DEBUG"
  fi
else
  CFLAGS="-Xa -xstrconst -mt -D_FORTEC_ $CFLAGS"
  if test "$ENABLE_DEBUG" = "yes"
  then
    CFLAGS="$CFLAGS -DHAVE_DEBUG"
  fi
  DTRACEFLAGS="$DTRACEFLAGS"
fi

LDFLAGS="-lm"
AC_C_CONST
AC_HEADER_TIME
AC_TYPE_SIZE_T
AC_SEARCH_LIBS(getopt_long, gnugetopt)
AC_SEARCH_LIBS(socket, socket)
AC_SEARCH_LIBS(gethostbyname, nsl)
AC_OUTPUT(Makefile clients/Makefile tests/Makefile docs/Makefile libmemcached/Makefile support/Makefile support/libmemcached.pc support/libmemcached.spec)
