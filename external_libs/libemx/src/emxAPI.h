/* @generated
 * emxAPI.h
 *
 * Code generation for function 'emxAPI'
 *
 */

#ifndef EMXAPI_H
#define EMXAPI_H

/* Include files */
#include "types.h"
#include "rtwtypes.h"
#include <stddef.h>
#include <stdlib.h>
#ifdef __cplusplus

extern "C" {

#endif

  /* Function Declarations */
  extern emxArray_char_T *emxCreateND_char_T(int numDimensions, const int *size);
  extern emxArray_real_T *emxCreateND_real_T(int numDimensions, const int *size);
  extern emxArray_user_config_t *emxCreateND_user_config_t(int numDimensions,
    const int *size);
  extern emxArray_char_T *emxCreateWrapperND_char_T(char *data, int
    numDimensions, const int *size);
  extern emxArray_real_T *emxCreateWrapperND_real_T(double *data, int
    numDimensions, const int *size);
  extern emxArray_user_config_t *emxCreateWrapperND_user_config_t(user_config_t *
    data, int numDimensions, const int *size);
  extern emxArray_char_T *emxCreateWrapper_char_T(char *data, int rows, int cols);
  extern emxArray_real_T *emxCreateWrapper_real_T(double *data, int rows, int
    cols);
  extern emxArray_user_config_t *emxCreateWrapper_user_config_t(user_config_t
    *data, int rows, int cols);
  extern emxArray_char_T *emxCreate_char_T(int rows, int cols);
  extern emxArray_real_T *emxCreate_real_T(int rows, int cols);
  extern emxArray_user_config_t *emxCreate_user_config_t(int rows, int cols);
  extern void emxDestroyArray_char_T(emxArray_char_T *emxArray);
  extern void emxDestroyArray_real_T(emxArray_real_T *emxArray);
  extern void emxDestroyArray_user_config_t(emxArray_user_config_t *emxArray);
  extern void emxInitArray_real_T(emxArray_real_T **pEmxArray, int numDimensions);

#ifdef __cplusplus

}
#endif
#endif

/* End of code generation (emxAPI.h) */
