ARCHIV = build.tar.gz
SPEC =  $(notdir $(spec))

srpm:
	echo RAUS $(outdir) 
	echo SPEC $(spec)
	mkdir -p $(TMP)/X/{SOURCES,SPECS}
	tar czf $(TMP)/X/SOURCES/$(ARCHIV) .
	cp $(SPEC) $(TMP)/X/SPECS/
	sed -i "s|^Source0.*|Source0:\t$(ARCHIV)|g" $(TMP)/X/SPECS/$(SPEC)
	rpmbuild --define "_topdir $(TMP)/X" -bs $(spec)
	mv $(TMP)/X/SRPMS/* $(outdir)/
