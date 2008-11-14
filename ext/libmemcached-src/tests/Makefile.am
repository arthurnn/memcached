INCLUDES = -I$(top_builddir)
LDADDS = $(top_builddir)/libmemcached/libmemcached.la

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

testapp_SOURCES = test.c function.c server.c ../clients/generator.c ../clients/execute.c
testapp_LDADD = $(LDADDS)

testplus_SOURCES = test.c plus.cpp server.c
testplus_LDADD = $(LDADDS)

udptest_SOURCES = test.c udp.c server.c
udptest_LDADD = $(LDADDS)

atomsmasher_SOURCES = test.c atomsmasher.c server.c ../clients/generator.c ../clients/execute.c
atomsmasher_LDADD = $(LDADDS)

startservers_SOURCES = start.c server.c
startservers_LDADD = $(LDADDS)

record:
	./testapp > output.res

client-record:
	sh t/memcat.test > r/memcat.res
	sh t/memcp.test > r/memcp.res
	sh t/memrm.test > r/memrm.res
	sh t/memslap.test > r/memslap.res
	sh t/memstat.test > r/memstat.res

record-extended:
	./testapp extended > output2.res

test: testapp library_test
	echo "Tests completed"

library_test:
	./testapp > output.cmp
	diff output.res output.cmp

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
