AC_LOCAL_AMFLAGS = -I ../../../m4

if USE_MD5_HASH
BASE_NAME=volumedriver-test
else
BASE_NAME=volumedriver-no-dedup-test
endif

RPM_LOCATION = $(prefix)/rpm
ARTEFACT = ../$(BASE_NAME)_@VD_MAJOR_VERSION@.@VD_MINOR_VERSION@.@VD_PATCH_VERSION@.@VD_EXTRA_VERSION@-1.rpm
ARTEFACT_DBGSYM = ../$(BASE_NAME)-debuginfo_@VD_MAJOR_VERSION@.@VD_MINOR_VERSION@.@VD_PATCH_VERSION@.@VD_EXTRA_VERSION@-1.rpm

.PHONY: SPECS/$(BASE_NAME).spec $(ARTEFACT)

noinst_DATA = SPECS/$(BASE_NAME).spec add-python-dirs.sh

SPECS/$(BASE_NAME).spec:
	./add-python-dirs.sh

clean-local:
	-rm -rf ../$(BASE_NAME)_*

$(ARTEFACT):
	mkdir -p BUILDROOT
	ln -nsf . ../../../../usr
	awk 'BEGIN { fsection=0; } /^%files/  { fsection=1; } /^\// { if (fsection) { printf("."); print; } }' SPECS/$(BASE_NAME).spec > file.list
	d="${PWD}/BUILDROOT"; (cd $(prefix); cpio -pdm "$$d") < file.list
	rpmbuild --define "_topdir ${PWD}" \
                 --define "buildroot ${PWD}/BUILDROOT/" \
                 -ba SPECS/$(BASE_NAME).spec
	mv RPMS/x86_64/$(BASE_NAME)-@VD_MAJOR_VERSION@.@VD_MINOR_VERSION@.@VD_PATCH_VERSION@.@VD_EXTRA_VERSION@-1.x86_64.rpm $(ARTEFACT)
	mv RPMS/x86_64/$(BASE_NAME)-debuginfo-@VD_MAJOR_VERSION@.@VD_MINOR_VERSION@.@VD_PATCH_VERSION@.@VD_EXTRA_VERSION@-1.x86_64.rpm $(ARTEFACT_DBGSYM)

install-data-local: $(ARTEFACT)
	$(INSTALL) -d $(RPM_LOCATION)
	$(INSTALL) $(ARTEFACT) $(RPM_LOCATION)
	$(INSTALL) $(ARTEFACT_DBGSYM) $(RPM_LOCATION)
