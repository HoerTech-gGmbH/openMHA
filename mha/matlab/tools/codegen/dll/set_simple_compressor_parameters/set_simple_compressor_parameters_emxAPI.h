/*
 * set_simple_compressor_parameters_emxAPI.h
 *
 * Code generation for function 'set_simple_compressor_parameters_emxAPI'
 *
 */

#ifndef __SET_SIMPLE_COMPRESSOR_PARAMETERS_EMXAPI_H__
#define __SET_SIMPLE_COMPRESSOR_PARAMETERS_EMXAPI_H__

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

  extern emxArray_real_T *emxCreateND_real_T(int numDimensions, int *size);
  extern emxArray_real_T *emxCreateWrapperND_real_T(double *data, int
    numDimensions, int *size);
  extern emxArray_real_T *emxCreateWrapper_real_T(double *data, int rows, int
    cols);
  extern emxArray_real_T *emxCreate_real_T(int rows, int cols);
  extern void emxDestroyArray_real_T(emxArray_real_T *emxArray);

#ifdef __cplusplus

}
#endif
#endif

/* End of code generation (set_simple_compressor_parameters_emxAPI.h) */
