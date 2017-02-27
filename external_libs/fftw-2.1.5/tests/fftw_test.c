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

/*
 * fftw_test.c : test program for complex-complex transforms
 */

/* $Id: fftw_test.c,v 1.103 2003/03/16 23:43:46 stevenj Exp $ */
#include "fftw-int.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "test_main.h"

char fftw_prefix[] = "fftw";

void test_ergun(int n, fftw_direction dir, fftw_plan plan);

/*************************************************
 * Speed tests
 *************************************************/

void zero_arr(int n, fftw_complex *a)
{
     int i;
     for (i = 0; i < n; ++i)
	  c_re(a[i]) = c_im(a[i]) = 0.0;
}

void test_speed_aux(int n, fftw_direction dir, int flags, int specific)
{
     fftw_complex *in, *out;
     fftw_plan plan;
     double t;
     fftw_time begin, end;

     in = (fftw_complex *) fftw_malloc(n * howmany_fields
				       * sizeof(fftw_complex));
     out = (fftw_complex *) fftw_malloc(n * howmany_fields
					* sizeof(fftw_complex));

     if (specific) {
	  begin = fftw_get_time();
	  plan = fftw_create_plan_specific(n, dir,
					   speed_flag | flags 
					   | wisdom_flag | no_vector_flag,
					   in, howmany_fields,
					   out, howmany_fields);
	  end = fftw_get_time();
     } else {
	  begin = fftw_get_time();
	  plan = fftw_create_plan(n, dir, speed_flag | flags 
				  | wisdom_flag | no_vector_flag);
	  end = fftw_get_time();
     }
     CHECK(plan != NULL, "can't create plan");

     t = fftw_time_to_sec(fftw_time_diff(end, begin));
     WHEN_VERBOSE(2, printf("time for planner: %f s\n", t));

     WHEN_VERBOSE(2, fftw_print_plan(plan));

     if (paranoid && !(flags & FFTW_IN_PLACE)) {
	  begin = fftw_get_time();
	  test_ergun(n, dir, plan);
	  end = fftw_get_time();
	  t = fftw_time_to_sec(fftw_time_diff(end, begin));
	  WHEN_VERBOSE(2, printf("time for validation: %f s\n", t));
     }
     FFTW_TIME_FFT(fftw(plan, howmany_fields,
			in, howmany_fields, 1, out, howmany_fields, 1),
		   in, n * howmany_fields, t);

     fftw_destroy_plan(plan);

     WHEN_VERBOSE(1, printf("time for one fft: %s", smart_sprint_time(t)));
     WHEN_VERBOSE(1, printf(" (%s/point)\n", smart_sprint_time(t / n)));
     WHEN_VERBOSE(1, printf("\"mflops\" = 5 (n log2 n) / (t in microseconds)"
			    " = %f\n", howmany_fields * mflops(t, n)));

     fftw_free(in);
     fftw_free(out);

     WHEN_VERBOSE(1, printf("\n"));
}

void test_speed_nd_aux(struct size sz,
		       fftw_direction dir, int flags, int specific)
{
     fftw_complex *in;
     fftwnd_plan plan;
     double t;
     fftw_time begin, end;
     int i, N;

     /* only bench in-place multi-dim transforms */
     flags |= FFTW_IN_PLACE;	

     N = 1;
     for (i = 0; i < sz.rank; ++i)
	  N *= (sz.narray[i]);

     in = (fftw_complex *) fftw_malloc(N * howmany_fields *
				       sizeof(fftw_complex));

     if (specific) {
	  begin = fftw_get_time();
	  plan = fftwnd_create_plan_specific(sz.rank, sz.narray, dir,
					     speed_flag | flags
					     | wisdom_flag | no_vector_flag,
					     in, howmany_fields, 0, 1);
     } else {
	  begin = fftw_get_time();
	  plan = fftwnd_create_plan(sz.rank, sz.narray,
				    dir, speed_flag | flags 
				    | wisdom_flag | no_vector_flag);
     }
     end = fftw_get_time();
     CHECK(plan != NULL, "can't create plan");

     t = fftw_time_to_sec(fftw_time_diff(end, begin));
     WHEN_VERBOSE(2, printf("time for planner: %f s\n", t));

     WHEN_VERBOSE(2, printf("\n"));
     WHEN_VERBOSE(2, (fftwnd_print_plan(plan)));
     WHEN_VERBOSE(2, printf("\n"));

     FFTW_TIME_FFT(fftwnd(plan, howmany_fields,
			  in, howmany_fields, 1, 0, 0, 0),
		   in, N * howmany_fields, t);

     fftwnd_destroy_plan(plan);

     WHEN_VERBOSE(1, printf("time for one fft: %s", smart_sprint_time(t)));
     WHEN_VERBOSE(1, printf(" (%s/point)\n", smart_sprint_time(t / N)));
     WHEN_VERBOSE(1, printf("\"mflops\" = 5 (N log2 N) / (t in microseconds)"
			    " = %f\n", howmany_fields * mflops(t, N)));

     fftw_free(in);

     WHEN_VERBOSE(1, printf("\n"));
}

