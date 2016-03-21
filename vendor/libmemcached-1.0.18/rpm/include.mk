# vim:ft=automake

RPM_BUILDDIR= ~/rpmbuild

RPM_BUILD_TARGET= @PACKAGE@-@VERSION@-@RPM_RELEASE@.@build_cpu@.rpm
RPM_SOURCE= $(RPM_BUILDDIR)/SOURCES/$(DIST_ARCHIVES)

RPMS=
RPMS+= $(RPM_BUILD_TARGET)
RPMS+= @PACKAGE@-devel-@VERSION@-@RPM_RELEASE@.@build_cpu@.rpm
RPMS+= @PACKAGE@-debuginfo-@VERSION@-@RPM_RELEASE@.@build_cpu@.rpm

SRPMS= @PACKAGE@-@VERSION@-@RPM_RELEASE@.src.rpm

RPM_DIST= $(RPMS) $(SRPMS)

BUILD_RPMS= $(foreach rpm_iterator,$(RPMS),$(addprefix $(RPM_BUILDDIR)/RPMS/@build_cpu@/, $(rpm_iterator)))
BUILD_SRPMS= $(foreach srpm_iterator,$(SRPMS),$(addprefix $(RPM_BUILDDIR)/SRPMS/, $(srpm_iterator)))
BUILD_RPM_DIR= $(RPM_BUILDDIR)/BUILD/@PACKAGE@-@VERSION@

$(RPM_BUILDDIR):
	@@RPMDEV_SETUPTREE@

$(DIST_ARCHIVES): $(DISTFILES)
	$(MAKE) $(AM_MAKEFLAGS) dist-gzip

$(RPM_SOURCE): | $(RPM_BUILDDIR) $(DIST_ARCHIVES)
	@rm -f $(BUILD_RPMS) $(BUILD_SRPMS) $(BUILD_RPM_DIR)
	@cp $(DIST_ARCHIVES) $(RPM_SOURCE)

$(RPM_BUILD_TARGET): $(RPM_SOURCE) support/@PACKAGE@.spec
	@@RPMBUILD@ -ba $(top_srcdir)/support/@PACKAGE@.spec
	@cp $(BUILD_RPMS) $(BUILD_SRPMS) .

.PHONY: rpm-sign
rpm-sign: $(RPM_BUILD_TARGET)
	@@RPM@ --addsign $(RPM_DIST)
	@@RPM@ --checksig $(RPM_DIST)

.PHONY: clean-rpm
clean-rpm:
	-@rm -f $(BUILD_RPMS) $(BUILD_SRPMS) $(BUILD_RPM_SOURCE) $(BUILD_RPM_DIR) $(RPM_DIST)

rpm: $(RPM_BUILD_TARGET)
dist-rpm: $(RPM_BUILD_TARGET)

.PHONY: release
release: rpm rpm-sign

.PHONY: auto-rpmbuild
auto-rpmbuild: support/@PACKAGE@.spec
	@auto-br-rpmbuild -ba $(top_srcdir)/support/@PACKAGE@.spec
