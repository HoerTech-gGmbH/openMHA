/* Copyright (c) 2008-2011 Octasic Inc.
                 2012-2017 Jean-Marc Valin 
                 2020-2024 Nils L. Westhausen*/
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


#include <math.h>
#include "rnn.h"
#include "rnn_data.h"
#include <stdio.h>


static inline float tansig_approx(float x)
{
    return tanhf(x);
}

static inline float sigmoid_approx(float x)
{
   return .5 + .5*tansig_approx(.5*x);
}

static inline float relu(float x)
{
   return x < 0 ? 0 : x;
}

void compute_dense(const DenseLayer *layer, float *output, const float *input)
{

   int N, M;

   M = layer->nb_inputs;
   N = layer->nb_neurons;
   for (int i=0;i<N;i++)
   {
      /* Compute update gate. */
      float sum = .0f;
      float bias = (float) WEIGHTS_SCALE_BIAS * layer->bias[i];

      for (int j=0;j<M;j++)
         sum += layer->input_weights[i*M + j]*input[j]; // * (float) WEIGHTS_SCALE;
      sum *= (float) WEIGHTS_SCALE;
      sum += bias;
      output[i] = sum;
   }
   if (layer->activation == ACTIVATION_SIGMOID)
   {
      for (int i=0;i<N;i++)
         output[i] = sigmoid_approx(output[i]);
   }
   else if (layer->activation == ACTIVATION_TANH)
   {
      for (int i=0;i<N;i++)
         output[i] = tansig_approx(output[i]);
   }
   else if (layer->activation == ACTIVATION_RELU)
   {
      for (int i=0;i<N;i++)
         output[i] = relu(output[i]);
   }
   else if (layer->activation == ACTIVATION_LINEAR)
   {
      ;
   }
}

void compute_dense_grouped(const DenseLayer *layer, float *output, const float *input)
{
   int N, M, G;
   M = layer->nb_inputs;
   N = layer->nb_neurons;
   G = NUM_GROUPS;

   for (int g=0;g<G;g++)
   {
   
   for (int i=0;i<N;i++)
   {
      /* Compute update gate. */
      float sum = .0f;
      float bias = (float) WEIGHTS_SCALE_BIAS * layer->bias[i];

      for (int j=0;j<M;j++)
         sum += layer->input_weights[i*M + j]*input[M*g + j]; // * (float) WEIGHTS_SCALE;
      sum *= (float) WEIGHTS_SCALE;
      sum += bias;
      output[N*g + i] = sum;
   }
   if (layer->activation == ACTIVATION_SIGMOID)
   {

      for (int i=0;i<N;i++)
         output[N*g + i] = sigmoid_approx(output[N*g + i]);
   }
   else if (layer->activation == ACTIVATION_TANH)
   {

      for (int i=0;i<N;i++)
         output[N*g + i] = tansig_approx(output[N*g + i]);
   }
   else if (layer->activation == ACTIVATION_RELU)
   {

      for (int i=0;i<N;i++)
         output[N*g + i] = relu(output[N*g + i]);
   }
   else if (layer->activation == ACTIVATION_LINEAR) {
      ;
   
   }
   }
}


void compute_dconv_1x1_grouped(const DConvLayer1x1 *dclayer, float *output, const float *input, const float *input_skip)
{

   int N, G;
   G = NUM_GROUPS;
   N = dclayer->nb_neurons;

   for (int g=0;g<G;g++)
   {

   for (int i=0;i<N;i++)
   {
         output[g*N+i] = dclayer->bias[i] * (float) WEIGHTS_SCALE_BIAS;
         output[g*N+i] += input_skip[g*N+i] * dclayer->input_weights[i] * (float) WEIGHTS_SCALE;
   }

   if (dclayer->activation == ACTIVATION_SIGMOID)
   {

      for (int i=0;i<N;i++)
         output[g*N+i] = sigmoid_approx(output[g*N+i]);
   } 
   else if (dclayer->activation == ACTIVATION_TANH)
   {

      for (int i=0;i<N;i++)
         output[g*N+i] = tansig_approx(output[g*N+i]);
   } 
   else if (dclayer->activation == ACTIVATION_RELU)
   {

      for (int i=0;i<N;i++)
         output[g*N+i] = relu(output[g*N+i]);
   } 
   else if (dclayer->activation == ACTIVATION_LINEAR)
   {
      ;
   }

   for (int i=0;i<N;i++)
         output[g*N+i] += input[g*N+i];
   }
   
}

