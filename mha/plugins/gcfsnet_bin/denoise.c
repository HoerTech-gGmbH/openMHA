/*
  Copyright (c) 2017 Mozilla 
  Copyright (c) 2018 Gregor Richards
  Copyright (c) 2020 Nils L. Westhausen
  Copyright (c) 2023 Nils L. Westhausen (Heavily modified)
*/
/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/



#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "rnnoise.h"
#include "rnn.h"
#include "rnn_data.h"


#define FEAT_LEN 520
#define FFT_SIZE 128
#define FFT_HALF 65
#define NUM_CHAN 4
#define LEN_FILT_T 1

#define SQUARE(x) ((x)*(x))
#define MAX(x,y) ((x) > (y) ? (x) : (y))

#define NB_FEATURES FEAT_LEN


#ifndef TRAINING
#define TRAINING 0
#endif


/* The built-in model, used if no file is given as input */
extern const struct RNNModel rnnoise_model_orig;

/* Struct containing
  - rnn: the model with all weights,
  - features: pointer to the features of the current frame
  - filtering_buffer_(r/i): pointer filtering ring buffers
  - filter_(b/t): pointer the filter coefficients
  - buffer_start_idx: index of the oldest frame in the filtering buffer
  - buffer_write_idx: index of the newest frame in the filtering buffer
  */
struct DenoiseState {
  RNNState rnn;
  float *features; //[FEAT_LEN];
  float *filtering_buffer_r; //[FFT_HALF * LEN_FILT_T];
  float *filtering_buffer_i; //[FFT_HALF * LEN_FILT_T];
  float *filter_b; //[FFT_HALF * 2 * 2];
  float *filter_t; //[FFT_HALF * 2 * LEN_FILT_T];

  int buffer_start_idx;
  int buffer_write_idx;
};


int rnnoise_get_size() {
  return sizeof(DenoiseState);
}

/*
Function to initialize everything in the DenoiseState struct
*/
int rnnoise_init(DenoiseState *st) {
  memset(st, 0, sizeof(*st));
  st->rnn.model = &rnnoise_model_orig;
  st->rnn.gru_2_1_state = calloc(sizeof(float), st->rnn.model->gru_2_1_size * NUM_GROUPS);
  st->rnn.gru_2_2_state = calloc(sizeof(float), st->rnn.model->gru_2_2_size * NUM_GROUPS);
  st->rnn.dconv_5_buffer = calloc(sizeof(float), st->rnn.model->dconv_5_size * 5 * NUM_GROUPS);
  st->rnn.dconv_3_buffer = calloc(sizeof(float), st->rnn.model->dconv_3_size * 3 * NUM_GROUPS);
  st->rnn.dconv_3_idx_write = 2;
  st->rnn.dconv_3_idx_start = 0;
  st->rnn.dconv_5_idx_write = 4;
  st->rnn.dconv_5_idx_start = 0;
  st->buffer_start_idx = 0;
  st->buffer_write_idx = LEN_FILT_T - 1;
  st->filtering_buffer_i = calloc(sizeof(float), FFT_HALF * LEN_FILT_T);
  st->filtering_buffer_r = calloc(sizeof(float), FFT_HALF * LEN_FILT_T);
  st->filter_b = calloc(sizeof(float), FFT_HALF * 2 * 2);
  st->filter_t = calloc(sizeof(float), FFT_HALF * 2 * LEN_FILT_T);
  st->features = calloc(sizeof(float), FEAT_LEN);
  return 0;
}

/*
Function to create a DenoiseState struct
*/
DenoiseState *rnnoise_create() {
  DenoiseState *st;
  st = malloc(rnnoise_get_size());
  rnnoise_init(st);
  return st;
}

/*
Function to destroy a DenoiseState struct
*/
void rnnoise_destroy(DenoiseState *st) {
  free(st->rnn.gru_2_1_state);
  free(st->rnn.gru_2_2_state);
  free(st->rnn.dconv_5_buffer);
  free(st->rnn.dconv_3_buffer);
  free(st->filtering_buffer_i);
  free(st->filtering_buffer_r);
  free(st->filter_b);
  free(st->filter_t);
  free(st->features);
  free(st);
}

