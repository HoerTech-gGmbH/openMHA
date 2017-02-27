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

#include "fftw.h"
#include "config.h"

#ifdef F77_FUNC_ /* only compile wrappers if fortran mangling is known */

/* fftwf77.c:

   FORTRAN-callable "wrappers" for some of the FFTW routines.  To
   make these routines callable from FORTRAN, three things had to
   be done:

   * The routine names have to be in the style that is expected by
     the FORTRAN linker.  This is accomplished with the F77_FUNC_
     macro.  (See AC_F77_WRAPPERS in the autoconf manual.)

   * All parameters must be passed by reference.

   * Return values had to be converted into parameters (some
     Fortran implementations seem to have trouble calling C functions
     that return a value).

   Note that the "fftw_plan" and "fftwnd_plan" types are pointers.
   The calling FORTRAN code should use a type of the same size
   (probably "integer").

   The wrapper routines have the same name as the wrapped routine,
   except that "fftw" and "fftwnd" are replaced by "fftw_f77" and
   "fftwnd_f77".

*/

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

/************************************************************************/

void F77_FUNC_(fftw_f77_create_plan,FFTW_F77_CREATE_PLAN)
(fftw_plan *p, int *n, int *idir, int *flags)
{
     fftw_direction dir = *idir < 0 ? FFTW_FORWARD : FFTW_BACKWARD;

     *p = fftw_create_plan(*n,dir,*flags);
}

void F77_FUNC_(fftw_f77_destroy_plan,FFTW_F77_DESTROY_PLAN)
(fftw_plan *p)
{
     fftw_destroy_plan(*p);
}

void F77_FUNC_(fftw_f77,FFTW_F77)
(fftw_plan *p, int *howmany, fftw_complex *in, int *istride, int *idist,
 fftw_complex *out, int *ostride, int *odist)
{
     fftw(*p,*howmany,in,*istride,*idist,out,*ostride,*odist);
}

void F77_FUNC_(fftw_f77_one,FFTW_F77_ONE)
(fftw_plan *p, fftw_complex *in, fftw_complex *out)
{
     fftw_one(*p,in,out);
}

void fftw_reverse_int_array(int *a, int n)
{
     int i;

     for (i = 0; i < n/2; ++i) {
	  int swap_dummy = a[i];
	  a[i] = a[n - 1 - i];
	  a[n - 1 - i] = swap_dummy;
     }
}

void F77_FUNC_(fftwnd_f77_create_plan,FFTWND_F77_CREATE_PLAN)
(fftwnd_plan *p, int *rank, int *n, int *idir, int *flags)
{
     fftw_direction dir = *idir < 0 ? FFTW_FORWARD : FFTW_BACKWARD;

     fftw_reverse_int_array(n,*rank);  /* column-major -> row-major */
     *p = fftwnd_create_plan(*rank,n,dir,*flags);
     fftw_reverse_int_array(n,*rank);  /* reverse back */
}

void F77_FUNC_(fftw2d_f77_create_plan,FFTW2D_F77_CREATE_PLAN)
(fftwnd_plan *p, int *nx, int *ny, int *idir, int *flags)
{
     fftw_direction dir = *idir < 0 ? FFTW_FORWARD : FFTW_BACKWARD;

     *p = fftw2d_create_plan(*ny,*nx,dir,*flags);
}

void F77_FUNC_(fftw3d_f77_create_plan,FFTW3D_F77_CREATE_PLAN)
(fftwnd_plan *p, int *nx, int *ny, int *nz, int *idir, int *flags)
{
     fftw_direction dir = *idir < 0 ? FFTW_FORWARD : FFTW_BACKWARD;

     *p = fftw3d_create_plan(*nz,*ny,*nx,dir,*flags);
}

void F77_FUNC_(fftwnd_f77_destroy_plan,FFTWND_F77_DESTROY_PLAN)
(fftwnd_plan *p)
{
     fftwnd_destroy_plan(*p);
}

void F77_FUNC_(fftwnd_f77,FFTWND_F77)
(fftwnd_plan *p, int *howmany, fftw_complex *in, int *istride, int *idist,
 fftw_complex *out, int *ostride, int *odist)
{
     fftwnd(*p,*howmany,in,*istride,*idist,out,*ostride,*odist);
}

void F77_FUNC_(fftwnd_f77_one,FFTWND_F77_ONE)
(fftwnd_plan *p, fftw_complex *in, fftw_complex *out)
{
     fftwnd_one(*p,in,out);
}

/****************************************************************************/

#ifdef __cplusplus
}                               /* extern "C" */
#endif                          /* __cplusplus */

#endif /* defined(F77_FUNC_) */