/*************************************************
 * correctness tests
 *************************************************/
void fill_random(fftw_complex *a, int n)
{
     int i;

     /* generate random inputs */
     for (i = 0; i < n; ++i) {
	  c_re(a[i]) = DRAND();
	  c_im(a[i]) = DRAND();
     }
}

void array_copy(fftw_complex *out, fftw_complex *in, int n)
{
     int i;

     for (i = 0; i < n; ++i)
	  out[i] = in[i];
}

/* C = A + B */
void array_add(fftw_complex *C, fftw_complex *A, fftw_complex *B, int n)
{
     int i;

     for (i = 0; i < n; ++i) {
	  c_re(C[i]) = c_re(A[i]) + c_re(B[i]);
	  c_im(C[i]) = c_im(A[i]) + c_im(B[i]);
     }
}

/* C = A - B */
void array_sub(fftw_complex *C, fftw_complex *A, fftw_complex *B, int n)
{
     int i;

     for (i = 0; i < n; ++i) {
	  c_re(C[i]) = c_re(A[i]) - c_re(B[i]);
	  c_im(C[i]) = c_im(A[i]) - c_im(B[i]);
     }
}

/* B = rotate left A */
void array_rol(fftw_complex *B, fftw_complex *A,
	       int n, int n_before, int n_after)
{
     int i, ib, ia;

     for (ib = 0; ib < n_before; ++ib) {
	  for (i = 0; i < n - 1; ++i)
	       for (ia = 0; ia < n_after; ++ia)
		    B[(ib * n + i) * n_after + ia] =
			A[(ib * n + i + 1) * n_after + ia];

	  for (ia = 0; ia < n_after; ++ia)
	       B[(ib * n + n - 1) * n_after + ia] = A[ib * n * n_after + ia];
     }
}

/* A = alpha * A  (in place) */
void array_scale(fftw_complex *A, fftw_complex alpha, int n)
{
     int i;

     for (i = 0; i < n; ++i) {
	  fftw_complex a = A[i];
	  c_re(A[i]) = c_re(a) * c_re(alpha) - c_im(a) * c_im(alpha);
	  c_im(A[i]) = c_re(a) * c_im(alpha) + c_im(a) * c_re(alpha);
     }
}

void array_compare(fftw_complex *A, fftw_complex *B, int n)
{
     double d = compute_error_complex(A, 1, B, 1, n);
     if (d > TOLERANCE) {
	  fflush(stdout);
	  fprintf(stderr, "Found relative error %e\n", d);
	  fftw_die("failure in Ergun's verification procedure\n");
     }
}

/*
 * guaranteed out-of-place transform.  Does the necessary
 * copying if the plan is in-place.
 */
static void fftw_out_of_place(fftw_plan plan, int n,
			      fftw_complex *in, fftw_complex *out)
{
     if (plan->flags & FFTW_IN_PLACE) {
	  array_copy(out, in, n);
	  fftw(plan, 1, out, 1, n, (fftw_complex *)0, 1, n);
     } else {
	  fftw(plan, 1, in, 1, n, out, 1, n);
     }
}

/*
 * Implementation of the FFT tester described in
 *
 * Funda Ergün. Testing multivariate linear functions: Overcoming the
 * generator bottleneck. In Proceedings of the Twenty-Seventh Annual
 * ACM Symposium on the Theory of Computing, pages 407-416, Las Vegas,
 * Nevada, 29 May--1 June 1995.
 */
