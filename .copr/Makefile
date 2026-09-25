ARCHIV = build.tar.gz
SPEC =  $(notdir $(spec))

srpm:
	echo RAUS $(outdir) 
	echo SPEC $(spec)
	mkdir -p $(TMP)/X/{SOURCES,SPECS}
	tar czf $(TMP)/X/SOURCES/$(ARCHIV) .
	cp $(spec) $(TMP)/X/SPECS/
	sed -i "s|^Source0.*|Source0:\t$(ARCHIV)|g" $(TMP)/X/SPECS/$(SPEC)
	rpmbuild -bs $(TMP)/X/SPECS/$(SPEC)
	mv $(TMP)/X/SRPMS/* $(outdir)/
