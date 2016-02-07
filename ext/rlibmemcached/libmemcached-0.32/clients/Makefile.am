LDADDS = libutilities.la $(top_builddir)/libmemcached/libmemcached.la

bin_PROGRAMS = memcat memdump memcp memstat memrm memflush memslap memerror

noinst_HEADERS = client_options.h \
		utilities.h \
		generator.h \
		execute.h

noinst_LTLIBRARIES= libutilities.la libgenexec.la

libutilities_la_SOURCES= utilities.c
libgenexec_la_SOURCES= generator.c execute.c

memcat_SOURCES = memcat.c
memcat_LDADD = $(LDADDS)
memcat_LDFLAGS = -rpath $(pkglibdir)

memcp_SOURCES = memcp.c
memcp_LDADD = $(LDADDS)
memcp_LDFLAGS = -rpath $(pkglibdir)

memdump_SOURCES = memdump.c
memdump_LDADD = $(LDADDS)
memdump_LDFLAGS = -rpath $(pkglibdir)

memstat_SOURCES = memstat.c
memstat_LDADD = $(LDADDS)
memstat_LDFLAGS = -rpath $(pkglibdir)

memrm_SOURCES = memrm.c
memrm_LDADD = $(LDADDS)
memrm_LDFLAGS = -rpath $(pkglibdir)

memflush_SOURCES = memflush.c
memflush_LDADD = $(LDADDS)
memflush_LDFLAGS = -rpath $(pkglibdir)

memerror_SOURCES = memerror.c
memerror_LDADD = $(LDADDS)
memerror_LDFLAGS = -rpath $(pkglibdir)

memslap_SOURCES = memslap.c
memslap_CFLAGS = $(AM_CFLAGS) $(PTHREAD_CFLAGS)
memslap_LDADD = $(PTHREAD_LIBS) libgenexec.la $(LDADDS)
memslap_LDFLAGS = $(AM_LDFLAGS) -rpath $(pkglibdir)

if HAVE_SASL
memcat_LDFLAGS+= $(LTLIBSASL)
memcp_LDFLAGS+= $(LTLIBSASL)
memdump_LDFLAGS+= $(LTLIBSASL)
memstat_LDFLAGS+= $(LTLIBSASL)
memrm_LDFLAGS+= $(LTLIBSASL)
memflush_LDFLAGS+= $(LTLIBSASL)
memerror_LDFLAGS+= $(LTLIBSASL)
memslap_LDFLAGS+= $(LTLIBSASL)
endif

test-start-server:
	memflush --servers=localhost
	memcp --servers=localhost /etc/services
	memcat --servers=localhost /etc/services
	memrm --servers=localhost /etc/services
	memstat --servers=localhost
	memslap --servers=localhost
	memslap --servers=localhost --concurrency=10
	memslap --servers=localhost --concurrency=10 --initial-load=1000
	memslap --servers=localhost --concurrency=10 --initial-load=1000 --execute-number=10
	memslap --servers=localhost --concurrency=10 --initial-load=1000 --execute-number=10 --test=get
	memslap --servers=localhost --concurrency=10 --initial-load=1000 --execute-number=10 --test=set
	memslap --servers=localhost --concurrency=10 --initial-load=1000 --execute-number=10 --test=set --non-blocking

valgrind:
	libtool --mode=execute valgrind --leak-check=yes --show-reachable=yes  memslap --servers=localhost
	libtool --mode=execute valgrind --leak-check=yes --show-reachable=yes  memslap --servers=localhost --concurrency=10
	libtool --mode=execute valgrind --leak-check=yes --show-reachable=yes  memslap --servers=localhost --concurrency=10 --initial-load=1000
	libtool --mode=execute valgrind --leak-check=yes --show-reachable=yes  memslap --servers=localhost --concurrency=10 --initial-load=1000 --execute-number=10
	libtool --mode=execute valgrind --leak-check=yes --show-reachable=yes  memslap --servers=localhost --concurrency=10 --initial-load=1000 --execute-number=10 --test=get
	libtool --mode=execute valgrind --leak-check=yes --show-reachable=yes  memslap --servers=localhost --concurrency=10 --initial-load=1000 --execute-number=10 --test=set
	libtool --mode=execute valgrind --leak-check=yes --show-reachable=yes  memslap --servers=localhost --concurrency=10 --initial-load=1000 --execute-number=10 --test=set --non-blocking