void add_skip(int input_size, float *output, const float *input, const float *input_skip)
{
   for (int i=0;i<input_size;i++)
         output[i] = input[i] + input_skip[i];
}


void compute_scale(const ScalerLayer *dclayer, float *output, const float *input)
{
   int N;
   N = dclayer->nb_inputs;

   for (int i=0;i<N;i++)
   {
         output[i] = dclayer->bias[0]; //* (float) WEIGHTS_SCALE_BIAS;
         output[i] += input[i] * dclayer->input_weights[0]; //* (float) WEIGHTS_SCALE_BIAS);
   }
}


void compute_dconv_1xX_grouped(const DConvLayer *dclayer, counter *idx_start, counter *idx_write, float *buffer, float *output, const float *input)
{

   int N, L, G;
   L = dclayer->nb_lenfilt;
   N = dclayer->nb_neurons;
   G = NUM_GROUPS;
   float sum;

   for (int g=0;g<G;g++)
   {
  
   for (int i=0;i<N;i++)
   {
      buffer[(*idx_write * G + g) * N + i] = input[N * g + i];
   }

   for (int i=0;i<N;i++)
   {
      sum = 0.f;
      output[N * g + i] = dclayer->bias[i] * (float) WEIGHTS_SCALE_BIAS;

      for (int j=0;j<L;j++)
      {
         sum += buffer[(((j+*idx_start)%L) * G + g) * N + i] * dclayer->input_weights[j*N+i]; // * (float) WEIGHTS_SCALE;

      }
      sum *= (float) WEIGHTS_SCALE;
      output[N * g + i] += sum;
   }

   

   if (dclayer->activation == ACTIVATION_SIGMOID)
   {
      for (int i=0;i<N;i++)
         output[N * g + i] = sigmoid_approx(output[N * g + i]);
   } 
   else if (dclayer->activation == ACTIVATION_TANH)
   {
      for (int i=0;i<N;i++)
         output[N * g + i] = tansig_approx(output[N * g + i]);
   } 
   else if (dclayer->activation == ACTIVATION_RELU)
   {
      for (int i=0;i<N;i++)
         output[N * g + i] = relu(output[N * g + i]);
   } 
   else if (dclayer->activation == ACTIVATION_LINEAR)
   {
      ;
   }
   }
   *idx_start = (*idx_start + 1) % L;
   *idx_write = (*idx_write + 1) % L;

}


void compute_gru_grouped(const GRULayer *gru, float *state, const float *input)
{

   int N, M, G;
   G = NUM_GROUPS;
   

   M = gru->nb_inputs;
   N = gru->nb_neurons;

   for (int g=0;g<G;g++)
   {  
      float z_out, r_out, hh_out, state_out;
      float MM_out[MAX_NEURONS*3];
      float MM_out_r[MAX_NEURONS*3];
      for (int i=0;i<(3*N);i++)
      {

         MM_out[i] = 0.f;

         for (int j=0;j<M;j++)
         {
               MM_out[i] += gru->input_weights[i * M + j]*input[M*g + j]; // * (float) WEIGHTS_SCALE;
         }
         MM_out[i] *= (float) WEIGHTS_SCALE;
         MM_out[i] += gru->bias[i] * (float) WEIGHTS_SCALE_BIAS;

         MM_out_r[i] = 0.f;

         for (int j=0;j<M;j++)
         {
               MM_out_r[i] += gru->recurrent_weights[i * M + j]*state[M*g + j]; // * (float) WEIGHTS_SCALE;
         }
         MM_out_r[i] *= (float) WEIGHTS_SCALE;
         MM_out_r[i] += gru->bias[(3*N) + i] * (float) WEIGHTS_SCALE_BIAS;

      }


      for (int i=0;i<N;i++)
      {  
         z_out = sigmoid_approx(MM_out[i] + MM_out_r[i]);
         r_out = sigmoid_approx(MM_out[N + i] + MM_out_r[N + i]);
         hh_out = tanh(MM_out[2*N + i] + r_out*MM_out_r[2* N + i]);
         state_out = z_out*state[N*g + i] + (1-z_out)*hh_out;
         state[N*g + i] = state_out;
    
      }
   }

}


