/*
 * simple_compressor.c
 *
 * Code generation for function 'simple_compressor'
 *
 */

/* Include files */
#include "rt_nonfinite.h"
#include "set_simple_compressor_parameters.h"
#include "simple_compressor.h"
#include "set_simple_compressor_parameters_emxutil.h"
#include "rdivide.h"
#include "set_simple_compressor_parameters_data.h"

/* Variable Definitions */
static emxArray_real_T *state;
static boolean_T state_not_empty;

/* Function Declarations */
static double rt_powd_snf(double u0, double u1);

/* Function Definitions */
static double rt_powd_snf(double u0, double u1)
{
  double y;
  double d0;
  double d1;
  if (rtIsNaN(u0) || rtIsNaN(u1)) {
    y = rtNaN;
  } else {
    d0 = fabs(u0);
    d1 = fabs(u1);
    if (rtIsInf(u1)) {
      if (d0 == 1.0) {
        y = rtNaN;
      } else if (d0 > 1.0) {
        if (u1 > 0.0) {
          y = rtInf;
        } else {
          y = 0.0;
        }
      } else if (u1 > 0.0) {
        y = 0.0;
      } else {
        y = rtInf;
      }
    } else if (d1 == 0.0) {
      y = 1.0;
    } else if (d1 == 1.0) {
      if (u1 > 0.0) {
        y = u0;
      } else {
        y = 1.0 / u0;
      }
    } else if (u1 == 2.0) {
      y = u0 * u0;
    } else if ((u1 == 0.5) && (u0 >= 0.0)) {
      y = sqrt(u0);
    } else if ((u0 < 0.0) && (u1 > floor(u1))) {
      y = rtNaN;
    } else {
      y = pow(u0, u1);
    }
  }

  return y;
}

/*
 * function output_signal = simple_compressor(input_signal)
 */
