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

#include "fftw_f77_mpi.h"

#ifdef F77_FUNC_ /* only compile wrappers if fortran mangling is known */

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

/************************************************************************/

void F77_FUNC_(fftw_f77_mpi_create_plan,FFTW_F77_MPI_CREATE_PLAN)
(fftw_mpi_plan *p, void *comm, int *n, int *idir, int *flags)
{
     fftw_direction dir = *idir < 0 ? FFTW_FORWARD : FFTW_BACKWARD;

     *p = fftw_mpi_create_plan(FFTW_MPI_COMM_F2C(comm), *n,dir,*flags);
}

void F77_FUNC_(fftw_f77_mpi_destroy_plan,FFTW_F77_MPI_DESTROY_PLAN)
(fftw_mpi_plan *p)
{
     fftw_mpi_destroy_plan(*p);
}

void F77_FUNC_(fftw_f77_mpi,FFTW_F77_MPI)
(fftw_mpi_plan *p, int *n_fields, fftw_complex *local_data, 
 fftw_complex *work, int *use_work)
{
     fftw_mpi(*p, *n_fields, local_data, *use_work ? work : NULL);
}

void F77_FUNC_(fftw_f77_mpi_local_sizes,FFTW_F77_MPI_LOCAL_SIZES)
(fftw_mpi_plan *p,
 int *local_n, int *local_start,
 int *local_n_after_transform,
 int *local_start_after_transform,
 int *total_local_size)
{
     fftw_mpi_local_sizes(*p, local_n, local_start,
			  local_n_after_transform, local_start_after_transform,
			  total_local_size);
}

extern void fftw_reverse_int_array(int *a, int n);

void F77_FUNC_(fftwnd_f77_mpi_create_plan,FFTWND_F77_MPI_CREATE_PLAN)
(fftwnd_mpi_plan *p, void *comm, int *rank, int *n, int *idir, int *flags)
{
     fftw_direction dir = *idir < 0 ? FFTW_FORWARD : FFTW_BACKWARD;

     fftw_reverse_int_array(n,*rank);  /* column-major -> row-major */
     *p = fftwnd_mpi_create_plan(FFTW_MPI_COMM_F2C(comm),
				 *rank, n, dir, *flags);
     fftw_reverse_int_array(n,*rank);  /* reverse back */
}

void F77_FUNC_(fftw2d_f77_mpi_create_plan,FFTW2D_F77_MPI_CREATE_PLAN)
(fftwnd_mpi_plan *p, void *comm, int *nx, int *ny, int *idir, int *flags)
{
     fftw_direction dir = *idir < 0 ? FFTW_FORWARD : FFTW_BACKWARD;

     *p = fftw2d_mpi_create_plan(FFTW_MPI_COMM_F2C(comm), *ny,*nx,dir,*flags);
}

void F77_FUNC_(fftw3d_f77_mpi_create_plan,FFTW3D_F77_MPI_CREATE_PLAN)
(fftwnd_mpi_plan *p, void *comm, 
 int *nx, int *ny, int *nz, int *idir, int *flags)
{
     fftw_direction dir = *idir < 0 ? FFTW_FORWARD : FFTW_BACKWARD;

     *p = fftw3d_mpi_create_plan(FFTW_MPI_COMM_F2C(comm), 
				 *nz,*ny,*nx,dir,*flags);
}

void F77_FUNC_(fftwnd_f77_mpi_destroy_plan,FFTWND_F77_MPI_DESTROY_PLAN)
(fftwnd_mpi_plan *p)
{
     fftwnd_mpi_destroy_plan(*p);
}

void F77_FUNC_(fftwnd_f77_mpi,FFTWND_F77_MPI)
(fftwnd_mpi_plan *p, int *n_fields, fftw_complex *local_data,
 fftw_complex *work, int *use_work, int *ioutput_order)
{
     fftwnd_mpi_output_order output_order = *ioutput_order ? 
	  FFTW_TRANSPOSED_ORDER : FFTW_NORMAL_ORDER;

     fftwnd_mpi(*p, *n_fields, local_data, *use_work ? work : NULL, 
		output_order);
}

void F77_FUNC_(fftwnd_f77_mpi_local_sizes,FFTWND_F77_MPI_LOCAL_SIZES)
(fftwnd_mpi_plan *p,
 int *local_nx, int *local_x_start,
 int *local_ny_after_transform,
 int *local_y_start_after_transform,
 int *total_local_size)
{
     fftwnd_mpi_local_sizes(*p, local_nx, local_x_start,
			    local_ny_after_transform, 
			    local_y_start_after_transform,
			    total_local_size);
}

/****************************************************************************/

#ifdef __cplusplus
}                               /* extern "C" */
#endif                          /* __cplusplus */

#endif /* defined(F77_FUNC_) */
