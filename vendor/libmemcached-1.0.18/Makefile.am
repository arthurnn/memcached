# vim:ft=automake

ACLOCAL_AMFLAGS= -I m4 -I libtest/m4
AM_YFLAGS= -d

# includes append to these:
SUFFIXES =
.PHONY =
TESTS =
XFAIL_TESTS =
CLEANFILES =
DISTCLEANFILES =
bin_PROGRAMS =
noinst_HEADERS =
lib_LTLIBRARIES =
man_MANS =
noinst_LTLIBRARIES =
noinst_PROGRAMS =
include_HEADERS =
nobase_include_HEADERS =
check_PROGRAMS =
check_LTLIBRARIES=
EXTRA_HEADERS =
BUILT_SOURCES=
EXTRA_DIST=
dist_man_MANS=
MAINTAINERCLEANFILES=

#includes additional rules from aminclude.am
@INC_AMINCLUDE@

DISTCLEANFILES+= aminclude.am

EXTRA_DIST+= README.FIRST
EXTRA_DIST+= README.win32

aclocaldir= $(datadir)/aclocal
dist_aclocal_DATA=

# Build targets for Windows
if BUILD_WIN32
include libhashkit-1.0/include.am
include libhashkit/include.am
include libmemcached/include.am
include libmemcached-1.0/include.am
else

include libtest/include.am

include libhashkit-1.0/include.am
include libmemcached-1.0/include.am

if BUILD_LIBMEMCACHED_PROTOCOL
include libmemcachedprotocol-0.0/include.am
endif

include libmemcachedutil-1.0/include.am

include clients/include.am
include docs/include.am
include man/include.am

if BUILD_LIBMEMCACHED_PROTOCOL
include example/include.am
endif

include libhashkit/include.am
include libmemcached/include.am
include libmemcachedutil/include.am

if BUILD_LIBMEMCACHED_PROTOCOL
include libmemcachedprotocol/include.am
endif

include libmemcachedinternal/include.am
include libmemcachedinternal/util/include.am
include rpm/include.mk
include support/include.am
include tests/include.am
include util/include.am
include win32/include.am

include m4/include.am
endif

TESTS += ${bin_PROGRAMS}
TESTS += ${check_PROGRAMS}
XFAIL_TESTS += ${bin_PROGRAMS}

if ! BUILD_WIN32

merge-clean:
	@find ./ | $(GREP) \.gcda | xargs rm -f
	@find ./ | $(GREP) \.gcno | xargs rm -f
	@find ./ | $(GREP) \.gz | xargs rm -f
	@find ./ | $(GREP) \.moved | xargs rm -r -f
	@find ./ | $(GREP) \\.orig | xargs rm -f
	@find ./ | $(GREP) \.rej | xargs rm -f
	@find ./ | $(GREP) \.THIS | xargs rm -f
	@find ./ | $(GREP) \.OTHER | xargs rm -f
	@find ./ | $(GREP) \.BASE | xargs rm -f
	@find ./ | $(GREP) \~$$ | xargs rm -f
	@echo "Files that need to be either removed or checked in:"
	@bzr unknowns

clean-local: clean-libtest-check clean-docs-check clean-rpm


lcov: lcov-clean check
	@echo
	@echo "	------------------------------------------------------"
	@echo "Make sure ./configure was run with '--enable-coverage'"
	@echo "------------------------------------------------------"
	@echo
	@cd libmemcached && lcov --capture --directory . --base-directory .. --output-file lcov.out
	@genhtml -o lcov --function-coverage -t libmemcached libmemcached/lcov.out

lcov-clean: clean
	@rm -rf lcov */lcov.out
	@find . -name '*.gcno' | xargs rm -f
	@find . -name '*.gcda' | xargs rm -f
endif

DISTCLEANFILES+= config/top.h

maintainer-clean-local:
	find . -type f -name '*~' -exec rm -f '{}' \;
	-rm -f man/*.1
	-rm -f man/*.3
	-rm -f man/*.8
	-rm -rf man/.doctrees/
	-rm -f Makefile.in
	-rm -f aclocal.m4
	-rm -f build-aux/compile
	-rm -f build-aux/config.guess
	-rm -f build-aux/config.sub
	-rm -f build-aux/depcomp
	-rm -f build-aux/install-sh
	-rm -f build-aux/ltmain.sh
	-rm -f build-aux/missing
	-rm -f build-aux/test-driver
	-rm -f build-aux/ylwrap
	-rm -f mem_config.in
	-rm -f config.log
	-rm -f config.status
	-rm -f configure
	-rm -f m4/libtool.m4
	-rm -f m4/ltoptions.m4
	-rm -f m4/ltsugar.m4
	-rm -f m4/ltversion.m4
	-rm -f m4/lt~obsolete.m4
	-rm -f m4/test-driver
	-rmdir build-aux
	-rm -rf ${top_builddir}/html
	-rm -f $(DIST_ARCHIVES)
