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
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "fftw-int.h"
#include "fftw_mpi.h"

#include "test_main.h"

#define my_printf if (io_okay) printf
#define my_fprintf if (io_okay) fprintf
#define my_fflush if (io_okay) fflush

int ncpus = 1;
int my_cpu = 0;
int only_parallel = 0;

char fftw_prefix[] = "fftw_mpi";

/*************************************************
 * Speed tests
 *************************************************/

#define MPI_TIME_FFT(fft,a,n,t) \
{ \
     double ts,te; \
     double total_t; \
     int iters = 1,iter; \
     zero_arr((n), (a)); \
     do { \
          MPI_Barrier(MPI_COMM_WORLD); \
          ts = MPI_Wtime(); \
          for (iter = 0; iter < iters; ++iter) fft; \
          te = MPI_Wtime(); \
          t = (total_t = (te - ts)) / iters; \
          iters *= 2; \
     } while (total_t < 2.0); \
}

void zero_arr(int n, fftw_complex * a)
{
     int i;
     for (i = 0; i < n; ++i)
	  c_re(a[i]) = c_im(a[i]) = 0.0;
}

void test_speed_aux(int n, fftw_direction dir, int flags, int specific)
{
     int local_n, local_start, local_n_after_transform,
	  local_start_after_transform, total_local_size, nalloc;
     fftw_complex *in, *work;
     fftw_plan plan = 0;
     fftw_mpi_plan mpi_plan;
     double t, t0 = 0.0;

     if (specific || !(flags & FFTW_IN_PLACE))
	  return;

     if (io_okay && !only_parallel)
	  plan = fftw_create_plan(n, dir, speed_flag | flags
				  | wisdom_flag | no_vector_flag);

     mpi_plan = fftw_mpi_create_plan(MPI_COMM_WORLD, n, dir,
				     speed_flag | flags
				     | wisdom_flag | no_vector_flag);

     CHECK(mpi_plan, "failed to create plan!");

     fftw_mpi_local_sizes(mpi_plan, &local_n, &local_start,
			  &local_n_after_transform,
			  &local_start_after_transform,
			  &total_local_size);

     if (io_okay && !only_parallel)
	  nalloc = n;
     else
	  nalloc = total_local_size;

     in = (fftw_complex *) fftw_malloc(nalloc * howmany_fields
				       * sizeof(fftw_complex));
     work = (fftw_complex *) fftw_malloc(nalloc * howmany_fields
					 * sizeof(fftw_complex));

     if (io_okay) {
	  WHEN_VERBOSE(2, fftw_mpi_print_plan(mpi_plan));
     }

     if (io_okay && !only_parallel) {
	  FFTW_TIME_FFT(fftw(plan, howmany_fields,
			     in, howmany_fields, 1, work, 1, 0),
			in, n * howmany_fields, t0);

	  fftw_destroy_plan(plan);

	  WHEN_VERBOSE(1, printf("time for one fft (uniprocessor): %s\n", smart_sprint_time(t0)));
     }
     
     MPI_TIME_FFT(fftw_mpi(mpi_plan, howmany_fields, in, NULL),
		  in, total_local_size * howmany_fields, t);

     if (io_okay) {
	  WHEN_VERBOSE(1, printf("time for one fft (%d cpus): %s", ncpus, smart_sprint_time(t)));
	  WHEN_VERBOSE(1, printf(" (%s/point)\n", smart_sprint_time(t / n)));
	  WHEN_VERBOSE(1, printf("\"mflops\" = 5 (n log2 n) / (t in microseconds)"
				 " = %f\n", howmany_fields * mflops(t, n)));
	  if (!only_parallel)
	       WHEN_VERBOSE(1, printf("parallel speedup: %f\n", t0 / t));
     }

     MPI_TIME_FFT(fftw_mpi(mpi_plan, howmany_fields, in, work),
		  in, total_local_size * howmany_fields, t);

     if (io_okay) {
	  WHEN_VERBOSE(1, printf("w/WORK: time for one fft (%d cpus): %s", ncpus, smart_sprint_time(t)));
	  WHEN_VERBOSE(1, printf(" (%s/point)\n", smart_sprint_time(t / n)));
	  WHEN_VERBOSE(1, printf("w/WORK: \"mflops\" = 5 (n log2 n) / (t in microseconds)"
				 " = %f\n", howmany_fields * mflops(t, n)));
	  if (!only_parallel)
	     WHEN_VERBOSE(1, printf("w/WORK: parallel speedup: %f\n", t0 / t));
     }

     fftw_free(in);
     fftw_free(work);
     fftw_mpi_destroy_plan(mpi_plan);

     WHEN_VERBOSE(1, my_printf("\n"));
}

