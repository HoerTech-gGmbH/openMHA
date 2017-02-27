noinst_PROGRAMS = fftw_test rfftw_test
noinst_HEADERS = test_main.h

FFTWDIR=../fftw
RFFTWDIR=../rfftw
INCLUDES = -I$(srcdir)/$(FFTWDIR) -I$(srcdir)/$(RFFTWDIR)

fftw_test_SOURCES = fftw_test.c test_main.c
fftw_test_LDADD = $(FFTWDIR)/lib@FFTW_PREFIX@fftw.la

rfftw_test_SOURCES = rfftw_test.c test_main.c
rfftw_test_LDADD = $(RFFTWDIR)/lib@FFTW_PREFIX@rfftw.la \
                   $(FFTWDIR)/lib@FFTW_PREFIX@fftw.la

check: fftw-tests rfftw-tests 

fftw-tests: fftw_test
	./fftw_test -p 0
	./fftw_test -x 128 -a 0
	./fftw_test -x 32 -a 2
	./fftw_test -x 100 -r 0
	@echo "--------------------------------------------------------------"
	@echo "         FFTW complex-complex transforms passed tests!"
	@echo "--------------------------------------------------------------"

rfftw-tests: rfftw_test
	./rfftw_test -p 0
	./rfftw_test -x 128 -a 0
	./rfftw_test -x 32 -a 2
	./rfftw_test -x 100 -r 0
	@echo "--------------------------------------------------------------"
	@echo "          RFFTW real-complex transforms passed tests!"
	@echo "--------------------------------------------------------------"
