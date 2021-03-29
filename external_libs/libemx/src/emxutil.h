/* @generated
 * emxutil.h
 *
 * Code generation for function 'emxutil'
 *
 */

#ifndef EMXUTIL_H
#define EMXUTIL_H

/* Include files */
#include "types.h"
#include "rtwtypes.h"
#include <stddef.h>
#include <stdlib.h>
#ifdef __cplusplus

extern "C" {

#endif

  /* Function Declarations */
  extern void emxCopyStruct_user_config_t(user_config_t *dst, const
    user_config_t *src);
  extern void emxCopy_char_T(emxArray_char_T **dst, emxArray_char_T * const *src);
  extern void emxCopy_real_T(emxArray_real_T **dst, emxArray_real_T * const *src);
  extern void emxEnsureCapacity_char_T(emxArray_char_T *emxArray, int oldNumel);
  extern void emxEnsureCapacity_creal_T(emxArray_creal_T *emxArray, int oldNumel);
  extern void emxEnsureCapacity_real_T(emxArray_real_T *emxArray, int oldNumel);
  extern void emxEnsureCapacity_user_config_t(emxArray_user_config_t *emxArray,
    int oldNumel);
  extern void emxExpand_user_config_t(emxArray_user_config_t *emxArray, int
    fromIndex, int toIndex);
  extern void emxFreeStruct_user_config_t(user_config_t *pStruct);
  extern void emxFree_char_T(emxArray_char_T **pEmxArray);
  extern void emxFree_creal_T(emxArray_creal_T **pEmxArray);
  extern void emxFree_real_T(emxArray_real_T **pEmxArray);
  extern void emxFree_user_config_t(emxArray_user_config_t **pEmxArray);
  extern void emxInitStruct_user_config_t(user_config_t *pStruct);
  extern void emxInit_char_T(emxArray_char_T **pEmxArray, int numDimensions);
  extern void emxInit_creal_T(emxArray_creal_T **pEmxArray, int numDimensions);
  extern void emxInit_real_T(emxArray_real_T **pEmxArray, int numDimensions);
  extern void emxInit_user_config_t(emxArray_user_config_t **pEmxArray, int
    numDimensions);
  extern void emxTrim_user_config_t(emxArray_user_config_t *emxArray, int
    fromIndex, int toIndex);

#ifdef __cplusplus

}
#endif
#endif

/* End of code generation (emxutil.h) */
