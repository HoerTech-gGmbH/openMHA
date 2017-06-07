/*
 * set_simple_compressor_parameters.h
 *
 * Code generation for function 'set_simple_compressor_parameters'
 *
 */

#ifndef __SET_SIMPLE_COMPRESSOR_PARAMETERS_H__
#define __SET_SIMPLE_COMPRESSOR_PARAMETERS_H__

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

  extern void set_simple_compressor_parameters(unsigned int channels, double
    sampling_rate, unsigned int block_size, const emxArray_real_T
    *gaintable_levels_, const emxArray_real_T *gaintable_, const emxArray_real_T
    *attack_times, const emxArray_real_T *release_times);

#ifdef __cplusplus

}
#endif
#endif

/* End of code generation (set_simple_compressor_parameters.h) */
