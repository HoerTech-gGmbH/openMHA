/*
 * rdivide.h
 *
 * Code generation for function 'rdivide'
 *
 */

#ifndef __RDIVIDE_H__
#define __RDIVIDE_H__

/* Include files */
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "rt_nonfinite.h"
#include "rtwtypes.h"
#include "set_simple_compressor_parameters_types.h"

/* Function Declarations */
#ifdef __cplusplus

extern "C" {

#endif

  extern void b_rdivide(const emxArray_real_T *x, double y, emxArray_real_T *z);
  extern void rdivide(const emxArray_real_T *y, emxArray_real_T *z);

#ifdef __cplusplus

}
#endif
#endif

/* End of code generation (rdivide.h) */
