srpm:
	echo RAUS $(outdir) 
	echo SPEC $(spec)
	ls -l 
	rpmbuild --define "_topdir $(CURDIR)" -bs $(spec)
	mv SRPMS/* $(outdir)/