void test_ergun(int n, fftw_direction dir, fftw_plan plan)
{
     fftw_complex *inA, *inB, *inC, *outA, *outB, *outC;
     fftw_complex *tmp;
     fftw_complex impulse;
     int i;
     int rounds = 20;
     FFTW_TRIG_REAL twopin = FFTW_K2PI / (FFTW_TRIG_REAL) n;

     inA = (fftw_complex *) fftw_malloc(n * sizeof(fftw_complex));
     inB = (fftw_complex *) fftw_malloc(n * sizeof(fftw_complex));
     inC = (fftw_complex *) fftw_malloc(n * sizeof(fftw_complex));
     outA = (fftw_complex *) fftw_malloc(n * sizeof(fftw_complex));
     outB = (fftw_complex *) fftw_malloc(n * sizeof(fftw_complex));
     outC = (fftw_complex *) fftw_malloc(n * sizeof(fftw_complex));
     tmp = (fftw_complex *) fftw_malloc(n * sizeof(fftw_complex));

     WHEN_VERBOSE(2,
		  printf("Validating plan, n = %d, dir = %s\n", n,
			 dir == FFTW_FORWARD ? "FORWARD" : "BACKWARD"));

     /* test 1: check linearity */
     for (i = 0; i < rounds; ++i) {
	  fftw_complex alpha, beta;
	  c_re(alpha) = DRAND();
	  c_im(alpha) = DRAND();
	  c_re(beta) = DRAND();
	  c_im(beta) = DRAND();
	  fill_random(inA, n);
	  fill_random(inB, n);
	  fftw_out_of_place(plan, n, inA, outA);
	  fftw_out_of_place(plan, n, inB, outB);
	  array_scale(outA, alpha, n);
	  array_scale(outB, beta, n);
	  array_add(tmp, outA, outB, n);
	  array_scale(inA, alpha, n);
	  array_scale(inB, beta, n);
	  array_add(inC, inA, inB, n);
	  fftw_out_of_place(plan, n, inC, outC);
	  array_compare(outC, tmp, n);
     }

     /* test 2: check that the unit impulse is transformed properly */

     c_re(impulse) = 1.0;
     c_im(impulse) = 0.0;
     
     for (i = 0; i < n; ++i) {
	  /* impulse */
	  c_re(inA[i]) = 0.0;
	  c_im(inA[i]) = 0.0;
	  
	  /* transform of the impulse */
	  outA[i] = impulse;
     }
     inA[0] = impulse;
     
     for (i = 0; i < rounds; ++i) {
	  fill_random(inB, n);
	  array_sub(inC, inA, inB, n);
	  fftw_out_of_place(plan, n, inB, outB);
	  fftw_out_of_place(plan, n, inC, outC);
	  array_add(tmp, outB, outC, n);
	  array_compare(tmp, outA, n);
     }

     /* test 3: check the time-shift property */
     /* the paper performs more tests, but this code should be fine too */
     for (i = 0; i < rounds; ++i) {
	  int j;

	  fill_random(inA, n);
	  array_rol(inB, inA, n, 1, 1);
	  fftw_out_of_place(plan, n, inA, outA);
	  fftw_out_of_place(plan, n, inB, outB);
	  for (j = 0; j < n; ++j) {
	       FFTW_TRIG_REAL s = dir * FFTW_TRIG_SIN(j * twopin);
	       FFTW_TRIG_REAL c = FFTW_TRIG_COS(j * twopin);
	       c_re(tmp[j]) = c_re(outB[j]) * c - c_im(outB[j]) * s;
	       c_im(tmp[j]) = c_re(outB[j]) * s + c_im(outB[j]) * c;
	  }

	  array_compare(tmp, outA, n);
     }

     WHEN_VERBOSE(2, printf("Validation done\n"));

     fftw_free(tmp);
     fftw_free(outC);
     fftw_free(outB);
     fftw_free(outA);
     fftw_free(inC);
     fftw_free(inB);
     fftw_free(inA);
}

static void fftw_plan_hook_function(fftw_plan p)
{
     WHEN_VERBOSE(3, printf("Validating tentative plan\n"));
     WHEN_VERBOSE(3, fftw_print_plan(p));
     test_ergun(p->n, p->dir, p);
}

