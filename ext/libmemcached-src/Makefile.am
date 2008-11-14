INCLUDES = 
SUBDIRS = docs libmemcached tests support clients
EXTRA_dist = README.FIRST

test: all
	cd tests; make test; cd ..

test-extended: all
	cd tests; make test-extended; cd ..

valgrind:
	cd tests; make valgrind; cd ..

rpm: all dist
	cp libmemcached-$(VERSION).tar.gz /home/brian/rpm/SOURCES/
	rpmbuild -ba support/libmemcached.spec
	cp /home/brian/rpm/RPMS/x86_64/libmemcached-$(VERSION)*.rpm .
	cp /home/brian/rpm/SRPMS/libmemcached-$(VERSION)*.rpm .
