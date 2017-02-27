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

#ifndef FFTW_F77_MPI_H
#define FFTW_F77_MPI_H

#include "fftw_mpi.h"
#include "fftw-int.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***********************************************************************/

/* How do we pass MPI_Comm data types from Fortran?  Here are three
   possibilities, selected among by defining the appropriate cpp
   symbol (in fftw/config.h, preferably using the configure script):

   HAVE_MPI_COMM_F2C -- the MPI_Comm_f2c function is available
                        (this function is supplied e.g. by MPICH)

   FFTW_USE_F77_MPI_COMM -- Fortran gives us an (MPI_Comm *)

   FFTW_USE_F77_MPI_COMM_P -- MPI_Comm is a pointer, and Fortran
                              passes it to us directly by value (seems
			      unlikely).

   [default] -- ignore the comm parameter, and just use MPI_COMM_WORLD
                (this at least will always work, at the expense of flexibility)
*/

#if defined(HAVE_MPI_COMM_F2C)
#  define FFTW_MPI_COMM_F2C(comm) MPI_Comm_f2c(*((MPI_Comm *) comm))
#elif defined(FFTW_USE_F77_MPI_COMM)
#  define FFTW_MPI_COMM_F2C(comm) (* ((MPI_Comm *) comm))
#elif defined(FFTW_USE_F77_MPI_COMM_P)
#  define FFTW_MPI_COMM_F2C(comm) ((MPI_Comm) comm)
#else
#  define FFTW_MPI_COMM_F2C(comm) MPI_COMM_WORLD
#endif

/***********************************************************************/

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* FFTW_F77_MPI_H */