/* Same as test_ergun, but for multi-dimensional transforms: */
void testnd_ergun(int rank, int *n, fftw_direction dir, fftwnd_plan plan)
{
     fftw_complex *inA, *inB, *inC, *outA, *outB, *outC;
     fftw_complex *tmp;
     fftw_complex impulse;

     int N, n_before, n_after, dim;
     int i, which_impulse;
     int rounds = 20;
     FFTW_TRIG_REAL twopin;

     N = 1;
     for (dim = 0; dim < rank; ++dim)
	  N *= n[dim];

     inA = (fftw_complex *) fftw_malloc(N * sizeof(fftw_complex));
     inB = (fftw_complex *) fftw_malloc(N * sizeof(fftw_complex));
     inC = (fftw_complex *) fftw_malloc(N * sizeof(fftw_complex));
     outA = (fftw_complex *) fftw_malloc(N * sizeof(fftw_complex));
     outB = (fftw_complex *) fftw_malloc(N * sizeof(fftw_complex));
     outC = (fftw_complex *) fftw_malloc(N * sizeof(fftw_complex));
     tmp = (fftw_complex *) fftw_malloc(N * sizeof(fftw_complex));

     WHEN_VERBOSE(2,
		  printf("Validating plan, N = %d, dir = %s\n", N,
			 dir == FFTW_FORWARD ? "FORWARD" : "BACKWARD"));

     /* test 1: check linearity */
     for (i = 0; i < rounds; ++i) {
	  fftw_complex alpha, beta;
	  c_re(alpha) = DRAND();
	  c_im(alpha) = DRAND();
	  c_re(beta) = DRAND();
	  c_im(beta) = DRAND();
	  fill_random(inA, N);
	  fill_random(inB, N);
	  fftwnd(plan, 1, inA, 1, N, outA, 1, N);
	  fftwnd(plan, 1, inB, 1, N, outB, 1, N);
	  array_scale(outA, alpha, N);
	  array_scale(outB, beta, N);
	  array_add(tmp, outA, outB, N);
	  array_scale(inA, alpha, N);
	  array_scale(inB, beta, N);
	  array_add(inC, inA, inB, N);
	  fftwnd(plan, 1, inC, 1, N, outC, 1, N);
	  array_compare(outC, tmp, N);
     }

     /*
      * test 2: check that the unit impulse is transformed properly -- we
      * need to test both the real and imaginary impulses 
      */

     for (which_impulse = 0; which_impulse < 2; ++which_impulse) {
	  if (which_impulse == 0) {	/* real impulse */
	       c_re(impulse) = 1.0;
	       c_im(impulse) = 0.0;
	  } else {		/* imaginary impulse */
	       c_re(impulse) = 0.0;
	       c_im(impulse) = 1.0;
	  }

	  for (i = 0; i < N; ++i) {
	       /* impulse */
	       c_re(inA[i]) = 0.0;
	       c_im(inA[i]) = 0.0;

	       /* transform of the impulse */
	       outA[i] = impulse;
	  }
	  inA[0] = impulse;

	  for (i = 0; i < rounds; ++i) {
	       fill_random(inB, N);
	       array_sub(inC, inA, inB, N);
	       fftwnd(plan, 1, inB, 1, N, outB, 1, N);
	       fftwnd(plan, 1, inC, 1, N, outC, 1, N);
	       array_add(tmp, outB, outC, N);
	       array_compare(tmp, outA, N);
	  }
     }

     /* test 3: check the time-shift property */
     /* the paper performs more tests, but this code should be fine too */
     /* -- we have to check shifts in each dimension */

     n_before = 1;
     n_after = N;
     for (dim = 0; dim < rank; ++dim) {
	  int n_cur = n[dim];

	  n_after /= n_cur;
	  twopin = FFTW_K2PI / (FFTW_TRIG_REAL) n_cur;

	  for (i = 0; i < rounds; ++i) {
	       int j, jb, ja;

	       fill_random(inA, N);
	       array_rol(inB, inA, n_cur, n_before, n_after);
	       fftwnd(plan, 1, inA, 1, N, outA, 1, N);
	       fftwnd(plan, 1, inB, 1, N, outB, 1, N);

	       for (jb = 0; jb < n_before; ++jb)
		    for (j = 0; j < n_cur; ++j) {
			 FFTW_TRIG_REAL s = dir * FFTW_TRIG_SIN(j * twopin);
			 FFTW_TRIG_REAL c = FFTW_TRIG_COS(j * twopin);

			 for (ja = 0; ja < n_after; ++ja) {
			      c_re(tmp[(jb * n_cur + j) * n_after + ja]) =
				  c_re(outB[(jb * n_cur + j) * n_after + ja]) * c
				  - c_im(outB[(jb * n_cur + j) * n_after + ja]) * s;
			      c_im(tmp[(jb * n_cur + j) * n_after + ja]) =
				  c_re(outB[(jb * n_cur + j) * n_after + ja]) * s
				  + c_im(outB[(jb * n_cur + j) * n_after + ja]) * c;
			 }
		    }

	       array_compare(tmp, outA, N);
	  }

	  n_before *= n_cur;
     }

     WHEN_VERBOSE(2, printf("Validation done\n"));

     fftw_free(tmp);
     fftw_free(outC);
     fftw_free(outB);
     fftw_free(outA);
     fftw_free(inC);
     fftw_free(inB);
     fftw_free(inA);
}

