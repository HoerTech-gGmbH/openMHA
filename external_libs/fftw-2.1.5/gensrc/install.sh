#! /bin/sh

. ./config

mv Makefile.fftw ${FFTW_SRCDIR}/Makefile.am
mv Makefile.rfftw ${RFFTW_SRCDIR}/Makefile.am
mv ${REAL2HC_PREFIX}*.c ${HC2REAL_PREFIX}*.c ${HC2HC_FORWARD_PREFIX}*.c \
   ${HC2HC_BACKWARD_PREFIX}*.c rconfig.c ${RFFTW_SRCDIR}
mv *.c ${FFTW_SRCDIR}
