srpm:
	echo RAUS $(outdir) 
	echo SPEC $(spec)
	mkdir $$(TMP)/X
	rpmbuild --define "_topdir $$(TMP)/X" -bs $(spec)
	mv $$(TMP)/X/SRPMS/* $(outdir)/
