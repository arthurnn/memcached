EXTRA_DIST= libmemcachedutil.ver

lib_LTLIBRARIES=

if BUILD_LIBMEMCACHEDUTIL
lib_LTLIBRARIES+= libmemcachedutil.la
endif

libmemcachedutil_la_SOURCES= memcached_pool.c
libmemcachedutil_la_LDFLAGS= -version-info $(MEMCACHEDUTIL_LIBRARY_VERSION) $(LD_UTIL_VERSION_SCRIPT)
libmemcachedutil_la_LIBADD= ${top_builddir}/libmemcached/libmemcached.la
