srpm:
	echo RAUS $(outdir) 
	echo SPEC $(spec)
	rpmbuild --define "_topdir $(pwd)" -bs $(spec)
	mv SRPMS/* $(outdir)/
