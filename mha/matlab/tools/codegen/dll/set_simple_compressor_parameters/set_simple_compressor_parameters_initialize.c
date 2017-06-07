/*
 * set_simple_compressor_parameters_initialize.c
 *
 * Code generation for function 'set_simple_compressor_parameters_initialize'
 *
 */

/* Include files */
#include "rt_nonfinite.h"
#include "set_simple_compressor_parameters.h"
#include "simple_compressor.h"
#include "set_simple_compressor_parameters_initialize.h"
#include "set_simple_compressor_parameters_emxutil.h"
#include "set_simple_compressor_parameters_data.h"

/* Function Definitions */
void set_simple_compressor_parameters_initialize(void)
{
  int i0;
  static const double c_attack_coeffs[4] = { 0.9, 0.8, 0.8, 0.8 };

  static const short b_gaintable_levels[5] = { -100, 0, 50, 90, 200 };

  static const signed char b_gaintable[20] = { 30, 40, 50, 30, 30, 40, 50, 30,
    10, 10, 20, 11, 0, 0, 0, 0, 0, 0, 0, 0 };

  rt_InitInfAndNaN(8U);
  emxInit_real_T(&gaintable, 2);
  emxInit_real_T(&gaintable_levels, 2);
  emxInit_real_T(&attack_coeffs, 2);
  emxInit_real_T(&release_coeffs, 2);
  i0 = release_coeffs->size[0] * release_coeffs->size[1];
  release_coeffs->size[0] = 1;
  release_coeffs->size[1] = 4;
  emxEnsureCapacity((emxArray__common *)release_coeffs, i0, (int)sizeof(double));
  for (i0 = 0; i0 < 4; i0++) {
    release_coeffs->data[i0] = 0.95;
  }

  i0 = attack_coeffs->size[0] * attack_coeffs->size[1];
  attack_coeffs->size[0] = 1;
  attack_coeffs->size[1] = 4;
  emxEnsureCapacity((emxArray__common *)attack_coeffs, i0, (int)sizeof(double));
  for (i0 = 0; i0 < 4; i0++) {
    attack_coeffs->data[i0] = c_attack_coeffs[i0];
  }

  i0 = gaintable_levels->size[0] * gaintable_levels->size[1];
  gaintable_levels->size[0] = 1;
  gaintable_levels->size[1] = 5;
  emxEnsureCapacity((emxArray__common *)gaintable_levels, i0, (int)sizeof(double));
  for (i0 = 0; i0 < 5; i0++) {
    gaintable_levels->data[i0] = b_gaintable_levels[i0];
  }

  i0 = gaintable->size[0] * gaintable->size[1];
  gaintable->size[0] = 4;
  gaintable->size[1] = 5;
  emxEnsureCapacity((emxArray__common *)gaintable, i0, (int)sizeof(double));
  for (i0 = 0; i0 < 20; i0++) {
    gaintable->data[i0] = b_gaintable[i0];
  }

  simple_compressor_init();
}

/* End of code generation (set_simple_compressor_parameters_initialize.c) */
