*
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

c	Simple program to demonstrate calling the wrapper routines
c	to perform 1D transforms in Fortran.  This program should be
c       linked with -lfftw -lm (assuming double-precision FFTW).  Note
c       also that the plan should be integer*8 on a 64-bit machine.

	program test

	implicit none

#include "fftw_f77.i"

	integer N
	parameter(N=4)
	double complex in, out
	dimension in(N),out(N)

	integer i

	integer plan

	write(*,*) 'Input array:'
	do i = 1,N,1
		in(i) = dcmplx(float(i),float(i+1))
		write(*,*) '    in(',i,') = ',in(i)
	enddo

	call fftw_f77_create_plan(plan,N,FFTW_FORWARD,FFTW_ESTIMATE)

	call fftw_f77(plan,1,in,1,0,out,1,0)

        write(*,*) 'Output array:'
        do i = 1,N,1
                write(*,*) '    out(',i,') = ',out(i)
        enddo

	call fftw_f77_destroy_plan(plan)

	call fftw_f77_create_plan(plan,N,FFTW_BACKWARD,FFTW_ESTIMATE)

        call fftw_f77(plan,1,out,1,0,in,1,0)

        write(*,*) 'Output array after inverse FFT:'
        do i = 1,N,1
                write(*,*) '    ',N,' * in(',i,') = ',in(i)
        enddo

        call fftw_f77_destroy_plan(plan)

	end
