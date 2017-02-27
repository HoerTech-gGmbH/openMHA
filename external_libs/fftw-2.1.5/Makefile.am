SUBDIRS = fftw rfftw tests threads mpi
EXTRA_DIST = README.hacks bootstrap.sh COPYRIGHT acx_pthread.m4 acx_mpi.m4

extradist = fortran gensrc matlab cilk FAQ
distsrc=$(distdir)/fftw

# for some reason, automake tries to use autoheader in order to
# generate config.h.in, and fails because config.h.in is GNU-lly
# incorrect.  Just disable autoheader
AUTOHEADER=echo

dist-hook:
	rm -f $(distsrc)/fftw.h
	awk '{handled=0} /^#undef .*$$/ {print "/* " $$line " */"; handled=1} {if (handled == 0) print}' $(distsrc)/fftw.h.in > $(distsrc)/fftw.h
	rm -f $(distsrc)/config.h
	cat $(distsrc)/config.h.in | sed -e "s/^#undef FFTW_VERSION/#define FFTW_VERSION \"$(VERSION)\"/" | awk '{handled=0}	/^#undef .*$$/ {print "/* " $$line " */"; handled=1} {if (handled == 0) print}' > $(distsrc)/config.h
	(cd FAQ; make clean; make)
	for i in gensrc mpi threads cilk; do (cd $$i; make distclean); done
	for i in $(extradist); do \
	  echo "Adding $$i/ to the distribution"; \
          find $$i -name CVS -prune -o -print | cpio -pd $(distdir); \
        done

RPM_SRCDIR = `rpm --showrc |grep "^sourcedir" | sed 's/^.*: *//'`
RPM_ARCH   = `rpm --showrc |grep "^build arch" | sed 's/^.*: *//'`
RPM_RPMS   = `rpm --showrc |grep "^rpmdir" | sed 's/^.*: *//'`
RPM_SRPMS  = `rpm --showrc |grep "^srcrpmdir" | sed 's/^.*: *//'`
RPM_TOPDIR = `rpm --showrc |grep "^TOPDIRdir" | sed 's/^.*: *//'`

rpm:
	cp -f $(PACKAGE)-$(VERSION).tar.gz $(RPM_SRCDIR)/$(PACKAGE)-$(VERSION).tar.gz
	rm -f $(RPM_SRCDIR)/$(PACKAGE)-logo-thumb.gif
	lynx -source -dump http://www.fftw.org/fftw-logo-thumb.gif > $(RPM_SRCDIR)/fftw-logo-thumb.gif
	rpm --clean -ba $(PACKAGE).spec
	rpm -U $(RPM_RPMS)/$(RPM_ARCH)/$(PACKAGE)-$(VERSION)-1.$(RPM_ARCH).rpm\
         $(RPM_RPMS)/$(RPM_ARCH)/$(PACKAGE)-devel-$(VERSION)-1.$(RPM_ARCH).rpm
	@echo "-------------------------------------------------------------"
	@echo "  RPMs are built and installed, and are located under:       "
	@echo "       $(RPM_RPMS)/$(RPM_ARCH)"
	@echo "       $(RPM_SRPMS)"
	@echo "-------------------------------------------------------------"
