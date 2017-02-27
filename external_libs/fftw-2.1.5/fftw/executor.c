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
 * executor.c -- execute the fft
 */

/* $Id: executor.c,v 1.68 2003/03/16 23:43:46 stevenj Exp $ */
#include "fftw-int.h"
#include <stdio.h>
#include <stdlib.h>

const char *fftw_version = "FFTW V" FFTW_VERSION " ($Id: executor.c,v 1.68 2003/03/16 23:43:46 stevenj Exp $)";

/*
 * This function is called in other files, so we cannot declare
 * it static. 
 */
void fftw_strided_copy(int n, fftw_complex *in, int ostride,
		       fftw_complex *out)
{
     int i;
     fftw_real r0, r1, i0, i1;
     fftw_real r2, r3, i2, i3;

     i = 0;

     for (; i < (n & 3); ++i) {
	  out[i * ostride] = in[i];
     }

     for (; i < n; i += 4) {
	  r0 = c_re(in[i]);
	  i0 = c_im(in[i]);
	  r1 = c_re(in[i + 1]);
	  i1 = c_im(in[i + 1]);
	  r2 = c_re(in[i + 2]);
	  i2 = c_im(in[i + 2]);
	  r3 = c_re(in[i + 3]);
	  i3 = c_im(in[i + 3]);
	  c_re(out[i * ostride]) = r0;
	  c_im(out[i * ostride]) = i0;
	  c_re(out[(i + 1) * ostride]) = r1;
	  c_im(out[(i + 1) * ostride]) = i1;
	  c_re(out[(i + 2) * ostride]) = r2;
	  c_im(out[(i + 2) * ostride]) = i2;
	  c_re(out[(i + 3) * ostride]) = r3;
	  c_im(out[(i + 3) * ostride]) = i3;
     }
}

static void executor_many(int n, const fftw_complex *in,
			  fftw_complex *out,
			  fftw_plan_node *p,
			  int istride,
			  int ostride,
			  int howmany, int idist, int odist,
			  fftw_recurse_kind recurse_kind)
{
     int s;

     switch (p->type) {
	 case FFTW_NOTW:
	      {
		   fftw_notw_codelet *codelet = p->nodeu.notw.codelet;

		   HACK_ALIGN_STACK_ODD;
		   for (s = 0; s < howmany; ++s)
			codelet(in + s * idist,
				out + s * odist,
				istride, ostride);
		   break;
	      }

	 default:
	      for (s = 0; s < howmany; ++s)
		   fftw_executor_simple(n, in + s * idist,
					out + s * odist,
					p, istride, ostride,
					recurse_kind);
     }
}

#ifdef FFTW_ENABLE_VECTOR_RECURSE

/* executor_many_vector is like executor_many, but it pushes the
   howmany loop down to the leaves of the transform: */
static void executor_many_vector(int n, const fftw_complex *in,
				 fftw_complex *out,
				 fftw_plan_node *p,
				 int istride,
				 int ostride,
				 int howmany, int idist, int odist)
{
     int s;

     switch (p->type) {
	 case FFTW_NOTW:
	      {
		   fftw_notw_codelet *codelet = p->nodeu.notw.codelet;

		   HACK_ALIGN_STACK_ODD;
		   for (s = 0; s < howmany; ++s)
			codelet(in + s * idist,
				out + s * odist,
				istride, ostride);
		   break;
	      }

	 case FFTW_TWIDDLE:
	      {
		   int r = p->nodeu.twiddle.size;
		   int m = n / r;
		   fftw_twiddle_codelet *codelet;
		   fftw_complex *W;

		   for (s = 0; s < r; ++s)
			executor_many_vector(m, in + s * istride, 
					     out + s * (m * ostride),
					     p->nodeu.twiddle.recurse,
					     istride * r, ostride,
					     howmany, idist, odist);

		   codelet = p->nodeu.twiddle.codelet;
		   W = p->nodeu.twiddle.tw->twarray;

		   /* This may not be the right thing.  We maybe should have
		      the howmany loop for the twiddle codelets at the
		      topmost level of the recursion, since odist is big;
		      i.e. separate recursions for twiddle and notwiddle. */
		   HACK_ALIGN_STACK_EVEN;
		   for (s = 0; s < howmany; ++s)
			codelet(out + s * odist, W, m * ostride, m, ostride);

		   break;
	      }

	 case FFTW_GENERIC:
	      {
		   int r = p->nodeu.generic.size;
		   int m = n / r;
		   fftw_generic_codelet *codelet;
		   fftw_complex *W;

		   for (s = 0; s < r; ++s)
			executor_many_vector(m, in + s * istride, 
					     out + s * (m * ostride),
					     p->nodeu.generic.recurse,
					     istride * r, ostride,
					     howmany, idist, odist);

		   codelet = p->nodeu.generic.codelet;
		   W = p->nodeu.generic.tw->twarray;
		   for (s = 0; s < howmany; ++s)
			codelet(out + s * odist, W, m, r, n, ostride);

		   break;
	      }

	 case FFTW_RADER:
	      {
		   int r = p->nodeu.rader.size;
		   int m = n / r;
		   fftw_rader_codelet *codelet;
		   fftw_complex *W;

		   for (s = 0; s < r; ++s)
			executor_many_vector(m, in + s * istride, 
					     out + s * (m * ostride),
					     p->nodeu.rader.recurse,
					     istride * r, ostride,
					     howmany, idist, odist);

		   codelet = p->nodeu.rader.codelet;
		   W = p->nodeu.rader.tw->twarray;
		   for (s = 0; s < howmany; ++s)
			codelet(out + s * odist, W, m, r, ostride,
				p->nodeu.rader.rader_data);

		   break;
	      }

	 default:
	      fftw_die("BUG in executor: invalid plan\n");
	      break;
     }     
}

