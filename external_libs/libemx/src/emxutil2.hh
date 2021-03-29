//@generated
#ifndef EMXUTIL2_H
#define EMXUTIL2_H

/* Include files */
#include "emxAPI.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

  /* Function Declarations */
  /** Initialize a new 1d emxArray_char_T struct
   * @param sz Array size
   * @return Pointer to the struct
   */
  extern emxArray_char_T *argInit_1xUnbounded_char_T(int sz=0);
  /** Initialize a new 1d emxArray_real_T struct
   * @param sz Array size
   * @return Pointer to the struct
   */
  extern emxArray_real_T *argInit_Unboundedx1_real_T(int sz=0);
  /** Initialize a char
   * @return null-byte
   */
  extern char argInit_char_T(void);
  /** Initialize a complex double to zero
   * @return (0.0,0.0)
   */
  extern creal_T argInit_creal_T(void);
  /** Initialize a real double to zero
   * @return 0.0
   */
  extern double argInit_real_T(void);
  /** Initialize a signal_dimensions_t struct with default values
   * @return Default initialized signal_dimension
   */
  extern signal_dimensions_t argInit_signal_dimensions_t(void);
  /** Initialize a uint32_t
   * @return 0U
   */
  extern unsigned int argInit_uint32_T(void);
  /** Initialize an empty user_config_t struct. The 'name' and 'value' fields are
   * initialized with a size of zero
   * @return Default user_config_t
   */
  extern user_config_t argInit_user_config_t(void);
  /** Initialize emxArray struct of Unbounded x sz user_config_t structs
   * @param Size of the array
   * @return Pointer to the array
   */
  extern emxArray_user_config_t *c_argInit_Unboundedx1_user_conf(int sz=0);
  /** Initialize a sx x sy matrix of complex doubles
   * @param Number of rows
   * @param Number of columns
   */
  extern emxArray_creal_T *c_argInit_UnboundedxUnbounded_c(void);
  /** Initialize a sx x sy matrix of doubles
   * @param Number of rows
   * @param Number of columns
   */
  extern emxArray_real_T *c_argInit_UnboundedxUnbounded_r(int sx=0, int sy=0);
#ifdef __cplusplus
}
#endif
#endif
