EXTRA_DIST = libmemcached_probes.d memcached/README.txt

INCLUDES = -I$(top_builddir)
LIBS = @LIBS@

DTRACE =                @DTRACE@
DTRACEFLAGS =           @DTRACEFLAGS@
DTRACEFILES = memcached.o \
              memcached_strerror.o \
              memcached_connect.o \
              memcached_response.o \
              memcached_get.o \
              memcached_storage.o \
              memcached_delete.o \
              memcached_hash.o \
              memcached_auto.o \
              memcached_verbosity.o \
              memcached_quit.o \
              memcached_flush.o \
              memcached_string.o \
              memcached_stats.o

noinst_HEADERS = libmemcached_probes.h \
		 memcached_io.h \
                 common.h \
                 memcached/protocol_binary.h

pkginclude_HEADERS= memcached.h \
		    memcached.hh \
		    memcached_constants.h \
		    memcached_get.h \
		    memcached_result.h \
		    memcached_server.h \
		    memcached_storage.h \
		    memcached_string.h \
		    memcached_types.h \
		    memcached_watchpoint.h

lib_LTLIBRARIES = libmemcached.la

libmemcached_la_SOURCES = crc.c \
			  hsieh_hash.c \
			  memcached.c \
			  memcached_auto.c \
			  memcached_behavior.c \
			  memcached_callback.c \
			  memcached_connect.c \
			  memcached_delete.c \
			  memcached_do.c \
			  memcached_fetch.c \
			  memcached_fetch_execute.c \
			  memcached_flush.c \
			  memcached_get.c \
			  memcached_hash.c \
			  memcached_hosts.c \
			  memcached_io.c \
			  memcached_purge.c \
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
                          byteorder.c

libmemcached_la_LIBADD =
libmemcached_la_LDFLAGS = -version-info $(MEMCACHED_LIBRARY_VERSION)

if HAVE_DTRACE
  libmemcached_la_LIBADD += libmemcached_probes.o
endif

libmemcached_probes.h: libmemcached_probes.d
	$(DTRACE) $(DTRACEFLAGS) -h -s libmemcached_probes.d
	mv libmemcached_probes.h libmemcached_probes.h.bak
	sed "s/#include <unistd.h>//g" libmemcached_probes.h.bak > libmemcached_probes.h
	rm libmemcached_probes.h.bak

libmemcached_probes.o:
	$(DTRACE) $(DTRACEFLAGS) -G -s libmemcached_probes.d $(DTRACEFILES)
