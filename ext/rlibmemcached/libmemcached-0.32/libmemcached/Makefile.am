EXTRA_DIST = libmemcached_probes.d memcached/README.txt libmemcached.ver \
             memcached_configure.h.in

EXTRA_HEADERS =
BUILT_SOURCES=

noinst_HEADERS = libmemcached_probes.h \
                 memcached_io.h \
                 memcached_internal.h \
                 common.h \
                 memcached/protocol_binary.h

pkginclude_HEADERS= memcached.h \
		    memcached.hpp \
                    memcached_configure.h \
		    memcached_constants.h \
		    memcached_exist.h \
		    memcached_get.h \
		    memcached_result.h \
				memcached_sasl.h \
		    memcached_server.h \
		    memcached_storage.h \
		    memcached_string.h \
		    memcached_touch.h \
		    memcached_types.h \
		    memcached_watchpoint.h \
		    visibility.h


if BUILD_LIBMEMCACHEDUTIL
pkginclude_HEADERS+= memcached_util.h memcached_pool.h
endif

lib_LTLIBRARIES = libmemcached.la
noinst_LTLIBRARIES = libmemcachedcallbacks.la
libmemcachedcallbacks_la_CFLAGS = ${AM_CFLAGS} ${NO_STRICT_ALIASING}
libmemcachedcallbacks_la_SOURCES = memcached_callback.c

libmemcached_la_CFLAGS= ${AM_CFLAGS} ${NO_CONVERSION}
libmemcached_la_SOURCES = crc.c \
			  memcached.c \
			  memcached_auto.c \
			  memcached_analyze.c \
			  memcached_behavior.c \
			  memcached_connect.c \
			  memcached_delete.c \
			  memcached_do.c \
			  memcached_dump.c \
			  memcached_exist.c \
			  memcached_fetch.c \
			  memcached_flush.c \
			  memcached_get.c \
			  memcached_touch.c \
			  memcached_hash.c \
			  memcached_hosts.c \
			  memcached_io.c \
			  memcached_purge.c \
				memcached_flush_buffers.c \
			  md5.c \
			  memcached_key.c \
			  memcached_quit.c \
			  memcached_parse.c \
			  memcached_response.c \
			  memcached_result.c \
			  memcached_server.c \
			  memcached_storage.c \
			  memcached_string.c \
			  memcached_stats.c \
			  memcached_strerror.c \
			  memcached_verbosity.c \
			  memcached_version.c \
			  murmur_hash.c \
			  jenkins_hash.c \
				memcached_allocators.c


if INCLUDE_HSIEH_SRC
libmemcached_la_SOURCES += hsieh_hash.c
endif

if BUILD_BYTEORDER
libmemcached_la_SOURCES += byteorder.c
endif

libmemcached_la_DEPENDENCIES= libmemcachedcallbacks.la
libmemcached_la_LIBADD= $(LIBM) libmemcachedcallbacks.la
libmemcached_la_LDFLAGS = -version-info $(MEMCACHED_LIBRARY_VERSION) $(LD_VERSION_SCRIPT) $(LIBM)

if HAVE_SASL
libmemcached_la_LDFLAGS+= $(LTLIBSASL)
libmemcached_la_SOURCES += memcached_sasl.c
endif

if HAVE_DTRACE
BUILT_SOURCES+= dtrace_probes.h
libmemcached_la_SOURCES += libmemcached_probes.d
endif

if DTRACE_NEEDS_OBJECTS
libmemcached_la_DEPENDENCIES += libmemcached_probes.o
endif

SUFFIXES= .d

dtrace_probes.h: libmemcached_probes.d
	$(DTRACE) $(DTRACEFLAGS) -h -o dtrace_probes.h -s libmemcached_probes.d

libmemcached_probes.o: libmemcached_probes.d $(libmemcached_la_OBJECTS)
	$(DTRACE) $(DTRACEFLAGS) -o .libs/libmemcached_probes.o -G -s libmemcached_probes.d `grep '^pic_object' *.lo | cut -f 2 -d\'`
	$(DTRACE) $(DTRACEFLAGS) -o libmemcached_probes.o -G -s libmemcached_probes.d `grep non_pic_object *.lo | cut -f 2 -d\' `

