FFTWDIR = ../fftw
FFTW_INCLUDE = -I$(FFTWDIR)

FFTW_CILK_OBJ = executor_cilk.o fftwnd_cilk.o 

LIBFFTW_CILK = libfftw_cilk.a
CILK = cilk
LIBS = -L../fftw/.libs -lfftw -lm

# Set where you want to install the library for "make install"
prefix = /usr/local
LIBDIR = $(prefix)/lib
INCLUDEDIR = $(prefix)/include

# On systems that have ranlib:
RANLIB = ranlib

# On systems that don't have ranlib
# RANLIB = echo ranlib

all:	$(LIBFFTW_CILK)

tests: test_cilk time_cilk

RM = rm

clean:	
	$(RM) -f *.o core a.out *~ *.s *.bak 

distclean: clean
	$(RM) -f $(LIBFFTW_CILK) test_cilk time_cilk *.out

####################### Cilk Libraries, etc. ####################

HEADERS = $(FFTWDIR)/fftw.h fftw_cilk.cilkh
CILKCFLAGS = $(FFTW_INCLUDE) $(CFLAGS) -O6 -DFFTW_USING_CILK

###################### Building fftw_cilk #######################

AR = ar

%.o:	%.cilk $(HEADERS)  Makefile
	$(CILK) $(CILKCFLAGS) -c $< -o $@

%.s:	%.c $(HEADERS)  Makefile
	$(CILK) $(CILKCFLAGS) -S $< -o $@


$(LIBFFTW_CILK): $(FFTW_CILK_OBJ)
	$(RM) -f $(LIBFFTW_CILK)
	$(AR) rv $(LIBFFTW_CILK) $(FFTW_CILK_OBJ)
	$(RANLIB) $(LIBFFTW_CILK)

install: $(LIBFFTW_CILK)
	$(CP) $(LIBFFTW_CILK) $(LIBDIR)
	$(RANLIB) $(LIBDIR)/$(LIBFFTW_CILK)
	$(CP) fftw_cilk.cilkh $(INCLUDEDIR)

test_cilk: test_cilk.o $(LIBFFTW_CILK) 
	$(CILK) $(LDFLAGS) test_cilk.o \
              $(LIBFFTW_CILK) $(LIBS) -o test_cilk

time_cilk: time_cilk.o $(LIBFFTW_CILK) 
	$(CILK) $(LDFLAGS) time_cilk.o \
              $(LIBFFTW_CILK) $(LIBS) -o time_cilk

