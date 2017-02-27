# The mpi stuff is only compiled if FFTW was configured with
# the --enable-mpi option.  This requires us to use the combination
# of foo_* and EXTRA_* variables below.

lib_LTLIBRARIES = @FFTW_MPI_LIBLIST@
include_HEADERS = @FFTW_MPI_INCLUDELIST@
noinst_PROGRAMS = @FFTW_MPI_PROGLIST@

EXTRA_LTLIBRARIES = lib@FFTW_PREFIX@fftw_mpi.la \
                    lib@FFTW_PREFIX@rfftw_mpi.la
EXTRA_HEADERS = @FFTW_PREFIX@fftw_mpi.h @FFTW_PREFIX@rfftw_mpi.h 
EXTRA_PROGRAMS = test_sched test_transpose_mpi fftw_mpi_test rfftw_mpi_test
EXTRA_DIST = README.f77

CC=@MPICC@

FFTWDIR=../fftw
RFFTWDIR=../rfftw
INCLUDES = -I$(srcdir)/../tests \
           -I$(srcdir)/$(FFTWDIR) -I$(srcdir)/$(RFFTWDIR) -I$(srcdir)

lib@FFTW_PREFIX@fftw_mpi_la_SOURCES = \
                         sched.c sched.h TOMS_transpose.c TOMS_transpose.h \
                         transpose_mpi.c fftwnd_mpi.c fftw_mpi.c \
                         @FFTW_PREFIX@fftw_mpi.h \
			 fftw_f77_mpi.c fftw_f77_mpi.h
lib@FFTW_PREFIX@fftw_mpi_la_LDFLAGS = \
                         -version-info @SHARED_VERSION_INFO@ \
                         -rpath $(libdir)

@FFTW_PREFIX1@fftw_mpi.h: fftw_mpi.h
	rm -f $@
	sed 's/\"fftw/\"@FFTW_PREFIX@fftw/g;s/\"rfftw/\"@FFTW_PREFIX@rfftw/g' $(srcdir)/fftw_mpi.h > $@

lib@FFTW_PREFIX@rfftw_mpi_la_SOURCES = \
                          rfftwnd_mpi.c @FFTW_PREFIX@rfftw_mpi.h \
			  rfftw_f77_mpi.c
lib@FFTW_PREFIX@rfftw_mpi_la_LDFLAGS = \
                          -version-info @SHARED_VERSION_INFO@ \
                          -rpath $(libdir)

@FFTW_PREFIX1@rfftw_mpi.h: rfftw_mpi.h
	rm -f $@
	sed 's/\"fftw/\"@FFTW_PREFIX@fftw/g;s/\"rfftw/\"@FFTW_PREFIX@rfftw/g' $(srcdir)/rfftw_mpi.h > $@

CLEANFILES = @FFTW_PREFIX1@fftw_mpi.h @FFTW_PREFIX1@rfftw_mpi.h

test_sched_SOURCES = test_sched.c
test_sched_LDADD = lib@FFTW_PREFIX@fftw_mpi.la \
                   $(FFTWDIR)/lib@FFTW_PREFIX@fftw.la @MPILIBS@

test_transpose_mpi_SOURCES = test_transpose_mpi.c
test_transpose_mpi_LDADD = lib@FFTW_PREFIX@fftw_mpi.la \
                           $(FFTWDIR)/lib@FFTW_PREFIX@fftw.la @MPILIBS@

fftw_mpi_test_SOURCES = fftw_mpi_test.c
fftw_mpi_test_LDADD = ../tests/test_main.o \
                      lib@FFTW_PREFIX@fftw_mpi.la \
                      $(FFTWDIR)/lib@FFTW_PREFIX@fftw.la @MPILIBS@

rfftw_mpi_test_SOURCES = rfftw_mpi_test.c
rfftw_mpi_test_LDADD = ../tests/test_main.o \
                       lib@FFTW_PREFIX@rfftw_mpi.la \
                       lib@FFTW_PREFIX@fftw_mpi.la \
		       $(RFFTWDIR)/lib@FFTW_PREFIX@rfftw.la \
                       $(FFTWDIR)/lib@FFTW_PREFIX@fftw.la \
                       @MPILIBS@

# for some reason, automake tries to use autoheader in order to
# generate config.h.in, and fails because config.h.in is GNU-lly
# incorrect.  Just disable autoheader
AUTOHEADER=echo