#endif /* FFTW_ENABLE_VECTOR_RECURSE */

/*
 * Do *not* declare simple executor static--we need to call it
 * from other files...also, preface its name with "fftw_"
 * to avoid any possible name collisions. 
 */
void fftw_executor_simple(int n, const fftw_complex *in,
			  fftw_complex *out,
			  fftw_plan_node *p,
			  int istride,
			  int ostride,
			  fftw_recurse_kind recurse_kind)
{
     switch (p->type) {
	 case FFTW_NOTW:
	      HACK_ALIGN_STACK_ODD;
	      (p->nodeu.notw.codelet)(in, out, istride, ostride);
	      break;

	 case FFTW_TWIDDLE:
	      {
		   int r = p->nodeu.twiddle.size;
		   int m = n / r;
		   fftw_twiddle_codelet *codelet;
		   fftw_complex *W;

#ifdef FFTW_ENABLE_VECTOR_RECURSE
		   if (recurse_kind == FFTW_NORMAL_RECURSE)
#endif
			executor_many(m, in, out,
				      p->nodeu.twiddle.recurse,
				      istride * r, ostride,
				      r, istride, m * ostride,
				      FFTW_NORMAL_RECURSE);
#ifdef FFTW_ENABLE_VECTOR_RECURSE
		   else
			executor_many_vector(m, in, out,
					     p->nodeu.twiddle.recurse,
					     istride * r, ostride,
					     r, istride, m * ostride);
#endif

		   codelet = p->nodeu.twiddle.codelet;
		   W = p->nodeu.twiddle.tw->twarray;

		   HACK_ALIGN_STACK_EVEN;
		   codelet(out, W, m * ostride, m, ostride);

		   break;
	      }

	 case FFTW_GENERIC:
	      {
		   int r = p->nodeu.generic.size;
		   int m = n / r;
		   fftw_generic_codelet *codelet;
		   fftw_complex *W;

#ifdef FFTW_ENABLE_VECTOR_RECURSE
		   if (recurse_kind == FFTW_NORMAL_RECURSE)
#endif
			executor_many(m, in, out,
				      p->nodeu.generic.recurse,
				      istride * r, ostride,
				      r, istride, m * ostride,
                                      FFTW_NORMAL_RECURSE);
#ifdef FFTW_ENABLE_VECTOR_RECURSE
		   else
			executor_many_vector(m, in, out,
					     p->nodeu.generic.recurse,
					     istride * r, ostride,
					     r, istride, m * ostride);
#endif

		   codelet = p->nodeu.generic.codelet;
		   W = p->nodeu.generic.tw->twarray;
		   codelet(out, W, m, r, n, ostride);

		   break;
	      }

	 case FFTW_RADER:
	      {
		   int r = p->nodeu.rader.size;
		   int m = n / r;
		   fftw_rader_codelet *codelet;
		   fftw_complex *W;

#ifdef FFTW_ENABLE_VECTOR_RECURSE
		   if (recurse_kind == FFTW_NORMAL_RECURSE)
#endif
			executor_many(m, in, out,
				      p->nodeu.rader.recurse,
				      istride * r, ostride,
				      r, istride, m * ostride,
                                      FFTW_NORMAL_RECURSE);
#ifdef FFTW_ENABLE_VECTOR_RECURSE
		   else
			executor_many_vector(m, in, out,
					     p->nodeu.rader.recurse,
					     istride * r, ostride,
					     r, istride, m * ostride);
#endif

		   codelet = p->nodeu.rader.codelet;
		   W = p->nodeu.rader.tw->twarray;
		   codelet(out, W, m, r, ostride,
			   p->nodeu.rader.rader_data);

		   break;
	      }

	 default:
	      fftw_die("BUG in executor: invalid plan\n");
	      break;
     }
}

