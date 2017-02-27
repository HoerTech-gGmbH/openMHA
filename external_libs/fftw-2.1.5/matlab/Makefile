all: fftw.mex

# This variable should be set to the directory containing
# libfftw.a and fftw.h:
FFTWDIR = ../fftw

fftw.mex:
	mex fftw.c -I${FFTWDIR} -L${FFTWDIR}/.libs -lfftw
