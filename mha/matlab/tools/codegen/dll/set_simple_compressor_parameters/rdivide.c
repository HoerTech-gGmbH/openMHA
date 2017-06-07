/*
 * rdivide.c
 *
 * Code generation for function 'rdivide'
 *
 */

/* Include files */
#include "rt_nonfinite.h"
#include "set_simple_compressor_parameters.h"
#include "simple_compressor.h"
#include "rdivide.h"
#include "set_simple_compressor_parameters_emxutil.h"

/* Function Definitions */

/*
 *
 */
void b_rdivide(const emxArray_real_T *x, double y, emxArray_real_T *z)
{
  int i3;
  int loop_ub;
  i3 = z->size[0] * z->size[1];
  z->size[0] = 1;
  z->size[1] = x->size[1];
  emxEnsureCapacity((emxArray__common *)z, i3, (int)sizeof(double));
  loop_ub = x->size[0] * x->size[1];
  for (i3 = 0; i3 < loop_ub; i3++) {
    z->data[i3] = x->data[i3] / y;
  }
}

/*
 *
 */
void rdivide(const emxArray_real_T *y, emxArray_real_T *z)
{
  int i2;
  int loop_ub;
  i2 = z->size[0] * z->size[1];
  z->size[0] = 1;
  z->size[1] = y->size[1];
  emxEnsureCapacity((emxArray__common *)z, i2, (int)sizeof(double));
  loop_ub = y->size[0] * y->size[1];
  for (i2 = 0; i2 < loop_ub; i2++) {
    z->data[i2] = -1.0 / y->data[i2];
  }
}

/* End of code generation (rdivide.c) */
