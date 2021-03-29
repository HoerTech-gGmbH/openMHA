/* @generated
 * types.h
 *
 * Code generation for function 'init'
 *
 */

#ifndef TYPES_H
#define TYPES_H

/* Include files */
#include "rtwtypes.h"

/* Type Definitions */
#ifndef typedef_signal_dimensions_t
#define typedef_signal_dimensions_t

typedef struct {
  unsigned int channels;
  char domain;
  unsigned int fragsize;
  unsigned int wndlen;
  unsigned int fftlen;
  double srate;
} signal_dimensions_t;

#endif                                 /*typedef_signal_dimensions_t*/

#ifndef struct_emxArray_char_T
#define struct_emxArray_char_T

struct emxArray_char_T
{
  char *data;
  int *size;
  int allocatedSize;
  int numDimensions;
  bool canFreeData;
};

#endif                                 /*struct_emxArray_char_T*/

#ifndef typedef_emxArray_char_T
#define typedef_emxArray_char_T

typedef struct emxArray_char_T emxArray_char_T;

#endif                                 /*typedef_emxArray_char_T*/

#ifndef struct_emxArray_real_T
#define struct_emxArray_real_T

struct emxArray_real_T
{
  double *data;
  int *size;
  int allocatedSize;
  int numDimensions;
  bool canFreeData;
};

#endif                                 /*struct_emxArray_real_T*/

#ifndef typedef_emxArray_real_T
#define typedef_emxArray_real_T

typedef struct emxArray_real_T emxArray_real_T;

#endif                                 /*typedef_emxArray_real_T*/

#ifndef typedef_user_config_t
#define typedef_user_config_t

typedef struct {
  emxArray_char_T *name;
  emxArray_real_T *value;
} user_config_t;

#endif                                 /*typedef_user_config_t*/

#ifndef typedef_emxArray_user_config_t
#define typedef_emxArray_user_config_t

typedef struct {
  user_config_t *data;
  int *size;
  int allocatedSize;
  int numDimensions;
  bool canFreeData;
} emxArray_user_config_t;

#endif                                 /*typedef_emxArray_user_config_t*/

#ifndef typedef_emxArray_creal_T
#define typedef_emxArray_creal_T

typedef struct {
  creal_T *data;
  int *size;
  int allocatedSize;
  int numDimensions;
  bool canFreeData;
} emxArray_creal_T;

#endif                                 /*typedef_emxArray_creal_T*/
#endif

/* End of code generation (types.h) */
