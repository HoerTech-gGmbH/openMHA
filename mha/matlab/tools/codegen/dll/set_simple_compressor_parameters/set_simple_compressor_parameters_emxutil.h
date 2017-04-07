/*
 * set_simple_compressor_parameters_emxutil.h
 *
 * Code generation for function 'set_simple_compressor_parameters_emxutil'
 *
 */

#ifndef __SET_SIMPLE_COMPRESSOR_PARAMETERS_EMXUTIL_H__
#define __SET_SIMPLE_COMPRESSOR_PARAMETERS_EMXUTIL_H__

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

  extern void emxEnsureCapacity(emxArray__common *emxArray, int oldNumel, int
    elementSize);
  extern void emxFree_real_T(emxArray_real_T **pEmxArray);
  extern void emxInit_real_T(emxArray_real_T **pEmxArray, int numDimensions);

#ifdef __cplusplus

}
#endif
#endif

/* End of code generation (set_simple_compressor_parameters_emxutil.h) */