void test_speed_nd_aux(struct size sz,
		       fftw_direction dir, int flags, int specific)
{
     int local_nx, local_x_start, local_ny_after_transpose,
	  local_y_start_after_transpose, total_local_size;
     fftw_complex *in, *work;
     fftwnd_plan plan = 0;
     fftwnd_mpi_plan mpi_plan;
     double t, t0 = 0.0;
     int i, N;
     
     if (sz.rank < 2)
	  return;

     /* only bench in-place multi-dim transforms */
     flags |= FFTW_IN_PLACE;	

     N = 1;
     for (i = 0; i < sz.rank; ++i)
	  N *= (sz.narray[i]);

     if (specific) {
	  return;
     } else {
	  if (io_okay && !only_parallel)
	       plan = fftwnd_create_plan(sz.rank, sz.narray,
					 dir, speed_flag | flags
					 | wisdom_flag | no_vector_flag);
	  mpi_plan = fftwnd_mpi_create_plan(MPI_COMM_WORLD, sz.rank, sz.narray,
					    dir, speed_flag | flags
					    | wisdom_flag | no_vector_flag);
     }
     CHECK(mpi_plan != NULL, "can't create plan");

     fftwnd_mpi_local_sizes(mpi_plan, &local_nx, &local_x_start,
			    &local_ny_after_transpose,
			    &local_y_start_after_transpose,
			    &total_local_size);

     if (io_okay && !only_parallel)
	  in = (fftw_complex *) fftw_malloc(N * howmany_fields *
					    sizeof(fftw_complex));
     else
	  in = (fftw_complex *) fftw_malloc(total_local_size * howmany_fields *
					    sizeof(fftw_complex));
     work = (fftw_complex *) fftw_malloc(total_local_size * howmany_fields *
					 sizeof(fftw_complex));
     
     if (io_okay && !only_parallel) {
	  FFTW_TIME_FFT(fftwnd(plan, howmany_fields,
			      in, howmany_fields, 1, 0, 0, 0),
		       in, N * howmany_fields, t0);

	  fftwnd_destroy_plan(plan);
	  
	  WHEN_VERBOSE(1, printf("time for one fft (uniprocessor): %s\n",
				 smart_sprint_time(t0)));
     }

     MPI_TIME_FFT(fftwnd_mpi(mpi_plan, howmany_fields,
			     in, NULL, FFTW_NORMAL_ORDER),
		   in, total_local_size * howmany_fields, t);

     if (io_okay) {
	  WHEN_VERBOSE(1, printf("NORMAL: time for one fft (%d cpus): %s",
				 ncpus, smart_sprint_time(t)));
	  WHEN_VERBOSE(1, printf(" (%s/point)\n", smart_sprint_time(t / N)));
	  WHEN_VERBOSE(1, printf("NORMAL: \"mflops\" = 5 (N log2 N) / "
				 "(t in microseconds)"
				 " = %f\n", howmany_fields * mflops(t, N)));
	  if (!only_parallel)
	     WHEN_VERBOSE(1, printf("NORMAL: parallel speedup: %f\n", t0 / t));
     }

     MPI_TIME_FFT(fftwnd_mpi(mpi_plan, howmany_fields,
			     in, NULL, FFTW_TRANSPOSED_ORDER),
		   in, total_local_size * howmany_fields, t);

     if (io_okay) {
	  WHEN_VERBOSE(1, printf("TRANSP.: time for one fft (%d cpus): %s",
				 ncpus, smart_sprint_time(t)));
	  WHEN_VERBOSE(1, printf(" (%s/point)\n", smart_sprint_time(t / N)));
	  WHEN_VERBOSE(1, printf("TRANSP.: \"mflops\" = 5 (N log2 N) / "
				 "(t in microseconds)"
				 " = %f\n", howmany_fields * mflops(t, N)));
	  if (!only_parallel)
	    WHEN_VERBOSE(1, printf("TRANSP.: parallel speedup: %f\n", t0 / t));
     }

     MPI_TIME_FFT(fftwnd_mpi(mpi_plan, howmany_fields,
			     in, work, FFTW_NORMAL_ORDER),
		   in, total_local_size * howmany_fields, t);

     if (io_okay) {
	  WHEN_VERBOSE(1, printf("NORMAL,w/WORK: time for one fft (%d cpus): %s",
				 ncpus, smart_sprint_time(t)));
	  WHEN_VERBOSE(1, printf(" (%s/point)\n", smart_sprint_time(t / N)));
	  WHEN_VERBOSE(1, printf("NORMAL,w/WORK: \"mflops\" = 5 (N log2 N) / "
				 "(t in microseconds)"
				 " = %f\n", howmany_fields * mflops(t, N)));
	  if (!only_parallel)
	       WHEN_VERBOSE(1, printf("NORMAL,w/WORK: parallel speedup: %f\n", t0 / t));
     }

     MPI_TIME_FFT(fftwnd_mpi(mpi_plan, howmany_fields,
			     in, work, FFTW_TRANSPOSED_ORDER),
		   in, total_local_size * howmany_fields, t);

     if (io_okay) {
	  WHEN_VERBOSE(1, printf("TRANSP.,w/WORK: time for one fft (%d cpus): %s",
				 ncpus, smart_sprint_time(t)));
	  WHEN_VERBOSE(1, printf(" (%s/point)\n", smart_sprint_time(t / N)));
	  WHEN_VERBOSE(1, printf("TRANSP.,w/WORK: \"mflops\" = 5 (N log2 N) / "
				 "(t in microseconds)"
				 " = %f\n", howmany_fields * mflops(t, N)));
	  if (!only_parallel)
	       WHEN_VERBOSE(1, printf("TRANSP.,w/WORK: parallel speedup: %f\n", t0 / t));
     }

     fftwnd_mpi_destroy_plan(mpi_plan);

     fftw_free(in);
     fftw_free(work);

     WHEN_VERBOSE(1, my_printf("\n"));
}