static void executor_simple_inplace(int n, fftw_complex *in,
				    fftw_complex *out,
				    fftw_plan_node *p,
				    int istride,
				    fftw_recurse_kind recurse_kind)
{
     switch (p->type) {
	 case FFTW_NOTW:
	      HACK_ALIGN_STACK_ODD;
	      (p->nodeu.notw.codelet)(in, in, istride, istride);
	      break;

	 default:
	      {
		   fftw_complex *tmp;

		   if (out)
			tmp = out;
		   else
			tmp = (fftw_complex *)
			    fftw_malloc(n * sizeof(fftw_complex));

		   fftw_executor_simple(n, in, tmp, p, istride, 1,
					recurse_kind);
		   fftw_strided_copy(n, tmp, istride, in);

		   if (!out)
			fftw_free(tmp);
	      }
     }
}

static void executor_many_inplace(int n, fftw_complex *in,
				  fftw_complex *out,
				  fftw_plan_node *p,
				  int istride,
				  int howmany, int idist,
				  fftw_recurse_kind recurse_kind)
{
     switch (p->type) {
	 case FFTW_NOTW:
	      {
		   fftw_notw_codelet *codelet = p->nodeu.notw.codelet;
		   int s;

		   HACK_ALIGN_STACK_ODD;
		   for (s = 0; s < howmany; ++s)
			codelet(in + s * idist,
				in + s * idist,
				istride, istride);
		   break;
	      }

	 default:
	      {
		   int s;
		   fftw_complex *tmp;
		   if (out)
			tmp = out;
		   else
			tmp = (fftw_complex *)
			    fftw_malloc(n * sizeof(fftw_complex));

		   for (s = 0; s < howmany; ++s) {
			fftw_executor_simple(n,
					     in + s * idist,
					     tmp,
					     p, istride, 1, recurse_kind);
			fftw_strided_copy(n, tmp, istride, in + s * idist);
		   }

		   if (!out)
			fftw_free(tmp);
	      }
     }
}

/* user interface */
void fftw(fftw_plan plan, int howmany, fftw_complex *in, int istride,
	  int idist, fftw_complex *out, int ostride, int odist)
{
     int n = plan->n;

     if (plan->flags & FFTW_IN_PLACE) {
	  if (howmany == 1) {
	       executor_simple_inplace(n, in, out, plan->root, istride,
				       plan->recurse_kind);
	  } else {
	       executor_many_inplace(n, in, out, plan->root, istride, howmany,
				     idist, plan->recurse_kind);
	  }
     } else {
	  if (howmany == 1) {
	       fftw_executor_simple(n, in, out, plan->root, istride, ostride,
				    plan->recurse_kind);
	  } else {
#ifdef FFTW_ENABLE_VECTOR_RECURSE
	       int vector_size = plan->vector_size;
	       if (vector_size <= 1)
#endif
		    executor_many(n, in, out, plan->root, istride, ostride,
				  howmany, idist, odist, plan->recurse_kind);
#ifdef FFTW_ENABLE_VECTOR_RECURSE
	       else {
		    int s;
		    int num_vects = howmany / vector_size;
		    fftw_plan_node *root = plan->root;

		    for (s = 0; s < num_vects; ++s)
			 executor_many_vector(n, 
					     in + s * (vector_size * idist), 
					     out + s * (vector_size * odist),
					     root,
					     istride, ostride,
					     vector_size, idist, odist);

		    s = howmany % vector_size;
		    if (s > 0)
			 executor_many(n,
				       in + num_vects * (vector_size * idist), 
				       out + num_vects * (vector_size * odist),
				       root,
				       istride, ostride,
				       s, idist, odist, 
				       FFTW_NORMAL_RECURSE);
	       }
#endif
	  }
     }
}

void fftw_one(fftw_plan plan, fftw_complex *in, fftw_complex *out)
{
     int n = plan->n;

     if (plan->flags & FFTW_IN_PLACE)
	  executor_simple_inplace(n, in, out, plan->root, 1,
				  plan->recurse_kind);
     else
	  fftw_executor_simple(n, in, out, plan->root, 1, 1,
			       plan->recurse_kind);
}
