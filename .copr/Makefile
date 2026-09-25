ARCHIV = build.tar.gz

srpm:
	echo RAUS $(outdir) 
	echo SPEC $(spec)
	mkdir -p $(TMP)/X/SOURCES
	tar czf $(TMP)/X/SOURCES/$(ARCHIV) .
	rpmbuild --define "_topdir $(TMP)/X" -bs $(spec)
	mv $(TMP)/X/SRPMS/* $(outdir)/
