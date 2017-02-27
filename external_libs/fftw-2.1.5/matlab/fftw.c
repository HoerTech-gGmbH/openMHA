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

#include <stdlib.h>
#include "fftw.h"

#include "mex.h"

/**************************************************************************/

/* MEX programs need to use special memory allocation routines,
   so we use the hooks provided by FFTW to ensure that MEX
   allocation is used: */

void *fftw_mex_malloc_hook(size_t n)
{
     void *buf;

     buf = mxMalloc(n);

     /* Call this routine so that we can retain allocations and
	data between calls to the FFTW MEX: */
     mexMakeMemoryPersistent(buf);

     return buf;
}

void fftw_mex_free_hook(void *buf)
{
     mxFree(buf);
}

void install_fftw_hooks(void)
{
     fftw_malloc_hook = fftw_mex_malloc_hook;
     fftw_free_hook = fftw_mex_free_hook;
}

/**************************************************************************/

/* We retain various information between calls to the FFTW MEX in
   order to maximize performance.  (Reusing plans, data, and
   allocated blocks where possible.)  This information is referenced
   by the following variables, which are initialized in the function
   initialize_fftw_mex_data. */

#define MAX_RANK 10

int first_call = 1; /* 1 if this is the first call to the FFTW MEX,
		       and nothing has been initialized yet.  0 otherwise. */

/* Keep track of the array dimensions that stored data (below) is for.
   When these dimensions changed, we have to recompute the plans,
   work arrays, etc. */
int  cur_rank = 0,         /* rank of the array */
     cur_dims[MAX_RANK],   /* dimensions */
     cur_N;                /* product of the dimensions */

/* Work arrays.  MATLAB stores complex numbers as separate real/imag.
   arrays, so we have to translate into our format before the FFT.
   In case allocation is slow, we retain these work arrays between
   calls so that they can be reused. */
fftw_complex *input_work = 0, *output_work = 0;

/* The number of floating point operations required for the FFT.
   (Starting with FFTW 1.3, an exact count is computed by the planner.)
   This is used to update MATLAB's flops count. */
int fftw_mex_flops = 0, ifftw_mex_flops = 0;

/* Plans.  These are computed once and then reused as long as the
   dimensions of the array don't changed.  At any point in time,
   at most two plans are cached: a forward and backwards plan,
   either for one- or multi-dimensional transforms. */
fftw_plan p = 0, ip = 0;
fftwnd_plan pnd = 0, ipnd = 0;

/**************************************************************************/

int compute_fftw_mex_flops(fftw_direction dir)
{
#ifdef FFTW_HAS_COUNT_PLAN_OPS /* this feature will be in FFTW 1.3 */
     fftw_op_count ops;

     if (dir == FFTW_FORWARD) {
	  if (cur_rank == 1)
	       fftw_count_plan_ops(p,&ops);
	  else
	       fftwnd_count_plan_ops(pnd,&ops);
     }
     else {
	  if (cur_rank == 1)
	       fftw_count_plan_ops(ip,&ops);
	  else
	       fftwnd_count_plan_ops(ipnd,&ops);
     }

     return (ops.fp_additions + ops.fp_multiplications);
#else
     return 0;
#endif
}

/**************************************************************************/

/* The following functions destroy and/or initialize the data that
   FFTW-MEX caches between calls. */

void destroy_fftw_mex_data(void) {
     if (output_work != input_work)
	  fftw_free(output_work);
     if (input_work)
	  fftw_free(input_work);
     if (p)
	  fftw_destroy_plan(p);
     if (pnd)
	  fftwnd_destroy_plan(pnd);
     if (ip)
	  fftw_destroy_plan(ip);
     if (ipnd)
	  fftwnd_destroy_plan(ipnd);

     cur_rank = 0;
     input_work = output_work = 0;
     ip = p = 0;
     ipnd = pnd = 0;
}

/* This function is called when MATLAB exits or the MEX file is
   cleared, in which case we want to dispose of all data and
   free any allocated blocks. */

void fftw_mex_exit_function(void)
{
     if (!first_call) {
	  destroy_fftw_mex_data();
	  fftw_forget_wisdom();
	  first_call = 1;
     }
}

#define MAGIC(x) #x
#define STRINGIZE(x) MAGIC(x)

/* Initialize the cached data each time the MEX file is called.  First,
   we check if we have previously computed plans and data for these
   array dimensions.  Only if the dimensions have changed since the
   last call must we recompute the plans, etc. */

