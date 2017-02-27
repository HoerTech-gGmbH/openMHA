# This file was automatically generated
# DO NOT EDIT!
#
lib_LTLIBRARIES = lib@FFTW_PREFIX@rfftw.la
include_HEADERS = @FFTW_PREFIX@rfftw.h
INCLUDES = -I$(srcdir)/../fftw -I$(srcdir)

NOTW_CODELETS= frc_1.c frc_2.c frc_3.c frc_4.c frc_5.c frc_6.c frc_7.c frc_8.c frc_9.c frc_10.c frc_11.c frc_12.c frc_13.c frc_14.c frc_15.c frc_16.c frc_32.c frc_64.c frc_128.c
TWID_CODELETS= fhf_2.c fhf_3.c fhf_4.c fhf_5.c fhf_6.c fhf_7.c fhf_8.c fhf_9.c fhf_10.c fhf_16.c fhf_32.c
NOTWI_CODELETS= fcr_1.c fcr_2.c fcr_3.c fcr_4.c fcr_5.c fcr_6.c fcr_7.c fcr_8.c fcr_9.c fcr_10.c fcr_11.c fcr_12.c fcr_13.c fcr_14.c fcr_15.c fcr_16.c fcr_32.c fcr_64.c fcr_128.c
TWIDI_CODELETS= fhb_2.c fhb_3.c fhb_4.c fhb_5.c fhb_6.c fhb_7.c fhb_8.c fhb_9.c fhb_10.c fhb_16.c fhb_32.c

CODELETS=$(NOTW_CODELETS) $(TWID_CODELETS) $(NOTWI_CODELETS) $(TWIDI_CODELETS)
OTHERSRC = rconfig.c rplanner.c rexec.c rexec2.c rfftwnd.c rgeneric.c \
           rfftwf77.c

lib@FFTW_PREFIX@rfftw_la_SOURCES = $(CODELETS) $(OTHERSRC)    \
					 rfftw.h                   

lib@FFTW_PREFIX@rfftw_la_LDFLAGS = -version-info @SHARED_VERSION_INFO@
MAINTAINERCLEANFILES = $(CODELETS) rconfig.c
DISTCLEANFILES = srfftw.h drfftw.h

@FFTW_PREFIX1@rfftw.h: rfftw.h
	rm -f $@
	sed 's/\"fftw/\"@FFTW_PREFIX@fftw/g;s/\"rfftw/\"@FFTW_PREFIX@rfftw/g' $(srcdir)/rfftw.h > $@

CLEANFILES = @FFTW_PREFIX1@rfftw.h

# for some reason, automake tries to use autoheader in order to
# generate config.h.in, and fails because config.h.in is GNU-lly
# incorrect.  Just disable autoheader
AUTOHEADER=echo