void test_out_of_place(int n, int istride, int ostride,
		       int howmany, fftw_direction dir,
		       fftw_plan validated_plan,
		       int specific)
{
     fftw_complex *in1, *in2, *out1, *out2;
     fftw_plan plan;
     int i, j;
     int flags = measure_flag | wisdom_flag | FFTW_OUT_OF_PLACE;

     if (coinflip())
	  flags |= FFTW_THREADSAFE;

     in1 = (fftw_complex *) 
	  fftw_malloc(istride * n * sizeof(fftw_complex) * howmany);
     in2 = (fftw_complex *) 
	  fftw_malloc(n * sizeof(fftw_complex) * howmany);
     out1 = (fftw_complex *)
	  fftw_malloc(ostride * n * sizeof(fftw_complex) * howmany);
     out2 = (fftw_complex *)
	  fftw_malloc(n * sizeof(fftw_complex) * howmany);

     if (!specific)
	  plan = fftw_create_plan(n, dir, flags);
     else
	  plan = fftw_create_plan_specific(n, dir,
					   flags,
					   in1, istride, out1, ostride);

     /* generate random inputs */
     for (i = 0; i < n * howmany; ++i) {
	  c_re(in1[i * istride]) = c_re(in2[i]) = DRAND();
	  c_im(in1[i * istride]) = c_im(in2[i]) = DRAND();
     }

     /* 
      * fill in other positions of the array, to make sure that
      * fftw doesn't overwrite them 
      */
     for (j = 1; j < istride; ++j)
	  for (i = 0; i < n * howmany; ++i) {
	       c_re(in1[i * istride + j]) = i * istride + j;
	       c_im(in1[i * istride + j]) = i * istride - j;
	  }

     for (j = 1; j < ostride; ++j)
	  for (i = 0; i < n * howmany; ++i) {
	       c_re(out1[i * ostride + j]) = -i * ostride + j;
	       c_im(out1[i * ostride + j]) = -i * ostride - j;
	  }

     CHECK(plan != NULL, "can't create plan");
     WHEN_VERBOSE(2, fftw_print_plan(plan));

     /* fft-ize */
     if (howmany != 1 || istride != 1 || ostride != 1 || coinflip())
	  fftw(plan, howmany, in1, istride, n * istride, out1, ostride,
	       n * ostride);
     else
	  fftw_one(plan, in1, out1);

     fftw_destroy_plan(plan);

     /* check for overwriting */
     for (j = 1; j < istride; ++j)
	  for (i = 0; i < n * howmany; ++i)
	       CHECK(c_re(in1[i * istride + j]) == i * istride + j &&
		     c_im(in1[i * istride + j]) == i * istride - j,
		     "input has been overwritten");
     for (j = 1; j < ostride; ++j)
	  for (i = 0; i < n * howmany; ++i)
	       CHECK(c_re(out1[i * ostride + j]) == -i * ostride + j &&
		     c_im(out1[i * ostride + j]) == -i * ostride - j,
		     "output has been overwritten");

     for (i = 0; i < howmany; ++i) {
	  fftw(validated_plan, 1, in2 + n * i, 1, n, out2 + n * i, 1, n);
     }

     CHECK(compute_error_complex(out1, ostride, out2, 1, n * howmany) < TOLERANCE,
	   "test_out_of_place: wrong answer");
     WHEN_VERBOSE(2, printf("OK\n"));

     fftw_free(in1);
     fftw_free(in2);
     fftw_free(out1);
     fftw_free(out2);
}

