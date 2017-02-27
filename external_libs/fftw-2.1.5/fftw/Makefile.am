# This file was automatically generated
# DO NOT EDIT!
#
lib_LTLIBRARIES = lib@FFTW_PREFIX@fftw.la
include_HEADERS = @FFTW_PREFIX@fftw.h
INCLUDES = -I$(srcdir)/../fftw -I$(srcdir)

NOTW_CODELETS= fn_1.c fn_2.c fn_3.c fn_4.c fn_5.c fn_6.c fn_7.c fn_8.c fn_9.c fn_10.c fn_11.c fn_12.c fn_13.c fn_14.c fn_15.c fn_16.c fn_32.c fn_64.c
TWID_CODELETS= ftw_2.c ftw_3.c ftw_4.c ftw_5.c ftw_6.c ftw_7.c ftw_8.c ftw_9.c ftw_10.c ftw_16.c ftw_32.c ftw_64.c
NOTWI_CODELETS= fni_1.c fni_2.c fni_3.c fni_4.c fni_5.c fni_6.c fni_7.c fni_8.c fni_9.c fni_10.c fni_11.c fni_12.c fni_13.c fni_14.c fni_15.c fni_16.c fni_32.c fni_64.c
TWIDI_CODELETS= ftwi_2.c ftwi_3.c ftwi_4.c ftwi_5.c ftwi_6.c ftwi_7.c ftwi_8.c ftwi_9.c ftwi_10.c ftwi_16.c ftwi_32.c ftwi_64.c

CODELETS=$(NOTW_CODELETS) $(TWID_CODELETS) $(NOTWI_CODELETS) $(TWIDI_CODELETS)
OTHERSRC = timer.c config.c planner.c twiddle.c executor.c \
	   generic.c fftwnd.c malloc.c wisdom.c wisdomio.c putils.c rader.c \
           fftwf77.c

lib@FFTW_PREFIX@fftw_la_SOURCES = $(CODELETS) $(OTHERSRC)             \
                                        fftw.h fftw-int.h 

lib@FFTW_PREFIX@fftw_la_LDFLAGS = -version-info @SHARED_VERSION_INFO@
MAINTAINERCLEANFILES = $(CODELETS) config.c
DISTCLEANFILES = fftw.h sfftw.h dfftw.h

@FFTW_PREFIX1@fftw.h: fftw.h
	rm -f $@
	sed 's/\"fftw/\"@FFTW_PREFIX@fftw/g;s/\"rfftw/\"@FFTW_PREFIX@rfftw/g' $(srcdir)/fftw.h > $@

CLEANFILES = @FFTW_PREFIX1@fftw.h

# for some reason, automake tries to use autoheader in order to
# generate config.h.in, and fails because config.h.in is GNU-lly
# incorrect.  Just disable autoheader
AUTOHEADER=echo
