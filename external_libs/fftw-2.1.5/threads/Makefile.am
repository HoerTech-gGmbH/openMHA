# The threads stuff is only compiled if FFTW was configured with
# the --enable-threads option.  This requires us to use the combination
# of foo_* and EXTRA_* variables below.

lib_LTLIBRARIES = @FFTW_THREADS_LIBLIST@
include_HEADERS = @FFTW_THREADS_INCLUDELIST@
noinst_PROGRAMS = @FFTW_THREADS_PROGLIST@

EXTRA_LTLIBRARIES = lib@FFTW_PREFIX@fftw_threads.la \
                    lib@FFTW_PREFIX@rfftw_threads.la
EXTRA_HEADERS = @FFTW_PREFIX@fftw_threads.h \
                @FFTW_PREFIX@rfftw_threads.h
EXTRA_PROGRAMS = fftw_threads_test rfftw_threads_test

FFTWDIR=../fftw
RFFTWDIR=../rfftw
INCLUDES = -I$(srcdir)/../tests \
           -I$(srcdir)/$(FFTWDIR) -I$(srcdir)/$(RFFTWDIR) -I$(srcdir)


lib@FFTW_PREFIX@fftw_threads_la_SOURCES = \
                             executor_threads.c fftw_threads.c \
                             fftwnd_threads.c \
                             fftw_f77_threads.c \
	                     @FFTW_PREFIX@fftw_threads.h \
                             fftw_threads-int.h
lib@FFTW_PREFIX@fftw_threads_la_LDFLAGS = \
                             -version-info @SHARED_VERSION_INFO@ \
                             -rpath $(libdir)

@FFTW_PREFIX1@fftw_threads.h: fftw_threads.h
	rm -f $@
	sed 's/\"fftw/\"@FFTW_PREFIX@fftw/g;s/\"rfftw/\"@FFTW_PREFIX@rfftw/g' $(srcdir)/fftw_threads.h > $@

fftw_threads_test_SOURCES = fftw_threads_test.c
fftw_threads_test_LDADD = ../tests/test_main.o \
	                  lib@FFTW_PREFIX@fftw_threads.la \
                          $(FFTWDIR)/lib@FFTW_PREFIX@fftw.la \
	 		  @THREADLIBS@

lib@FFTW_PREFIX@rfftw_threads_la_SOURCES = \
                              rexec_threads.c rexec2_threads.c \
			      rfftwnd_threads.c \
                              rfftw_f77_threads.c \
	                      @FFTW_PREFIX@fftw_threads.h \
                              fftw_threads-int.h \
                              @FFTW_PREFIX@rfftw_threads.h
lib@FFTW_PREFIX@rfftw_threads_la_LDFLAGS = \
                              -version-info @SHARED_VERSION_INFO@ \
                              -rpath $(libdir)

@FFTW_PREFIX1@rfftw_threads.h: rfftw_threads.h
	rm -f $@
	sed 's/\"fftw/\"@FFTW_PREFIX@fftw/g;s/\"rfftw/\"@FFTW_PREFIX@rfftw/g' $(srcdir)/rfftw_threads.h > $@

CLEANFILES = @FFTW_PREFIX1@fftw_threads.h \
             @FFTW_PREFIX1@rfftw_threads.h

rfftw_threads_test_SOURCES = rfftw_threads_test.c
rfftw_threads_test_LDADD = ../tests/test_main.o \
	                  lib@FFTW_PREFIX@rfftw_threads.la \
                          lib@FFTW_PREFIX@fftw_threads.la \
                          $(RFFTWDIR)/lib@FFTW_PREFIX@rfftw.la \
                          $(FFTWDIR)/lib@FFTW_PREFIX@fftw.la \
			  @THREADLIBS@

# for some reason, automake tries to use autoheader in order to
# generate config.h.in, and fails because config.h.in is GNU-lly
# incorrect.  Just disable autoheader
AUTOHEADER=echo

check: @FFTW_THREADS_PROGLIST@
	@set fnord $(MAKEFLAGS); amf=$$2; \
        list='@FFTW_THREADS_PROGLIST@'; for prog in $$list; do \
          target="$$prog""-check"; \
          echo "Making $$target"; \
          ($(MAKE) $$target) \
           || case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
        done && test -z "$$fail"

fftw_threads_test-check: fftw_threads_test
	./fftw_threads_test 1 -x 100 -r 0
	./fftw_threads_test 2 -x 100 -r 0
	./fftw_threads_test 3 -x 100 -r 0
	./fftw_threads_test 10 -x 100 -r 0
	@echo "--------------------------------------------------------------"
	@echo "    FFTW complex-complex threads transforms passed tests!"
	@echo "--------------------------------------------------------------"

rfftw_threads_test-check: rfftw_threads_test
	./rfftw_threads_test 1 -x 100 -r 0
	./rfftw_threads_test 2 -x 100 -r 0
	./rfftw_threads_test 3 -x 100 -r 0
	./rfftw_threads_test 10 -x 100 -r 0
	@echo "--------------------------------------------------------------"
	@echo "     RFFTW real-complex threads transforms passed tests!"
	@echo "--------------------------------------------------------------"