void test_in_place(int n, int istride, int howmany, fftw_direction dir,
		   fftw_plan validated_plan, int specific)
{
     fftw_complex *in1, *in2, *out2;
     fftw_plan plan;
     int i, j;
     int flags = measure_flag | wisdom_flag | FFTW_IN_PLACE;

     if (coinflip())
	  flags |= FFTW_THREADSAFE;

     in1 = (fftw_complex *) fftw_malloc(istride * n * sizeof(fftw_complex) * howmany);
     in2 = (fftw_complex *) fftw_malloc(n * sizeof(fftw_complex) * howmany);
     out2 = (fftw_complex *) fftw_malloc(n * sizeof(fftw_complex) * howmany);

     if (!specific)
	  plan = fftw_create_plan(n, dir, flags);
     else
	  plan = fftw_create_plan_specific(n, dir, flags,
					   in1, istride,
					   (fftw_complex *) NULL, 0);

     /* generate random inputs */
     for (i = 0; i < n * howmany; ++i) {
	  c_re(in1[i * istride]) = c_re(in2[i]) = DRAND();
	  c_im(in1[i * istride]) = c_im(in2[i]) = DRAND();
     }

     /* 
      * fill in other positions of the array, to make sure that
      * fftw doesn't overwrite them 
      */
     for (j = 1; j < istride; ++j)
	  for (i = 0; i < n * howmany; ++i) {
	       c_re(in1[i * istride + j]) = i * istride + j;
	       c_im(in1[i * istride + j]) = i * istride - j;
	  }
     CHECK(plan != NULL, "can't create plan");
     WHEN_VERBOSE(2, fftw_print_plan(plan));

     /* fft-ize */
     if (howmany != 1 || istride != 1 || coinflip())
	  fftw(plan, howmany, in1, istride, n * istride,
	       (fftw_complex *) NULL, 0, 0);
     else
	  fftw_one(plan, in1, NULL);

     fftw_destroy_plan(plan);

     /* check for overwriting */
     for (j = 1; j < istride; ++j)
	  for (i = 0; i < n * howmany; ++i)
	       CHECK(c_re(in1[i * istride + j]) == i * istride + j &&
		     c_im(in1[i * istride + j]) == i * istride - j,
		     "input has been overwritten");

     for (i = 0; i < howmany; ++i) {
	  fftw(validated_plan, 1, in2 + n * i, 1, n, out2 + n * i, 1, n);
     }

     CHECK(compute_error_complex(in1, istride, out2, 1, n * howmany) < TOLERANCE,
	   "test_in_place: wrong answer");
     WHEN_VERBOSE(2, printf("OK\n"));

     fftw_free(in1);
     fftw_free(in2);
     fftw_free(out2);
}

void test_out_of_place_both(int n, int istride, int ostride,
			    int howmany,
			    fftw_plan validated_plan_forward,
			    fftw_plan validated_plan_backward)
{
     int specific;

     for (specific = 0; specific <= 1; ++specific) {
	  WHEN_VERBOSE(2,
	       printf("TEST CORRECTNESS (out of place, FFTW_FORWARD, %s)"
		   " n = %d  istride = %d  ostride = %d  howmany = %d\n",
		      SPECIFICP(specific),
		      n, istride, ostride, howmany));
	  test_out_of_place(n, istride, ostride, howmany, FFTW_FORWARD,
			    validated_plan_forward, specific);

	  WHEN_VERBOSE(2,
	      printf("TEST CORRECTNESS (out of place, FFTW_BACKWARD, %s)"
		   " n = %d  istride = %d  ostride = %d  howmany = %d\n",
		     SPECIFICP(specific),
		     n, istride, ostride, howmany));
	  test_out_of_place(n, istride, ostride, howmany, FFTW_BACKWARD,
			    validated_plan_backward, specific);
     }
}

void test_in_place_both(int n, int istride, int howmany,
			fftw_plan validated_plan_forward,
			fftw_plan validated_plan_backward)
{
     int specific;

     for (specific = 0; specific <= 1; ++specific) {
	  WHEN_VERBOSE(2,
		  printf("TEST CORRECTNESS (in place, FFTW_FORWARD, %s) "
			 "n = %d  istride = %d  howmany = %d\n",
			 SPECIFICP(specific),
			 n, istride, howmany));
	  test_in_place(n, istride, howmany, FFTW_FORWARD,
			validated_plan_forward, specific);

	  WHEN_VERBOSE(2,
		 printf("TEST CORRECTNESS (in place, FFTW_BACKWARD, %s) "
			"n = %d  istride = %d  howmany = %d\n",
			SPECIFICP(specific),
			n, istride, howmany));
	  test_in_place(n, istride, howmany, FFTW_BACKWARD,
			validated_plan_backward, specific);
     }
}

