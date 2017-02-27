# Makefile for the generation of FFTW sources/Makefile

install: allfiles
	sh install.sh
	make clean

allfiles: Makefile.fftw sources config.c rconfig.c

config.c:
	sh makeconfig.sh | indent >$@

rconfig.c:
	sh makerconfig.sh | indent >$@

sources:
	sh makesources.sh

gen:
	make -f Makefile.genfft

Makefile.fftw:
	sh makemakefile.sh

distclean: clean

clean:
	rm -f *~ a.out core *.c genfft *.zi *.zo nohup.out 
	rm -f *.cm[xio] *.o
	rm -f Makefile.fftw Makefile.rfftw
	rm -f *.c
