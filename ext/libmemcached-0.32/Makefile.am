ACLOCAL_AMFLAGS = -I m4

SUBDIRS = libmemcached libmemcachedutil support clients tests
EXTRA_dist = README.FIRST

check-local: test-no-outputdiff

test: all
	@(cd tests; ${MAKE} test)

test-extended: all
	@(cd tests; ${MAKE} test-extended)

valgrind:
	@(cd tests; ${MAKE} valgrind)

test-no-outputdiff:
	@(cd tests; ${MAKE} test-no-outputdiff)

fedora:
	rm -f ~/rpmbuild/RPMS/x86_64/libmemcached-$(VERSION)*.rpm
	rm -f ~/rpmbuild/SRPMS/libmemcached-$(VERSION)*.rpm
	cp libmemcached-$(VERSION).tar.gz /home/brian/rpmbuild/SOURCES/
	rpmbuild -ba support/libmemcached.spec
	cp ~/rpmbuild/RPMS/x86_64/libmemcached-$(VERSION)*.rpm .
	cp ~/rpmbuild/SRPMS/libmemcached-$(VERSION)*.rpm .

generic:
	rm -f ~/rpmbuild/RPMS/x86_64/libmemcached-$(VERSION)*.rpm
	rm -f ~/rpmbuild/SRPMS/libmemcached-$(VERSION)*.rpm
	cp libmemcached-$(VERSION).tar.gz /home/brian/rpmbuild/SOURCES/
	rpmbuild -ba support/libmemcached.spec
	cp ~/rpmbuild/RPMS/x86_64/libmemcached-$(VERSION)*.rpm .
	cp ~/rpmbuild/SRPMS/libmemcached-$(VERSION)*.rpm .

rpm: all dist generic fedora