void test_correctness(int n)
{
     int istride, ostride, howmany;
     fftw_plan validated_plan_forward, validated_plan_backward;

     WHEN_VERBOSE(1,
		  printf("Testing correctness for n = %d...", n);
		  fflush(stdout));

     /* produce a *good* plan (validated by Ergun's test procedure) */
     validated_plan_forward =
	 fftw_create_plan(n, FFTW_FORWARD, measure_flag | wisdom_flag);
     test_ergun(n, FFTW_FORWARD, validated_plan_forward);
     validated_plan_backward =
	 fftw_create_plan(n, FFTW_BACKWARD, measure_flag | wisdom_flag);
     test_ergun(n, FFTW_BACKWARD, validated_plan_backward);

     for (istride = 1; istride <= MAX_STRIDE; ++istride)
	  for (ostride = 1; ostride <= MAX_STRIDE; ++ostride)
	       for (howmany = 1; howmany <= MAX_HOWMANY; ++howmany)
		    test_out_of_place_both(n, istride, ostride, howmany,
					   validated_plan_forward,
					   validated_plan_backward);

     for (istride = 1; istride <= MAX_STRIDE; ++istride)
	  for (howmany = 1; howmany <= MAX_HOWMANY; ++howmany)
	       test_in_place_both(n, istride, howmany,
				  validated_plan_forward,
				  validated_plan_backward);

     fftw_destroy_plan(validated_plan_forward);
     fftw_destroy_plan(validated_plan_backward);

     if (!(wisdom_flag & FFTW_USE_WISDOM) && chk_mem_leak)
	  fftw_check_memory_leaks();

     WHEN_VERBOSE(1, printf("OK\n"));
}

/*************************************************
 * multi-dimensional correctness tests
 *************************************************/

void testnd_out_of_place(int rank, int *n, fftw_direction dir,
			 fftwnd_plan validated_plan)
{
     int istride, ostride;
     int N, dim, i;
     fftw_complex *in1, *in2, *out1, *out2;
     fftwnd_plan p;
     int flags = measure_flag | wisdom_flag;

     if (coinflip())
	  flags |= FFTW_THREADSAFE;

     N = 1;
     for (dim = 0; dim < rank; ++dim)
	  N *= n[dim];

     in1 = (fftw_complex *) fftw_malloc(N * MAX_STRIDE * sizeof(fftw_complex));
     out1 = (fftw_complex *) fftw_malloc(N * MAX_STRIDE * sizeof(fftw_complex));
     in2 = (fftw_complex *) fftw_malloc(N * sizeof(fftw_complex));
     out2 = (fftw_complex *) fftw_malloc(N * sizeof(fftw_complex));

     p = fftwnd_create_plan(rank, n, dir, flags);

     for (istride = 1; istride <= MAX_STRIDE; ++istride) {
	  /* generate random inputs */
	  for (i = 0; i < N; ++i) {
	       int j;
	       c_re(in2[i]) = DRAND();
	       c_im(in2[i]) = DRAND();
	       for (j = 0; j < istride; ++j) {
		    c_re(in1[i * istride + j]) = c_re(in2[i]);
		    c_im(in1[i * istride + j]) = c_im(in2[i]);
	       }
	  }

	  for (ostride = 1; ostride <= MAX_STRIDE; ++ostride) {
	       int howmany = (istride < ostride) ? istride : ostride;

	       if (howmany != 1 || istride != 1 || ostride != 1 || coinflip())
		    fftwnd(p, howmany, in1, istride, 1, out1, ostride, 1);
	       else
		    fftwnd_one(p, in1, out1);

	       fftwnd(validated_plan, 1, in2, 1, 1, out2, 1, 1);

	       for (i = 0; i < howmany; ++i)
		    CHECK(compute_error_complex(out1 + i, ostride, out2, 1, N)
			  < TOLERANCE,
			  "testnd_out_of_place: wrong answer");
	  }
     }

     fftwnd_destroy_plan(p);

     fftw_free(out2);
     fftw_free(in2);
     fftw_free(out1);
     fftw_free(in1);
}