/*************************************************
 * correctness tests
 *************************************************/

void test_out_of_place(int n, int istride, int ostride,
		       int howmany, fftw_direction dir,
		       fftw_plan validated_plan,
		       int specific)
{
     /* one-dim. out-of-place transforms will never be supported in MPI */
     WHEN_VERBOSE(2, my_printf("N/A\n"));
}

void test_in_place(int n, int istride, int howmany, fftw_direction dir,
		   fftw_plan validated_plan, int specific)
{
     int local_n, local_start, local_n_after_transform,
	  local_start_after_transform, total_local_size;
     fftw_complex *in1, *work = NULL, *in2, *out2;
     fftw_mpi_plan plan;
     int i;
     int flags = measure_flag | wisdom_flag | FFTW_IN_PLACE;

     if (specific) {
	  WHEN_VERBOSE(2, my_printf("N/A\n"));
	  return;
     }

     if (coinflip())
	  flags |= FFTW_THREADSAFE;

     plan = fftw_mpi_create_plan(MPI_COMM_WORLD, n, dir, flags);

     fftw_mpi_local_sizes(plan, &local_n, &local_start,
			  &local_n_after_transform,
			  &local_start_after_transform,
			  &total_local_size);

     in1 = (fftw_complex *) fftw_malloc(total_local_size 
					* sizeof(fftw_complex) * howmany);
     if (coinflip()) {
	  WHEN_VERBOSE(2, my_printf("w/work..."));
	  work = (fftw_complex *) fftw_malloc(total_local_size
                                        * sizeof(fftw_complex) * howmany);
     }
     in2 = (fftw_complex *) fftw_malloc(n * sizeof(fftw_complex) * howmany);
     out2 = (fftw_complex *) fftw_malloc(n * sizeof(fftw_complex) * howmany);

     /* generate random inputs */
     for (i = 0; i < n * howmany; ++i) {
	  c_re(in2[i]) = DRAND();
	  c_im(in2[i]) = DRAND();
     }
     for (i = 0; i < local_n * howmany; ++i) {
	  c_re(in1[i]) = c_re(in2[i + local_start*howmany]);
	  c_im(in1[i]) = c_im(in2[i + local_start*howmany]);
     }	  

     /* fft-ize */
     fftw_mpi(plan, howmany, in1, work);

     fftw_mpi_destroy_plan(plan);

     fftw(validated_plan, howmany, in2, howmany, 1, out2, howmany, 1);

     CHECK(compute_error_complex(in1, 1,
				 out2 + local_start_after_transform*howmany, 1,
				 howmany*local_n_after_transform) < TOLERANCE,
	   "test_in_place: wrong answer");

     WHEN_VERBOSE(2, my_printf("OK\n"));

     fftw_free(in1);
     fftw_free(work);
     fftw_free(in2);
     fftw_free(out2);
}

