/*
 * simple_compressor.h
 *
 * Code generation for function 'simple_compressor'
 *
 */

#ifndef __SIMPLE_COMPRESSOR_H__
#define __SIMPLE_COMPRESSOR_H__

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

  extern void simple_compressor(const emxArray_real_T *input_signal,
    emxArray_real_T *output_signal);
  extern void simple_compressor_free(void);
  extern void simple_compressor_init(void);

#ifdef __cplusplus

}
#endif
#endif

/* End of code generation (simple_compressor.h) */
