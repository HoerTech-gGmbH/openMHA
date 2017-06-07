/*
 * set_simple_compressor_parameters_terminate.c
 *
 * Code generation for function 'set_simple_compressor_parameters_terminate'
 *
 */

/* Include files */
#include "rt_nonfinite.h"
#include "set_simple_compressor_parameters.h"
#include "simple_compressor.h"
#include "set_simple_compressor_parameters_terminate.h"
#include "set_simple_compressor_parameters_emxutil.h"
#include "set_simple_compressor_parameters_data.h"

/* Function Definitions */
void set_simple_compressor_parameters_terminate(void)
{
  emxFree_real_T(&gaintable);
  emxFree_real_T(&gaintable_levels);
  emxFree_real_T(&attack_coeffs);
  emxFree_real_T(&release_coeffs);
  simple_compressor_free();
}

/* End of code generation (set_simple_compressor_parameters_terminate.c) */
