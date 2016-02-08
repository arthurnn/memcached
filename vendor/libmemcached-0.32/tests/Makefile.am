LDADDS = $(top_builddir)/libmemcached/libmemcached.la

if BUILD_LIBMEMCACHEDUTIL
LDADDS+= $(top_builddir)/libmemcachedutil/libmemcachedutil.la
endif

EXTRA_DIST = output.res output2.res\
		r/memcat.res\
		r/memcp.res\
		r/memrm.res\
		r/memslap.res\
		r/memstat.res\
		t/memcat.test\
		t/memcp.test\
		t/memrm.test\
		t/memslap.test\
		t/memstat.test

LIBS = 

noinst_HEADERS = test.h server.h ketama_test_cases.h
noinst_PROGRAMS = testapp testplus udptest atomsmasher startservers
noinst_LTLIBRARIES= libserver.la libtest.la

libserver_la_SOURCES= server.c
libtest_la_SOURCES= test.c

testapp_CFLAGS= $(AM_CFLAGS) $(NO_CONVERSION) $(NO_STRICT_ALIASING)
testapp_SOURCES = function.c
testapp_LDADD = $(top_builddir)/clients/libgenexec.la libtest.la libserver.la $(LDADDS)

testplus_SOURCES = plus.cpp
testplus_LDADD = libtest.la libserver.la $(LDADDS)

udptest_SOURCES = udp.c
udptest_LDADD = libtest.la libserver.la $(LDADDS)

atomsmasher_SOURCES = atomsmasher.c
atomsmasher_LDADD = $(top_builddir)/clients/libgenexec.la libtest.la libserver.la $(LDADDS)

startservers_SOURCES = start.c
startservers_LDADD = libserver.la $(LDADDS)

if HAVE_SASL
testapp_LDADD+= $(LTLIBSASL)
testplus_LDADD+= $(LTLIBSASL)
udptest_LDADD+= $(LTLIBSASL)
atomsmasher_LDADD+= $(LTLIBSASL)
startservers_LDADD+= $(LTLIBSASL)
endif

record:
	./testapp > output.res
	./testplus > output_plus.res

client-record:
	sh t/memcat.test > r/memcat.res
	sh t/memcp.test > r/memcp.res
	sh t/memrm.test > r/memrm.res
	sh t/memslap.test > r/memslap.res
	sh t/memstat.test > r/memstat.res

record-extended:
	./testapp extended > output2.res

test: testapp testplus library_test
	echo "Tests completed"

library_test:
	./testapp > output.cmp
	diff output.res output.cmp
#	./testplus > output_plus.cmp
#	diff output_plus.res output_plus.cmp

clients:
	memcached -d -P /tmp/Xumemc.pid -p 12555
	export MEMCACHED_SERVERS="localhost:12555"
	sh t/memcat.test > r/memcat.cmp
	diff r/memcat.res r/memcat.cmp
	sh t/memcp.test > r/memcp.cmp
	diff r/memcp.res r/memcp.cmp
	sh t/memrm.test > r/memrm.cmp
	diff r/memrm.res r/memrm.cmp
	sh t/memslap.test > r/memslap.cmp
	diff r/memslap.res r/memslap.cmp
	sh t/memstat.test > r/memstat.cmp
	diff r/memstat.res r/memstat.cmp
	cat /tmp/Xumemc.pid | xargs kill
	rm /tmp/Xumemc.pid

valgrind:
	libtool --mode=execute valgrind --leak-check=yes --show-reachable=yes  testapp

cachegrind:
	rm -f cachegrind.out.*
	libtool --mode=execute valgrind --tool=cachegrind  --branch-sim=yes testapp
	cg_annotate cachegrind.out.* --auto=yes > /tmp/cachegrind.out

callgrind:
	rm -f callgrind.out.*
	libtool --mode=execute valgrind --tool=callgrind  testapp
	callgrind_annotate callgrind.out.* --auto=yes > /tmp/callgrind.out

helgrind:
	rm -f helgrind.out.*
	libtool --mode=execute valgrind --tool=helgrind  testapp

helgrind-slap:
	libtool --mode=execute valgrind --tool=helgrind  ${top_builddir}/clients/memslap --server=localhost --concurrency=30

test-no-outputdiff: testapp
	./testapp > /dev/null
	@echo "Test completed"
