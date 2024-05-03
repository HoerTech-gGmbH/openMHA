/* Copyright (c) 2017 Jean-Marc Valin 
                 2020 Nils L. Westhausen */
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

#ifndef RNN_H_
#define RNN_H_

#include "rnnoise.h"
#include <stdint.h>

#define WEIGHTS_SCALE (1.f/128)
#define WEIGHTS_SCALE_BIAS (1.f/32768)


#define MAX_NEURONS 32

#define ACTIVATION_TANH    0
#define ACTIVATION_SIGMOID 1
#define ACTIVATION_RELU    2
#define ACTIVATION_LINEAR  3

#define NUM_GROUPS 8

typedef int8_t rnn_weight;
typedef int16_t rnn_bias;
typedef int16_t counter;

typedef struct {
  const rnn_bias *bias;
  const rnn_weight *input_weights;
  counter nb_inputs;
  counter nb_neurons;
  counter activation;
} DenseLayer;

typedef struct {
  const rnn_bias *bias;
  const rnn_weight *input_weights;
  counter nb_inputs;
  counter nb_neurons;
  counter activation;
} DConvLayer1x1;

typedef struct {
  const float *bias;
  const float *input_weights;
  counter nb_inputs;
  counter nb_neurons;
  counter activation;
} ScalerLayer;

typedef struct {
  const rnn_bias *bias;
  const rnn_weight *input_weights;
  counter nb_lenfilt;
  counter nb_neurons;
  counter activation;
} DConvLayer;

typedef struct {
  const rnn_bias *bias;
  const rnn_weight *input_weights;
  const rnn_weight *recurrent_weights;
  counter nb_inputs;
  counter nb_neurons;
  counter activation;
} GRULayer;

typedef struct RNNState RNNState;

void compute_dense(const DenseLayer *layer, float *output, const float *input);
void compute_dense_grouped(const DenseLayer *layer, float *output, const float *input);

void compute_gru_grouped(const GRULayer *gru, float *state, const float *input);

void add_skip(int input_size, float *output, const float *input, const float *input_skip);

void compute_rnn(RNNState *rnn, float *filter_b, float *filter_t, const float *input);

void compute_dconv_1x1_grouped(const DConvLayer1x1 *dclayer, float *output, const float *input, const float *input_skip);

void compute_scale(const ScalerLayer *dclayer, float *output, const float *input);

void compute_dconv_1xX_grouped(const DConvLayer *dclayer, counter *idx_start, counter *idx_write, float *buffer, float *output, const float *input);



#endif /* _MLP_H_ */