void simple_compressor(const emxArray_real_T *input_signal, emxArray_real_T
  *output_signal)
{
  int ix;
  int ixstart;
  unsigned int sz[2];
  emxArray_real_T *y;
  emxArray_real_T *b_y;
  int iy;
  int mid_i;
  double s;
  emxArray_real_T *c_y;
  emxArray_real_T *d_y;
  emxArray_real_T *instant_levels;
  int channel;
  emxArray_real_T *varargin_2;
  int32_T exitg1;
  double xtmp;

  /* 'simple_compressor:10' nchannels_in = size(input_signal,2); */
  /* fragsize = size(input_signal,1); */
  /* 'simple_compressor:13' if isempty(state) */
  if (!state_not_empty) {
    /* 'simple_compressor:14' state = ones(1,nchannels_in)*65; */
    ix = state->size[0] * state->size[1];
    state->size[0] = 1;
    emxEnsureCapacity((emxArray__common *)state, ix, (int)sizeof(double));
    ixstart = input_signal->size[1];
    ix = state->size[0] * state->size[1];
    state->size[1] = ixstart;
    emxEnsureCapacity((emxArray__common *)state, ix, (int)sizeof(double));
    ixstart = input_signal->size[1];
    for (ix = 0; ix < ixstart; ix++) {
      state->data[ix] = 65.0;
    }

    state_not_empty = !(state->size[1] == 0);
  }

  /* 'simple_compressor:17' assert(size(gaintable,1) == nchannels_in); */
  /* 'simple_compressor:18' assert(size(attack_coeffs,2) == nchannels_in); */
  /* 'simple_compressor:19' assert(size(attack_coeffs,1) == 1); */
  /* 'simple_compressor:20' assert(isequal(size(release_coeffs), size(attack_coeffs))); */
  /* 'simple_compressor:21' assert(size(gaintable_levels,1) == 1); */
  /* 'simple_compressor:22' assert(size(gaintable_levels,2) == size(gaintable,2)); */
  /* 'simple_compressor:24' meansquares = mean(input_signal.^2); */
  for (ix = 0; ix < 2; ix++) {
    sz[ix] = (unsigned int)input_signal->size[ix];
  }

  emxInit_real_T(&y, 2);
  ix = y->size[0] * y->size[1];
  y->size[0] = (int)sz[0];
  y->size[1] = (int)sz[1];
  emxEnsureCapacity((emxArray__common *)y, ix, (int)sizeof(double));
  ix = (int)sz[0] * (int)sz[1];
  for (ixstart = 0; ixstart < ix; ixstart++) {
    y->data[ixstart] = input_signal->data[ixstart] * input_signal->data[ixstart];
  }

  for (ix = 0; ix < 2; ix++) {
    sz[ix] = (unsigned int)y->size[ix];
  }

  emxInit_real_T(&b_y, 2);
  ix = b_y->size[0] * b_y->size[1];
  b_y->size[0] = 1;
  b_y->size[1] = (int)sz[1];
  emxEnsureCapacity((emxArray__common *)b_y, ix, (int)sizeof(double));
  if ((y->size[0] == 0) || (y->size[1] == 0)) {
    ix = b_y->size[0] * b_y->size[1];
    b_y->size[0] = 1;
    emxEnsureCapacity((emxArray__common *)b_y, ix, (int)sizeof(double));
    ix = b_y->size[0] * b_y->size[1];
    b_y->size[1] = (int)sz[1];
    emxEnsureCapacity((emxArray__common *)b_y, ix, (int)sizeof(double));
    ixstart = (int)sz[1];
    for (ix = 0; ix < ixstart; ix++) {
      b_y->data[ix] = 0.0;
    }
  } else {
    ix = -1;
    iy = -1;
    for (mid_i = 1; mid_i <= y->size[1]; mid_i++) {
      ixstart = ix + 1;
      ix++;
      s = y->data[ixstart];
      for (ixstart = 2; ixstart <= y->size[0]; ixstart++) {
        ix++;
        s += y->data[ix];
      }

      iy++;
      b_y->data[iy] = s;
    }
  }

  emxInit_real_T(&c_y, 2);

  /* 'simple_compressor:25' instant_levels = 10*log10(meansquares / 4e-10 + 1e-10); */
  ix = c_y->size[0] * c_y->size[1];
  c_y->size[0] = 1;
  c_y->size[1] = b_y->size[1];
  emxEnsureCapacity((emxArray__common *)c_y, ix, (int)sizeof(double));
  ixstart = b_y->size[0] * b_y->size[1];
  for (ix = 0; ix < ixstart; ix++) {
    c_y->data[ix] = b_y->data[ix];
  }

  emxInit_real_T(&d_y, 2);
  b_rdivide(c_y, y->size[0], b_y);
  ix = d_y->size[0] * d_y->size[1];
  d_y->size[0] = 1;
  d_y->size[1] = b_y->size[1];
  emxEnsureCapacity((emxArray__common *)d_y, ix, (int)sizeof(double));
  ixstart = b_y->size[0] * b_y->size[1];
  emxFree_real_T(&c_y);
  emxFree_real_T(&y);
  for (ix = 0; ix < ixstart; ix++) {
    d_y->data[ix] = b_y->data[ix];
  }

  b_rdivide(d_y, 4.0E-10, b_y);
  ix = b_y->size[0] * b_y->size[1];
  b_y->size[0] = 1;
  emxEnsureCapacity((emxArray__common *)b_y, ix, (int)sizeof(double));
  ixstart = b_y->size[0];
  ix = b_y->size[1];
  ixstart *= ix;
  emxFree_real_T(&d_y);
  for (ix = 0; ix < ixstart; ix++) {
    b_y->data[ix] += 1.0E-10;
  }

  emxInit_real_T(&instant_levels, 2);
  ix = instant_levels->size[0] * instant_levels->size[1];
  instant_levels->size[0] = 1;
  instant_levels->size[1] = b_y->size[1];
  emxEnsureCapacity((emxArray__common *)instant_levels, ix, (int)sizeof(double));
  ixstart = b_y->size[0] * b_y->size[1];
  for (ix = 0; ix < ixstart; ix++) {
    instant_levels->data[ix] = b_y->data[ix];
  }

  for (ixstart = 0; ixstart < b_y->size[1]; ixstart++) {
    instant_levels->data[ixstart] = log10(instant_levels->data[ixstart]);
  }

  ix = instant_levels->size[0] * instant_levels->size[1];
  instant_levels->size[0] = 1;
  emxEnsureCapacity((emxArray__common *)instant_levels, ix, (int)sizeof(double));
  ixstart = instant_levels->size[0];
  ix = instant_levels->size[1];
  ixstart *= ix;
  for (ix = 0; ix < ixstart; ix++) {
    instant_levels->data[ix] *= 10.0;
  }

  /* 'simple_compressor:26' filtered_levels = state; */
  /* 'simple_compressor:28' output_signal = input_signal; */
  ix = output_signal->size[0] * output_signal->size[1];
  output_signal->size[0] = input_signal->size[0];
  output_signal->size[1] = input_signal->size[1];
  emxEnsureCapacity((emxArray__common *)output_signal, ix, (int)sizeof(double));
  ixstart = input_signal->size[0] * input_signal->size[1];
  for (ix = 0; ix < ixstart; ix++) {
    output_signal->data[ix] = input_signal->data[ix];
  }

  /* 'simple_compressor:29' for channel = 1:nchannels_in */
  channel = 0;
  emxInit_real_T(&varargin_2, 2);
  while (channel <= input_signal->size[1] - 1) {
    /* 'simple_compressor:30' coeff = release_coeffs(channel); */
    s = release_coeffs->data[channel];

    /* 'simple_compressor:31' if (instant_levels(channel) > filtered_levels(channel)) */
    if (instant_levels->data[channel] > state->data[channel]) {
      /* 'simple_compressor:32' coeff = attack_coeffs(channel); */
      s = attack_coeffs->data[channel];
    }

    /* 'simple_compressor:34' filtered_levels(channel) = filtered_levels(channel) * coeff + (1-coeff) * instant_levels(channel); */
    state->data[channel] = state->data[channel] * s + (1.0 - s) *
      instant_levels->data[channel];

    /* 'simple_compressor:35' gain = interp1(gaintable_levels, gaintable(channel,:), filtered_levels(channel)); */
    ixstart = gaintable->size[1];
    ix = varargin_2->size[0] * varargin_2->size[1];
    varargin_2->size[0] = 1;
    varargin_2->size[1] = ixstart;
    emxEnsureCapacity((emxArray__common *)varargin_2, ix, (int)sizeof(double));
    for (ix = 0; ix < ixstart; ix++) {
      varargin_2->data[varargin_2->size[0] * ix] = gaintable->data[channel +
        gaintable->size[0] * ix];
    }

    ix = b_y->size[0] * b_y->size[1];
    b_y->size[0] = 1;
    b_y->size[1] = gaintable_levels->size[1];
    emxEnsureCapacity((emxArray__common *)b_y, ix, (int)sizeof(double));
    ixstart = gaintable_levels->size[0] * gaintable_levels->size[1];
    for (ix = 0; ix < ixstart; ix++) {
      b_y->data[ix] = gaintable_levels->data[ix];
    }

    s = rtNaN;
    ixstart = 1;
    do {
      exitg1 = 0;
      if (ixstart <= gaintable_levels->size[1]) {
        if (rtIsNaN(gaintable_levels->data[ixstart - 1])) {
          exitg1 = 1;
        } else {
          ixstart++;
        }
      } else {
        if (gaintable_levels->data[1] < gaintable_levels->data[0]) {
          ix = gaintable_levels->size[1] >> 1;
          for (ixstart = 1; ixstart <= ix; ixstart++) {
            xtmp = b_y->data[ixstart - 1];
            b_y->data[ixstart - 1] = b_y->data[gaintable_levels->size[1] -
              ixstart];
            b_y->data[gaintable_levels->size[1] - ixstart] = xtmp;
          }

          ix = gaintable->size[1];
          iy = ix / 2;
          for (ixstart = 1; ixstart <= iy; ixstart++) {
            ix = gaintable->size[1];
            ix -= ixstart;
            xtmp = varargin_2->data[varargin_2->size[0] * (ixstart - 1)];
            varargin_2->data[varargin_2->size[0] * (ixstart - 1)] =
              varargin_2->data[varargin_2->size[0] * ix];
            varargin_2->data[varargin_2->size[0] * ix] = xtmp;
          }
        }

        if (rtIsNaN(state->data[channel]) || (state->data[channel] > b_y->
             data[b_y->size[1] - 1]) || (state->data[channel] < b_y->data[0])) {
        } else {
          ixstart = 1;
          ix = 2;
          iy = b_y->size[1];
          while (iy > ix) {
            mid_i = (ixstart >> 1) + (iy >> 1);
            if (((ixstart & 1) == 1) && ((iy & 1) == 1)) {
              mid_i++;
            }

            if (state->data[channel] >= b_y->data[mid_i - 1]) {
              ixstart = mid_i;
              ix = mid_i + 1;
            } else {
              iy = mid_i;
            }
          }

          s = (state->data[channel] - b_y->data[ixstart - 1]) / (b_y->
            data[ixstart] - b_y->data[ixstart - 1]);
          if (varargin_2->data[ixstart - 1] == varargin_2->data[ixstart]) {
            s = varargin_2->data[ixstart - 1];
          } else {
            s = (1.0 - s) * varargin_2->data[ixstart - 1] + s * varargin_2->
              data[ixstart];
          }
        }

        exitg1 = 1;
      }
    } while (exitg1 == 0);

    /* 'simple_compressor:36' output_signal(:,channel) = input_signal(:,channel) * 10.^(gain/20); */
    s = rt_powd_snf(10.0, s / 20.0);
    ixstart = input_signal->size[0] - 1;
    for (ix = 0; ix <= ixstart; ix++) {
      output_signal->data[ix + output_signal->size[0] * channel] =
        input_signal->data[ix + input_signal->size[0] * channel] * s;
    }

    channel++;
  }

  emxFree_real_T(&b_y);
  emxFree_real_T(&varargin_2);
  emxFree_real_T(&instant_levels);

  /* 'simple_compressor:39' state = filtered_levels; */
  state_not_empty = !(state->size[1] == 0);
}

void simple_compressor_free(void)
{
  emxFree_real_T(&state);
}

void simple_compressor_init(void)
{
  emxInit_real_T(&state, 2);
  state_not_empty = false;
}

/* End of code generation (simple_compressor.c) */
