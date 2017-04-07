/*
 * set_simple_compressor_parameters.c
 *
 * Code generation for function 'set_simple_compressor_parameters'
 *
 */

/* Include files */
#include "rt_nonfinite.h"
#include "set_simple_compressor_parameters.h"
#include "simple_compressor.h"
#include "set_simple_compressor_parameters_emxutil.h"
#include "rdivide.h"
#include "set_simple_compressor_parameters_data.h"

/* Function Definitions */

/*
 * function set_simple_compressor_parameters(channels, sampling_rate, block_size, gaintable_levels_, gaintable_, attack_times, release_times)
 */
void set_simple_compressor_parameters(unsigned int channels, double
  sampling_rate, unsigned int block_size, const emxArray_real_T
  *gaintable_levels_, const emxArray_real_T *gaintable_, const emxArray_real_T
  *attack_times, const emxArray_real_T *release_times)
{
  int i1;
  int loop_ub;
  emxArray_real_T *b_attack_times;
  double block_rate;
  emxArray_real_T *b_release_times;
  (void)channels;

  /* 'set_simple_compressor_parameters:8' gaintable = gaintable_; */
  i1 = gaintable->size[0] * gaintable->size[1];
  gaintable->size[0] = gaintable_->size[0];
  gaintable->size[1] = gaintable_->size[1];
  emxEnsureCapacity((emxArray__common *)gaintable, i1, (int)sizeof(double));
  loop_ub = gaintable_->size[0] * gaintable_->size[1];
  for (i1 = 0; i1 < loop_ub; i1++) {
    gaintable->data[i1] = gaintable_->data[i1];
  }

  /* 'set_simple_compressor_parameters:9' gaintable_levels = gaintable_levels_; */
  i1 = gaintable_levels->size[0] * gaintable_levels->size[1];
  gaintable_levels->size[0] = 1;
  gaintable_levels->size[1] = gaintable_levels_->size[1];
  emxEnsureCapacity((emxArray__common *)gaintable_levels, i1, (int)sizeof(double));
  loop_ub = gaintable_levels_->size[0] * gaintable_levels_->size[1];
  for (i1 = 0; i1 < loop_ub; i1++) {
    gaintable_levels->data[i1] = gaintable_levels_->data[i1];
  }

  emxInit_real_T(&b_attack_times, 2);

  /* 'set_simple_compressor_parameters:11' assert(sampling_rate > 0); */
  /* 'set_simple_compressor_parameters:12' assert(block_size > 0); */
  /* 'set_simple_compressor_parameters:13' assert(size(gaintable,1) == channels); */
  /* 'set_simple_compressor_parameters:14' assert(size(attack_times,2) == channels); */
  /* 'set_simple_compressor_parameters:15' assert(size(attack_times,1) == 1); */
  /* 'set_simple_compressor_parameters:16' assert(isequal(size(release_times), size(attack_times))); */
  /* 'set_simple_compressor_parameters:17' assert(size(gaintable_levels,1) == 1); */
  /* 'set_simple_compressor_parameters:18' assert(size(gaintable_levels,2) == size(gaintable,2)); */
  /* 'set_simple_compressor_parameters:20' block_rate = sampling_rate / double(block_size); */
  block_rate = sampling_rate / (double)block_size;

  /* 'set_simple_compressor_parameters:21' attack_coeffs = exp(-1./(attack_times * block_rate)); */
  i1 = b_attack_times->size[0] * b_attack_times->size[1];
  b_attack_times->size[0] = 1;
  b_attack_times->size[1] = attack_times->size[1];
  emxEnsureCapacity((emxArray__common *)b_attack_times, i1, (int)sizeof(double));
  loop_ub = attack_times->size[0] * attack_times->size[1];
  for (i1 = 0; i1 < loop_ub; i1++) {
    b_attack_times->data[i1] = attack_times->data[i1] * block_rate;
  }

  rdivide(b_attack_times, attack_coeffs);
  i1 = attack_coeffs->size[1];
  loop_ub = 0;
  emxFree_real_T(&b_attack_times);
  while (loop_ub <= i1 - 1) {
    attack_coeffs->data[loop_ub] = exp(attack_coeffs->data[loop_ub]);
    loop_ub++;
  }

  emxInit_real_T(&b_release_times, 2);

  /* 'set_simple_compressor_parameters:22' release_coeffs = exp(-1./(release_times * block_rate)); */
  i1 = b_release_times->size[0] * b_release_times->size[1];
  b_release_times->size[0] = 1;
  b_release_times->size[1] = release_times->size[1];
  emxEnsureCapacity((emxArray__common *)b_release_times, i1, (int)sizeof(double));
  loop_ub = release_times->size[0] * release_times->size[1];
  for (i1 = 0; i1 < loop_ub; i1++) {
    b_release_times->data[i1] = release_times->data[i1] * block_rate;
  }

  rdivide(b_release_times, release_coeffs);
  i1 = release_coeffs->size[1];
  loop_ub = 0;
  emxFree_real_T(&b_release_times);
  while (loop_ub <= i1 - 1) {
    release_coeffs->data[loop_ub] = exp(release_coeffs->data[loop_ub]);
    loop_ub++;
  }
}

/* End of code generation (set_simple_compressor_parameters.c) */