void testnd_in_place(int rank, int *n, fftw_direction dir,
		     fftwnd_plan validated_plan,
		     int alternate_api, int specific, int force_buffered)
{
     int istride;
     int N, dim, i;
     fftw_complex *in1, *in2, *out2;
     fftwnd_plan p;
     int flags = measure_flag | wisdom_flag | FFTW_IN_PLACE;

     if (coinflip())
	  flags |= FFTW_THREADSAFE;

     if (force_buffered)
	  flags |= FFTWND_FORCE_BUFFERED;

     N = 1;
     for (dim = 0; dim < rank; ++dim)
	  N *= n[dim];

     in1 = (fftw_complex *) fftw_malloc(N * MAX_STRIDE * sizeof(fftw_complex));
     in2 = (fftw_complex *) fftw_malloc(N * sizeof(fftw_complex));
     out2 = (fftw_complex *) fftw_malloc(N * sizeof(fftw_complex));

     if (!specific) {
	  if (alternate_api && (rank == 2 || rank == 3)) {
	       if (rank == 2)
		    p = fftw2d_create_plan(n[0], n[1], dir, flags);
	       else
		    p = fftw3d_create_plan(n[0], n[1], n[2], dir, flags);
	  } else		/* standard api */
	       p = fftwnd_create_plan(rank, n, dir, flags);
     } else {			/* specific plan creation */
	  if (alternate_api && (rank == 2 || rank == 3)) {
	       if (rank == 2)
		    p = fftw2d_create_plan_specific(n[0], n[1], dir, flags,
						    in1, 1,
					       (fftw_complex *) NULL, 1);
	       else
		    p = fftw3d_create_plan_specific(n[0], n[1], n[2], dir, flags,
						    in1, 1,
					       (fftw_complex *) NULL, 1);
	  } else		/* standard api */
	       p = fftwnd_create_plan_specific(rank, n, dir, flags,
					       in1, 1,
					       (fftw_complex *) NULL, 1);

     }

     for (istride = 1; istride <= MAX_STRIDE; ++istride) {
	  /* 
	   * generate random inputs */
	  for (i = 0; i < N; ++i) {
	       int j;
	       c_re(in2[i]) = DRAND();
	       c_im(in2[i]) = DRAND();
	       for (j = 0; j < istride; ++j) {
		    c_re(in1[i * istride + j]) = c_re(in2[i]);
		    c_im(in1[i * istride + j]) = c_im(in2[i]);
	       }
	  }

	  if (istride != 1 || istride != 1 || coinflip())
	       fftwnd(p, istride, in1, istride, 1, (fftw_complex *) NULL, 1, 1);
	  else
	       fftwnd_one(p, in1, NULL);

	  fftwnd(validated_plan, 1, in2, 1, 1, out2, 1, 1);

	  for (i = 0; i < istride; ++i)
	       CHECK(compute_error_complex(in1 + i, istride, out2, 1, N) < TOLERANCE,
		     "testnd_in_place: wrong answer");
     }

     fftwnd_destroy_plan(p);

     fftw_free(out2);
     fftw_free(in2);
     fftw_free(in1);
}

void testnd_correctness(struct size sz, fftw_direction dir,
			int alt_api, int specific, int force_buf)
{
     fftwnd_plan validated_plan;

     validated_plan = fftwnd_create_plan(sz.rank, sz.narray,
					 dir, measure_flag | wisdom_flag);
     testnd_ergun(sz.rank, sz.narray, dir, validated_plan);

     testnd_out_of_place(sz.rank, sz.narray, dir, validated_plan);
     testnd_in_place(sz.rank, sz.narray, dir, validated_plan, alt_api, specific, force_buf);

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
     fftw_plan p[PLANNER_TEST_SIZE];
     fftwnd_plan pnd[PLANNER_TEST_SIZE];
     int *narr, maxdim;

     chk_mem_leak = 0;
     verbose--;

     please_wait();
     if (rank < 1)
	  rank = 1;

     narr = (int *) fftw_malloc(rank * sizeof(int));

     maxdim = (int) pow(8192.0, 1.0/rank);

     for (i = 0; i < PLANNER_TEST_SIZE; ++i) {
	  p[i] = (fftw_plan) 0;
	  pnd[i] = (fftwnd_plan) 0;
     }

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
		    fftw_destroy_plan(p[r]);

	       p[r] = fftw_create_plan(narr[0], random_dir(), measure_flag |
				       wisdom_flag);
	       if (paranoid && narr[0] < 200)
		    test_correctness(narr[0]);
	  }

	  if (pnd[r])
	       fftwnd_destroy_plan(pnd[r]);

	  pnd[r] = fftwnd_create_plan(rank, narr,
				      random_dir(), measure_flag |
				      wisdom_flag);

	  if (i % (PLANNER_TEST_SIZE * PLANNER_TEST_SIZE / 20) == 0) {
	       WHEN_VERBOSE(0, printf("test planner: so far so good\n"));
	       WHEN_VERBOSE(0, printf("test planner: iteration %d out of %d\n",
			      i, PLANNER_TEST_SIZE * PLANNER_TEST_SIZE));
	  }
     }

     for (i = 0; i < PLANNER_TEST_SIZE; ++i) {
	  if (p[i])
	       fftw_destroy_plan(p[i]);
	  if (pnd[i])
	       fftwnd_destroy_plan(pnd[i]);
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
}

void test_finish(void)
{
}

void enter_paranoid_mode(void)
{
     fftw_plan_hook = fftw_plan_hook_function;
}

int get_option(int argc, char **argv, char *argval, int argval_maxlen)
{
     return default_get_option(argc,argv,argval,argval_maxlen);
}
