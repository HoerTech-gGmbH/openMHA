/*
 * _coder_set_simple_compressor_parameters_api.h
 *
 * Code generation for function 'set_simple_compressor_parameters'
 *
 */

#ifndef ___CODER_SET_SIMPLE_COMPRESSOR_PARAMETERS_API_H__
#define ___CODER_SET_SIMPLE_COMPRESSOR_PARAMETERS_API_H__
/* Include files */
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "tmwtypes.h"
#include "mex.h"
#include "emlrt.h"

/* Type Definitions */
#ifndef struct_emxArray_real_T
#define struct_emxArray_real_T
struct emxArray_real_T
{
    double *data;
    int *size;
    int allocatedSize;
    int numDimensions;
    boolean_T canFreeData;
};
#endif /*struct_emxArray_real_T*/
#ifndef typedef_emxArray_real_T
#define typedef_emxArray_real_T
typedef struct emxArray_real_T emxArray_real_T;
#endif /*typedef_emxArray_real_T*/

/* Function Declarations */
extern void set_simple_compressor_parameters_initialize(emlrtContext *aContext);
extern void set_simple_compressor_parameters_terminate(void);
extern void set_simple_compressor_parameters_atexit(void);
extern void set_simple_compressor_parameters_api(const mxArray *prhs[7]);
extern void set_simple_compressor_parameters(unsigned int channels, double sampling_rate, unsigned int block_size, emxArray_real_T *gaintable_levels_, emxArray_real_T *gaintable_, emxArray_real_T *attack_times, emxArray_real_T *release_times);
extern void simple_compressor_api(const mxArray *prhs[1], const mxArray *plhs[1]);
extern void simple_compressor(emxArray_real_T *input_signal, emxArray_real_T *output_signal);
extern void set_simple_compressor_parameters_xil_terminate(void);

#endif
/* End of code generation (_coder_set_simple_compressor_parameters_api.h) */