void test_out_of_place_both(int n, int istride, int ostride,
			    int howmany,
			    fftw_plan validated_plan_forward,
			    fftw_plan validated_plan_backward)
{
}

void test_in_place_both(int n, int istride, int howmany,
			fftw_plan validated_plan_forward,
			fftw_plan validated_plan_backward)
{
     WHEN_VERBOSE(2,
		  my_printf("TEST CORRECTNESS (in place, FFTW_FORWARD, %s) "
			 "n = %d  istride = %d  howmany = %d\n",
			 SPECIFICP(0),
			 n, istride, howmany));
     test_in_place(n, istride, howmany, FFTW_FORWARD,
		   validated_plan_forward, 0);
     
     WHEN_VERBOSE(2,
		  my_printf("TEST CORRECTNESS (in place, FFTW_BACKWARD, %s) "
			 "n = %d  istride = %d  howmany = %d\n",
			 SPECIFICP(0),
			 n, istride, howmany));
     test_in_place(n, istride, howmany, FFTW_BACKWARD,
		   validated_plan_backward, 0);
}

void test_correctness(int n)
{
     int howmany;
     fftw_plan validated_plan_forward, validated_plan_backward;

     WHEN_VERBOSE(1,
		  my_printf("Testing correctness for n = %d...", n);
		  my_fflush(stdout));

     /* produce a good plan */
     validated_plan_forward =
	 fftw_create_plan(n, FFTW_FORWARD, measure_flag | wisdom_flag);
     validated_plan_backward =
	 fftw_create_plan(n, FFTW_BACKWARD, measure_flag | wisdom_flag);

     for (howmany = 1; howmany <= MAX_HOWMANY; ++howmany)
	  test_in_place_both(n, howmany, howmany,
			     validated_plan_forward,
			     validated_plan_backward);

     fftw_destroy_plan(validated_plan_forward);
     fftw_destroy_plan(validated_plan_backward);

     if (!(wisdom_flag & FFTW_USE_WISDOM) && chk_mem_leak)
	  fftw_check_memory_leaks();

     WHEN_VERBOSE(1, my_printf("OK\n"));
}

/*************************************************
 * multi-dimensional correctness tests
 *************************************************/

void testnd_out_of_place(int rank, int *n, fftw_direction dir,
			 fftwnd_plan validated_plan)
{
}

