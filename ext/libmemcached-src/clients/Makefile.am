INCLUDES = -I$(top_builddir)
LDADDS = $(top_builddir)/libmemcached/libmemcached.la

bin_PROGRAMS = memcat memcp memstat memrm memflush memslap memerror

noinst_HEADERS = client_options.h \
		utilities.h \
		generator.h \
		execute.h 

memcat_SOURCES = memcat.c utilities.c
memcat_LDADD = $(LDADDS)
memcat_LDFLAGS = -rpath $(pkglibdir)

memcp_SOURCES = memcp.c utilities.c
memcp_LDADD = $(LDADDS)
memcp_LDFLAGS = -rpath $(pkglibdir)

memstat_SOURCES = memstat.c utilities.c
memstat_LDADD = $(LDADDS)
memstat_LDFLAGS = -rpath $(pkglibdir)

memrm_SOURCES = memrm.c utilities.c
memrm_LDADD = $(LDADDS)
memrm_LDFLAGS = -rpath $(pkglibdir)

memflush_SOURCES = memflush.c utilities.c
memflush_LDADD = $(LDADDS)
memflush_LDFLAGS = -rpath $(pkglibdir)

memerror_SOURCES = memerror.c utilities.c
memerror_LDADD = $(LDADDS)
memerror_LDFLAGS = -rpath $(pkglibdir)

memslap_SOURCES = memslap.c utilities.c generator.c execute.c
memslap_LDADD = $(LDADDS) -lpthread
memslap_LDFLAGS = -rpath $(pkglibdir)

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