/*
Function to apply the spatial filter
*/
static void apply_filter_b(DenoiseState *st, const float *real_part, const float *imag_part) 
{
  for (int i=0;i<FFT_HALF;i++)
  {
    st->filtering_buffer_r[st->buffer_write_idx * FFT_HALF + i] = 0.f;
    st->filtering_buffer_i[st->buffer_write_idx * FFT_HALF + i] = 0.f;
    for (int j=0;j<2;j++)
    { 
    st->filtering_buffer_r[st->buffer_write_idx * FFT_HALF + i] += st->filter_b[(j * 2) * FFT_HALF + i] * real_part[j * FFT_HALF + i];
    st->filtering_buffer_r[st->buffer_write_idx * FFT_HALF + i] -= st->filter_b[(j * 2 + 1) * FFT_HALF + i] * imag_part[j * FFT_HALF + i];
    st->filtering_buffer_i[st->buffer_write_idx * FFT_HALF + i] += st->filter_b[(j * 2) * FFT_HALF + i] * imag_part[j * FFT_HALF + i];
    st->filtering_buffer_i[st->buffer_write_idx * FFT_HALF + i] += st->filter_b[(j * 2 + 1) * FFT_HALF + i] * real_part[j * FFT_HALF + i];
    }
    }
  
  st->buffer_write_idx = (st->buffer_write_idx + 1) % LEN_FILT_T;

}

/*
Function to apply a possible multi-frame temporal filter
Currently only a single frame is used
*/
static void apply_filter_t(DenoiseState *st, float *real_out, float *imag_out) 
{
  for (int i=0;i<FFT_HALF;i++)
  {
    real_out[i] = 0.f;
    imag_out[i] = 0.f;

    for (int j=0;j<LEN_FILT_T;j++)
    {
      real_out[i] += st->filter_t[(j * 2) * FFT_HALF + i] * st->filtering_buffer_r[((st->buffer_start_idx + j) % LEN_FILT_T) * FFT_HALF + i];
      real_out[i] -= st->filter_t[(j * 2 + 1) * FFT_HALF + i] * st->filtering_buffer_i[((st->buffer_start_idx + j) % LEN_FILT_T) * FFT_HALF + i];
      imag_out[i] += st->filter_t[(j * 2 + 1) * FFT_HALF + i] * st->filtering_buffer_r[((st->buffer_start_idx + j) % LEN_FILT_T) * FFT_HALF + i];
      imag_out[i] += st->filter_t[(j * 2) * FFT_HALF + i] * st->filtering_buffer_i[((st->buffer_start_idx + j) % LEN_FILT_T) * FFT_HALF + i];
    }
  }
  
  st->buffer_start_idx = (st->buffer_start_idx + 1) % LEN_FILT_T;
  

}

/*
Function to compute the features of the current frame
*/
static void compute_frame_features(DenoiseState *st, const float *real_part, const float *imag_part) 
{
 for (int c=0;c<NUM_CHAN;c++) 
  {
    for (int i=0;i<FFT_HALF;i++)
    {
    float imag;
    float real;
    real = real_part[c * FFT_HALF + i] ;
    imag = imag_part[c * FFT_HALF + i] ;
    st->features[(2*c)*FFT_HALF + i] = real;
    st->features[(2*c + 1)*FFT_HALF + i] = imag;
    }
  }
}

/*
Function to process one frame
*/
void rnnoise_process_frame(DenoiseState *st, float *real_output, float *imag_output, const float *real_input, const float *imag_input)
{
  // computing features
  compute_frame_features(st, real_input, imag_input);
  // running the rnn
  compute_rnn(&st->rnn, st->filter_b, st->filter_t, st->features);
  // applying the filters
  apply_filter_b(st, real_input, imag_input);
  apply_filter_t(st, real_output, imag_output);
}