void testnd_in_place(int rank, int *n, fftw_direction dir,
		     fftwnd_plan validated_plan,
		     int alternate_api, int specific, int force_buffered)
{
     int local_nx, local_x_start, local_ny_after_transpose,
          local_y_start_after_transpose, total_local_size;
     int istride;
     int N, dim, i;
     fftw_complex *in1, *work = 0, *in2;
     fftwnd_mpi_plan p = 0;
     int flags = measure_flag | wisdom_flag | FFTW_IN_PLACE;

     if (specific || rank < 2)
	  return;

     if (coinflip())
	  flags |= FFTW_THREADSAFE;

     if (force_buffered)
	  flags |= FFTWND_FORCE_BUFFERED;

     N = 1;
     for (dim = 0; dim < rank; ++dim)
	  N *= n[dim];

     if (alternate_api && (rank == 2 || rank == 3)) {
	  if (rank == 2)
	       p = fftw2d_mpi_create_plan(MPI_COMM_WORLD,
					  n[0], n[1], dir, flags);
	  else
	       p = fftw3d_mpi_create_plan(MPI_COMM_WORLD,
					  n[0], n[1], n[2], dir, flags);
     }
     else		/* standard api */
	  p = fftwnd_mpi_create_plan(MPI_COMM_WORLD, rank, n, dir, flags);

     fftwnd_mpi_local_sizes(p, &local_nx, &local_x_start,
                            &local_ny_after_transpose,
                            &local_y_start_after_transpose,
                            &total_local_size);

     in1 = (fftw_complex *) fftw_malloc(total_local_size * MAX_STRIDE
					* sizeof(fftw_complex));
     if (coinflip()) {
	  WHEN_VERBOSE(1, my_printf("w/work..."));
	  work = (fftw_complex *) fftw_malloc(total_local_size * MAX_STRIDE
					      * sizeof(fftw_complex));
     }
     in2 = (fftw_complex *) fftw_malloc(N * sizeof(fftw_complex));

     for (istride = 1; istride <= MAX_STRIDE; ++istride) {
	  /* generate random inputs */
	  for (i = 0; i < N; ++i) {
	       c_re(in2[i]) = DRAND();
	       c_im(in2[i]) = DRAND();
	  }

	  for (i = 0; i < local_nx * (N/n[0]); ++i) {
	       int j;
	       for (j = 0; j < istride; ++j) {
		    c_re(in1[i * istride + j]) = c_re((in2 + local_x_start 
						       * (N/n[0])) [i]);
		    c_im(in1[i * istride + j]) = c_im((in2 + local_x_start
                                                       * (N/n[0])) [i]);
	       }
	  }

	  fftwnd_mpi(p, istride, in1, work, FFTW_NORMAL_ORDER);

	  fftwnd(validated_plan, 1, in2, 1, 1, NULL, 0, 0);

	  for (i = 0; i < istride; ++i)
	       CHECK(compute_error_complex(in1 + i, istride,
					   in2 + local_x_start * (N/n[0]),
					   1, local_nx * (N/n[0])) < TOLERANCE,
		     "testnd_in_place: wrong answer");
     }

     fftwnd_mpi_destroy_plan(p);

     fftw_free(in2);
     fftw_free(work);
     fftw_free(in1);
}

void testnd_correctness(struct size sz, fftw_direction dir,
			int alt_api, int specific, int force_buf)
{
     fftwnd_plan validated_plan;

     validated_plan = fftwnd_create_plan(sz.rank, sz.narray,
					 dir, measure_flag | wisdom_flag |
					 FFTW_IN_PLACE);

     testnd_in_place(sz.rank, sz.narray, dir, validated_plan, alt_api,
		     specific, force_buf);

     fftwnd_destroy_plan(validated_plan);
}

/*************************************************
 * planner tests
 *************************************************/

