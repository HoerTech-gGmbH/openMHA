/* -*- C -*- */
/*
 * Copyright (c) 1997-1999, 2003 Massachusetts Institute of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* fftw.h -- system-wide definitions */
/* $Id: config.h.in,v 1.30 2003/03/16 23:43:46 stevenj Exp $ */

/* configuration options (guessed by configure) */

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define if you have the gettimeofday function.  */
/* #undef HAVE_GETTIMEOFDAY */

/* Define if you have the BSDgettimeofday function.  */
/* #undef HAVE_BSDGETTIMEOFDAY */

/* Define if you have the <sys/time.h> header file.  */
/* #undef HAVE_SYS_TIME_H */

/* Define if you have the <unistd.h> header file.  */
/* #undef HAVE_UNISTD_H */

/* Define if you have the <getopt.h> header file.  */
/* #undef HAVE_GETOPT_H */

/* Define if you have the <malloc.h> header file */
/* #undef HAVE_MALLOC_H */

/* Define if you have gethrtime() a la Solaris 2 */
/* #undef HAVE_GETHRTIME */
/* #undef HAVE_HRTIME_T */

/* Define to sizeof int and long long, if available: */
#define SIZEOF_INT 0
#define SIZEOF_LONG_LONG 0

#if (SIZEOF_INT != 0) && (SIZEOF_LONG_LONG >= 2 * SIZEOF_INT)
#  define LONGLONG_IS_TWOINTS
#endif

/* Define to use "unsafe" modular multiply (can cause integer overflow
   and errors for transforms of large prime sizes using Rader). */
/* #undef FFTW_ENABLE_UNSAFE_MULMOD */

/* Define if you have getopt() */
/* #undef HAVE_GETOPT */

/* Define if you have getopt_long() */
/* #undef HAVE_GETOPT_LONG */

/* Define if you have isnan() */
/* #undef HAVE_ISNAN */

/* Define for enabling the high resolution Pentium timer */
/* #undef FFTW_ENABLE_PENTIUM_TIMER */

/*
 * When using FFTW_ENABLE_PENTIUM_TIMER, set FFTW_CYCLES_PER_SEC 
 * to your real CPU clock speed! 
 */
/* This is for 200 MHz */
/* #define FFTW_CYCLES_PER_SEC 200000000L */

/*
 * Define to enable a gcc/x86 specific hack that aligns
 * the stack to an 8-byte boundary 
 */
/* #undef FFTW_ENABLE_I386_HACKS */

/* Define when using a version of gcc that aligns the stack properly */
/* #undef FFTW_GCC_ALIGNS_STACK */

/* Define to enable extra runtime checks for debugging. */
/* #undef FFTW_DEBUG */

/* Define to enable vector-recurse feature. */
/* #undef FFTW_ENABLE_VECTOR_RECURSE */

/*
 * Define to enable extra runtime checks for the alignment of variables
 * in the codelets (causes coredump for misaligned double on x86). 
 */
/* #undef FFTW_DEBUG_ALIGNMENT */

#define FFTW_VERSION "2.1.5" 

/* Use Win32 high-resolution timer */
#if defined(__WIN32__) || defined(WIN32) || defined(_WINDOWS)
#  define HAVE_WIN32_TIMER
#  define HAVE_WIN32
#endif

/* Use MacOS Time Manager timer */
#if defined(MAC) || defined(macintosh)
#  define HAVE_MAC_TIMER
#  define HAVE_MACOS

/* Define to use nanosecond timer on PCI PowerMacs: */
/* (WARNING: experimental, use at your own risk.) */
/* #undef HAVE_MAC_PCI_TIMER */
#endif

/* define if you have alloca.h: */
/* #undef HAVE_ALLOCA_H */

/* define if you have the alloca function: */
/* #undef HAVE_ALLOCA */

/************************** threads configuration ************************/

/* The following preprocessor symbols select which threads library
   to use when compiling the FFTW threads parallel libraries: */

/* #undef FFTW_USING_SOLARIS_THREADS */
/* #undef FFTW_USING_POSIX_THREADS */
/* #undef FFTW_USING_BEOS_THREADS */
/* #undef FFTW_USING_MACH_THREADS */
/* #undef FFTW_USING_OPENMP_THREADS */
/* #undef FFTW_USING_SGIMP_THREADS */

/* on AIX, this gets defined to PTHREAD_CREATE_UNDETACHED, as that
   system uses a non-standard name for this attribute (sigh). */
/* #undef PTHREAD_CREATE_JOINABLE */

/* #undef HAVE_MACH_CTHREADS_H */
/* #undef HAVE_CTHREADS_H */
/* #undef HAVE_CTHREAD_H */

#ifdef HAVE_WIN32
#define FFTW_USING_WIN32_THREADS
#endif

#ifdef HAVE_MACOS
#define FFTW_USING_MACOS_THREADS
#endif

/*********************** fortran wrapper configuration *********************/

/* F77_FUNC_ is defined to a macro F77_FUNC_(name,NAME) by autoconf, that
   takes an identifier name (lower case) and NAME (upper case) and returns
   the appropriately mangled identifier for the Fortran linker.  On
   non-Unix systems you will have to define this manually.  For example,
   if your linker converts identifiers to lower-case followed by an
   underscore, you would do: #define F77_FUNC_(name,NAME) name ## _ 
*/
/* #undef F77_FUNC_ */

/* The following symbols control how MPI_Comm data structures are
   translated between Fortran and C for the fftw_mpi wrappers.  See
   the file mpi/fftw_f77_mpi.h for more information. */
/* #undef HAVE_MPI_COMM_F2C */
/* #undef FFTW_USE_F77_MPI_COMM */
/* #undef FFTW_USE_F77_MPI_COMM_P */
