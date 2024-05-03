/* Copyright (c) 2023 Nils L. Westhausen */
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

#ifndef RNN_DATA_H
#define RNN_DATA_H

#include "rnn.h"

// new model definition
struct RNNModel {
  int input_scale_size;
  const ScalerLayer *scaler;

  int dense_projection_size;
  const DenseLayer *dense_projection;

  int dense_map_1_size;
  const DenseLayer *dense_map_1;

  int dconv_5_size;
  const DConvLayer *dconv_5;

  int dense_deconv_5_size;
  const DenseLayer *dense_deconv_5;

  int dconv_3_size;
  const DConvLayer *dconv_3;

  int dense_deconv_3_size;
  const DenseLayer *dense_deconv_3;

  int dconv_skip_1_size;
  const DConvLayer1x1 *dconv_skip_1;

  int dense_tac1_1_size;
  const DenseLayer *dense_tac1_1;

  int dense_tac1_2_size;
  const DenseLayer *dense_tac1_2;

  int dense_tac1_3_size;
  const DenseLayer *dense_tac1_3;

  int gru_2_1_size;
  const GRULayer *gru_2_1;

  int gru_2_2_size;
  const GRULayer *gru_2_2;

  int dconv_skip_2_size;
  const DConvLayer1x1 *dconv_skip_2;

  int dense_tac2_1_size;
  const DenseLayer *dense_tac2_1;

  int dense_tac2_2_size;
  const DenseLayer *dense_tac2_2;

  int dense_tac2_3_size;
  const DenseLayer *dense_tac2_3;

  int dense_map_2_size;
  const DenseLayer *dense_map_2;

  int dense_filt_b_size;
  const DenseLayer *dense_filt_b;

  int b_scale_size;
  const ScalerLayer *scaler_b;

  int dense_filt_t_size;
  const DenseLayer *dense_filt_t;

  int t_scale_size;
  const ScalerLayer *scaler_t;


};



// state definition
struct RNNState {
  const RNNModel *model;
  float *gru_2_1_state;
  float *gru_2_2_state;
  float *dconv_5_buffer;
  float *dconv_3_buffer;
  counter dconv_5_idx_start;
  counter dconv_3_idx_start;
  counter dconv_5_idx_write;
  counter dconv_3_idx_write;
};


#endif