void test_planner(int rank)
{
     /*
      * create and destroy many plans, at random.  Check the
      * garbage-collecting allocator of twiddle factors
      */
     int i, dim;
     int r, s;
     fftw_mpi_plan p[PLANNER_TEST_SIZE];
     fftwnd_mpi_plan pnd[PLANNER_TEST_SIZE];
     int *narr, maxdim;

     chk_mem_leak = 0;
     verbose--;

     please_wait();
     if (rank < 1)
          rank = 1;

     narr = (int *) fftw_malloc(rank * sizeof(int));

     for (i = 0; i < PLANNER_TEST_SIZE; ++i) {
          p[i] = (fftw_mpi_plan) 0;
          pnd[i] = (fftwnd_mpi_plan) 0;
     }

     if (PLANNER_TEST_SIZE >= 8) {
	  p[0] = fftw_mpi_create_plan(MPI_COMM_WORLD, 1024, FFTW_FORWARD, 0);
	  p[1] = fftw_mpi_create_plan(MPI_COMM_WORLD, 1024, FFTW_FORWARD, 0);
	  p[2] = fftw_mpi_create_plan(MPI_COMM_WORLD, 1024, FFTW_BACKWARD, 0);
	  p[3] = fftw_mpi_create_plan(MPI_COMM_WORLD, 1024, FFTW_BACKWARD, 0);
	  p[4] = fftw_mpi_create_plan(MPI_COMM_WORLD, 1024, FFTW_FORWARD, 0);
	  p[5] = fftw_mpi_create_plan(MPI_COMM_WORLD, 1024, FFTW_FORWARD, 0);
	  p[6] = fftw_mpi_create_plan(MPI_COMM_WORLD, 1024, FFTW_BACKWARD, 0);
	  p[7] = fftw_mpi_create_plan(MPI_COMM_WORLD, 1024, FFTW_BACKWARD, 0);
     }

     maxdim = (int) pow(8192, 1.0/rank);

     for (i = 0; i < PLANNER_TEST_SIZE * PLANNER_TEST_SIZE; ++i) {
          r = rand();
          if (r < 0)
               r = -r;
          r = r % PLANNER_TEST_SIZE;

          for (dim = 0; dim < rank; ++dim) {
               do {
                    s = rand();
                    if (s < 0)
                         s = -s;
                    s = s % maxdim + 1;
               } while (s == 0);
               narr[dim] = s;
          }

          if (rank == 1) {
               if (p[r])
                    fftw_mpi_destroy_plan(p[r]);

               p[r] = fftw_mpi_create_plan(MPI_COMM_WORLD,
					   narr[0], random_dir(), 
					   measure_flag | wisdom_flag);
          }

	  if (rank > 1) {
	       if (pnd[r])
		    fftwnd_mpi_destroy_plan(pnd[r]);
	       
	       pnd[r] = fftwnd_mpi_create_plan(MPI_COMM_WORLD, rank, narr,
					       random_dir(), measure_flag |
					       wisdom_flag);
	  }

          if (i % (PLANNER_TEST_SIZE * PLANNER_TEST_SIZE / 20) == 0) {
               WHEN_VERBOSE(0, my_printf("test planner: so far so good\n"));
               WHEN_VERBOSE(0, my_printf("test planner: iteration %d"
					 " out of %d\n",
                              i, PLANNER_TEST_SIZE * PLANNER_TEST_SIZE));
          }
     }

     for (i = 0; i < PLANNER_TEST_SIZE; ++i) {
          if (p[i])
               fftw_mpi_destroy_plan(p[i]);
          if (pnd[i])
               fftwnd_mpi_destroy_plan(pnd[i]);
     }

     fftw_free(narr);
     verbose++;
     chk_mem_leak = 1;
}

/*************************************************
 * test initialization
 *************************************************/

void test_init(int *argc, char ***argv)
{
     int i;
     unsigned int seed;

     MPI_Init(argc,argv);
     MPI_Comm_size(MPI_COMM_WORLD,&ncpus);
     MPI_Comm_rank(MPI_COMM_WORLD,&my_cpu);

     /* Only process 0 gets to do I/O: */
     io_okay = my_cpu == 0;

     if (io_okay)
	  for (i = 1; i < *argc; ++i)
	       if (!strcmp((*argv)[i], "--only-parallel")) {
		    only_parallel = 1;
		    strcpy((*argv)[i], "");
	       }
     MPI_Bcast(&only_parallel, 1, MPI_INT, 0, MPI_COMM_WORLD);

     /* Make sure all processes use the same seed for random numbers: */
     seed = time(NULL);
     MPI_Bcast(&seed, 1, MPI_INT, 0, MPI_COMM_WORLD);
     srand(seed);

     fftw_die_hook = fftw_mpi_die; /* call MPI_Abort on failure */
}

void test_finish(void)
{
     MPI_Finalize();
}

void enter_paranoid_mode(void)
{
}

/* in MPI, only process 0 is guaranteed to have access to the argument list */
int get_option(int argc, char **argv, char *argval, int argval_maxlen)
{
     int c;
     int arglen;

     if (io_okay) {
	  c = default_get_option(argc,argv,argval,argval_maxlen);
	  arglen = strlen(argval) + 1;
     }

     MPI_Bcast(&c, 1, MPI_INT, 0, MPI_COMM_WORLD);
     MPI_Bcast(&arglen, 1, MPI_INT, 0, MPI_COMM_WORLD);
     MPI_Bcast(argval, arglen, MPI_CHAR, 0, MPI_COMM_WORLD);

     return c;
}
