ARCHIV = build.tar.gz
SPEC =  $(notdir $(spec))
RPM_ROOT := $(shell rpm --eval %_topdir)

srpm:
	echo RAUS $(outdir) 
	echo SPEC $(spec)
	mkdir -p $(RPM_ROOT)/{SOURCES,SPECS}
	tar czf $(RPM_ROOT)/SOURCES/$(ARCHIV) .
	cp $(spec) $(RPM_ROOT)/SPECS/
	sed -i "s|^Source0.*|Source0:\t$(ARCHIV)|g" $(RPM_ROOT)/SPECS/$(SPEC)
	rpmbuild -bs $(SPEC)
	mv $(RPM_ROOT)/SRPMS/* $(outdir)/