void initialize_fftw_mex_data(int rank, const int *dims, fftw_direction dir)
{
     int new_plan = 0;

     if (first_call) {
	  /* The following things need only be done once: */
	  install_fftw_hooks();
	  mexAtExit(fftw_mex_exit_function);
	  first_call = 0;
     }

     if (rank == 1) {
	  if (cur_rank != 1 || cur_dims[0] != dims[0]) {
	       destroy_fftw_mex_data();

	       cur_rank = 1;
	       cur_dims[0] = cur_N = dims[0];
	       
	       input_work = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * cur_N);
	       output_work = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * cur_N);
	       
	       new_plan = 1;
	  }
	  else if (dir == FFTW_FORWARD && !p ||
		   dir == FFTW_BACKWARD && !ip)
	       new_plan = 1;

	  if (new_plan) {
	       if (dir == FFTW_FORWARD) {
		    p = fftw_create_plan(cur_N,dir,
					 FFTW_MEASURE | FFTW_USE_WISDOM);

		    fftw_mex_flops = compute_fftw_mex_flops(dir);
	       }
	       else {
		    ip = fftw_create_plan(cur_N,dir,
					  FFTW_MEASURE | FFTW_USE_WISDOM);

		    ifftw_mex_flops = compute_fftw_mex_flops(dir);
	       }
	  }
     }
     else {
	  int same_dims = 1, dim;

	  if (cur_rank == rank)
	       for (dim = 0; dim < rank && same_dims; ++dim)
		    same_dims = (cur_dims[dim] == dims[rank-1-dim]);
	  else
	       same_dims = 0;

	  if (!same_dims) {
	       if (rank > MAX_RANK)
		    mexErrMsgTxt("Sorry, dimensionality > " STRINGIZE(MAX_RANK)
				 " is not supported.");

	       destroy_fftw_mex_data();

	       cur_rank = rank;

	       cur_N = 1;
	       for (dim = 0; dim < rank; ++dim)
		    cur_N *= (cur_dims[dim] = dims[rank-1-dim]);

	       input_work = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * cur_N);
	       output_work = input_work;

	       new_plan = 1;
	  }
          else if (dir == FFTW_FORWARD && !pnd ||
                   dir == FFTW_BACKWARD && !ipnd)
               new_plan = 1;

	  if (new_plan) {
	       if (dir == FFTW_FORWARD) {
		    pnd = fftwnd_create_plan(rank,cur_dims,dir,
					     FFTW_IN_PLACE | 
					     FFTW_MEASURE | FFTW_USE_WISDOM);
		    
		    fftw_mex_flops = compute_fftw_mex_flops(dir);
	       }
	       else {
		    ipnd = fftwnd_create_plan(rank,cur_dims,dir,
					      FFTW_IN_PLACE | 
					      FFTW_MEASURE | FFTW_USE_WISDOM);
		    
		    ifftw_mex_flops = compute_fftw_mex_flops(dir);
	       }
	  }

     }
}

/**************************************************************************/

/* MATLAB stores complex numbers as separate arrays for real and
   imaginary parts.  The following functions take the data in
   this format and pack it into a fftw_complex work array, or
   unpack it, respectively. The globals input_work and output_work
   are used as the arrays to pack to/unpack from.*/

void pack_input_work(double *input_re, double *input_im)
{
     int i;

     if (input_im)
	  for (i = 0; i < cur_N; ++i) {
	       c_re(input_work[i]) = input_re[i];
	       c_im(input_work[i]) = input_im[i];
	  }
     else
	  for (i = 0; i < cur_N; ++i) {
	       c_re(input_work[i]) = input_re[i];
	       c_im(input_work[i]) = 0.0;
	  }
}

void unpack_output_work(double *output_re, double *output_im)
{
     int i;

     for (i = 0; i < cur_N; ++i) {
	  output_re[i] = c_re(output_work[i]);
	  output_im[i] = c_im(output_work[i]);
     }
}

/**************************************************************************/

/* The following function is called by MATLAB when the FFTW
   MEX is invoked from within the program.

   The rhs parameters are the list of arrays on the right-hand-side
   (rhs) of the MATLAB command--the arguments to FFTW.  The lhs
   parameters are the list of arrays on the left-hand-side (lhs) of
   the MATLAB command--these are what the output(s) of FFTW are
   assigned to.

   The syntax for the FFTW call in MATLAB is fftw(array,sign),
   as described in fftw.m  */

void mexFunction(int nlhs, mxArray *plhs[],
		 int nrhs, const mxArray *prhs[])
{
     int rank;
     const int *dims;
     int m, n;  /* Array is m x n, C-ordered */
     fftw_direction dir;

     if (nrhs != 2)
	  mexErrMsgTxt("Two input arguments are expected.");

     if (!mxIsDouble(prhs[0]))
	  mexErrMsgTxt("First input must be a double precision matrix.");
     if (mxIsSparse(prhs[0]))
	  mexErrMsgTxt("Sorry, sparse matrices are not currently supported.");

     if (mxGetM(prhs[1]) * mxGetN(prhs[1]) != 1)
	  mexErrMsgTxt("Second input must be a scalar (+/- 1).");

     if (mxGetScalar(prhs[1]) > 0.0)
	  dir = FFTW_BACKWARD;
     else
	  dir = FFTW_FORWARD;

     if ((rank = mxGetNumberOfDimensions(prhs[0])) == 2) {
	  int dims2[2];
	  m = mxGetM(prhs[0]);
	  n = mxGetN(prhs[0]);
	  if (m == 1 || n == 1) {
	       dims2[0] = m * n;
	       initialize_fftw_mex_data(1,dims2,dir);
	  }
	  else {
	       dims2[0] = m;
	       dims2[1] = n;
	       initialize_fftw_mex_data(2,dims2,dir);
	  }
     }
     else
	  initialize_fftw_mex_data(rank,dims = mxGetDimensions(prhs[0]),dir);

     pack_input_work(mxGetPr(prhs[0]),mxGetPi(prhs[0]));
     
     if (dir == FFTW_FORWARD) {
	  if (cur_rank == 1)
	       fftw(p,1, input_work,1,0, output_work,1,0);
	  else
	       fftwnd(pnd,1, input_work,1,0, 0,0,0);
	  
	  mexAddFlops(fftw_mex_flops);
     }
     else {
	  if (cur_rank == 1)
	       fftw(ip,1, input_work,1,0, output_work,1,0);
	  else
	       fftwnd(ipnd,1, input_work,1,0, 0,0,0);
	  
	  mexAddFlops(ifftw_mex_flops);
     }

     /* Create a matrix for the return argument. */
     if (cur_rank <= 2)
	  plhs[0] = mxCreateDoubleMatrix(m, n, mxCOMPLEX);
     else
	  plhs[0] = mxCreateNumericArray(rank,dims,
					 mxDOUBLE_CLASS,mxCOMPLEX);

     unpack_output_work(mxGetPr(plhs[0]),mxGetPi(plhs[0]));
}