void compute_rnn(RNNState *rnn, float *filter_b, float *filter_t, const float *input) {
   
   float scale_out[520];
   float projection_out[128];
   float map_out[256];
   float dconv_1_out[256];
   float dconv_2_out[256];
   float dconv_main_out[256];
   float tac_out_1[128];
   float tac_out_2[128];
   float tac_out_3[256];
   float tac_out_main[256];
   float gru_out_main[256];
   float filter_b_tmp[260];
   float filter_t_tmp[650];
   

  
  
   // scale input
   compute_scale(rnn->model->scaler, scale_out, input);
   // projection
   compute_dense(rnn->model->dense_projection, projection_out, scale_out);
   // calc mapping for all groups
   compute_dense_grouped(rnn->model->dense_map_1, map_out,  projection_out);
   // calc dconv for all groups
   compute_dconv_1xX_grouped(rnn->model->dconv_5, &rnn->dconv_5_idx_start, &rnn->dconv_5_idx_write, rnn->dconv_5_buffer, dconv_1_out, map_out);
   compute_dense_grouped(rnn->model->dense_deconv_5, dconv_2_out,  dconv_1_out);
   compute_dconv_1xX_grouped(rnn->model->dconv_3, &rnn->dconv_3_idx_start, &rnn->dconv_3_idx_write, rnn->dconv_3_buffer, dconv_1_out, dconv_2_out);
   compute_dense_grouped(rnn->model->dense_deconv_3, dconv_2_out,  dconv_1_out);
   compute_dconv_1x1_grouped(rnn->model->dconv_skip_1, dconv_main_out, dconv_2_out, map_out);
   // calc first group communication
   compute_dense_grouped(rnn->model->dense_tac1_1, tac_out_1,  dconv_main_out);
   compute_dense(rnn->model->dense_tac1_2, tac_out_2,  tac_out_1);
   compute_dense_grouped(rnn->model->dense_tac1_3, tac_out_3,  tac_out_2);
   add_skip(256, tac_out_main, tac_out_3, dconv_main_out);

   // compute grus for all groups
   compute_gru_grouped(rnn->model->gru_2_1, rnn->gru_2_1_state, tac_out_main);
   compute_gru_grouped(rnn->model->gru_2_2, rnn->gru_2_2_state, rnn->gru_2_1_state);
   compute_dconv_1x1_grouped(rnn->model->dconv_skip_2, gru_out_main, rnn->gru_2_2_state, tac_out_main);

   // calc second group communication
   compute_dense_grouped(rnn->model->dense_tac2_1, tac_out_1,  gru_out_main);
   compute_dense(rnn->model->dense_tac2_2, tac_out_2,  tac_out_1);
   compute_dense_grouped(rnn->model->dense_tac2_3, tac_out_3,  tac_out_2);
   add_skip(256, tac_out_main, tac_out_3, gru_out_main);

   // compute mapping for all groups
   compute_dense_grouped(rnn->model->dense_map_2, projection_out,  tac_out_main);

   // calc filter from projection
   compute_dense(rnn->model->dense_filt_b, filter_b_tmp,  projection_out);
   compute_dense(rnn->model->dense_filt_t, filter_t_tmp,  projection_out);

   compute_scale(rnn->model->scaler_b, filter_b, filter_b_tmp);
   compute_scale(rnn->model->scaler_t, filter_t, filter_t_tmp);

  
  
}
