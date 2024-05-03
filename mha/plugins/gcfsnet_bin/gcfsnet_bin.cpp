// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2023 2024 Hörzentrum Oldenburg gGmbH
//
// openMHA is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// openMHA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License, version 3 for more details.
//
// You should have received a copy of the GNU Affero General Public License, 
// version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.


#include <cstdio>
#include "mha_plugin.hh"
#include "rnnoise.h"
#include <stdlib.h>
#include <string.h>
// #include <omp.h>


class gcfsnet_bin_t : public MHAPlugin::plugin_t<int> {
    MHAParser::int_mon_t prepared;
    MHAParser::float_t remix_factor;
    MHAParser::float_t calib_factor;

   
public:
    /** This constructor initializes the configuration language
     * variables and inserts them into the \mha configuration tree.
     */
    gcfsnet_bin_t(MHA_AC::algo_comm_t & iac, const std::string & configured_name);

    /** Plugin preparation.
     * @param signal_info 
     *   Structure containing a description of the form of the signal
     *   (domain, number of channels, frames per block, sampling rate, ...).
     */
    void prepare(mhaconfig_t & signal_info);

    void release(void);

    

    /** Signal processing performed by the plugin.  
     * @param signal
     *   Pointer to the input signal structure.
     * @return
     *   Returns a pointer to the output signal structure.
     */
     mha_spec_t* process(mha_spec_t* signal);

private:
    
    float in_frameL_r[260] = { 0.0 };
    float in_frameL_i[260] = { 0.0 };
    float out_frameL_r[65] = { 0.0 };
    float out_frameL_i[65] = { 0.0 };

    float in_frameR_r[260] = { 0.0 };
    float in_frameR_i[260] = { 0.0 };
    float out_frameR_r[65] = { 0.0 };
    float out_frameR_i[65] = { 0.0 };
    
    unsigned int num_channels_in = 4;

    MHASignal::spectrum_t *out;
    DenoiseState *state_L;
    DenoiseState *state_R;
    float calib_fac = 0;
    

};

gcfsnet_bin_t::gcfsnet_bin_t(MHA_AC::algo_comm_t & iac, const std::string &)
    : MHAPlugin::plugin_t<int>("This plugin executes the binaural GCFSnet"
                               " speech enhancement model.",iac),
      prepared("State of this plugin: 0 = unprepared, 1 = prepared"),
      remix_factor("Attenuation factor applied to the original signal for remixing",
                   "0.0"),
      calib_factor("Gain applied to algo output.",
                   "0.0")

{
   
    prepared.data = 0;
    insert_item("prepared", &prepared);
    insert_item("remix_factor", &remix_factor);
    insert_item("calib_factor", &calib_factor);


}

void gcfsnet_bin_t::prepare(mhaconfig_t & signal_info)
{
    if (signal_info.domain != MHA_SPECTRUM)
    throw MHA_Error(__FILE__, __LINE__,
                    "This plugin can only process spectrum signals.");
    if (signal_info.fftlen != 128)
    throw MHA_Error(__FILE__, __LINE__,
                    "This plugin requires an FFT length of 128, got %u.",
                    signal_info.fftlen);
    if (signal_info.channels != 4)
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin requires four input channels, got %u.",
                        signal_info.channels);

    signal_info.channels = 2;

    out = new MHASignal::spectrum_t(65, 2);
    state_L = rnnoise_create();
    state_R = rnnoise_create();

    // bookkeeping
    prepared.data = 1;
    calib_fac = pow(10., calib_factor.data/20.);
}

void gcfsnet_bin_t::release(void)
{
  rnnoise_destroy(state_L);
  rnnoise_destroy(state_R);
  delete out;
  out = 0;
  prepared.data = 0;
}

mha_spec_t* gcfsnet_bin_t::process(mha_spec_t * signal)
{
    unsigned int frame;
    unsigned int num_frames = 65;
    float scale = 1.0f;
    float unscale = 1./scale;

    out->num_frames = signal->num_frames;
    for(frame = 0; frame < num_frames; frame++)
    {
        if (num_channels_in == 4)
        {
        in_frameL_r[0 * num_frames + frame] = signal->buf[0*num_frames + frame].re * scale;
        in_frameL_i[0 * num_frames + frame] = signal->buf[0*num_frames + frame].im * scale;

        in_frameL_r[1 * num_frames + frame] = signal->buf[2*num_frames + frame].re * scale;
        in_frameL_i[1 * num_frames + frame] = signal->buf[2*num_frames + frame].im * scale;

        in_frameL_r[2 * num_frames + frame] = signal->buf[1*num_frames + frame].re * scale;
        in_frameL_i[2 * num_frames + frame] = signal->buf[1*num_frames + frame].im * scale;

        in_frameL_r[3 * num_frames + frame] = signal->buf[3*num_frames + frame].re * scale;
        in_frameL_i[3 * num_frames + frame] = signal->buf[3*num_frames + frame].im * scale;

        // ------------------------------
        in_frameR_r[0 * num_frames + frame] = signal->buf[1*num_frames + frame].re * scale;
        in_frameR_i[0 * num_frames + frame] = signal->buf[1*num_frames + frame].im * scale;

        in_frameR_r[1 * num_frames + frame] = signal->buf[3*num_frames + frame].re * scale;
        in_frameR_i[1 * num_frames + frame] = signal->buf[3*num_frames + frame].im * scale;

        in_frameR_r[2 * num_frames + frame] = signal->buf[0*num_frames + frame].re * scale;
        in_frameR_i[2 * num_frames + frame] = signal->buf[0*num_frames + frame].im * scale;

        in_frameR_r[3 * num_frames + frame] = signal->buf[2*num_frames + frame].re * scale;
        in_frameR_i[3 * num_frames + frame] = signal->buf[2*num_frames + frame].im * scale;

        }
        else
        {
            throw MHA_Error(__FILE__, __LINE__,
                        "This plugin can only process 4  channels.");
        }
    }
    // #pragma omp parallel for default(none) shared(state_L, state_R, out_frameL_r, out_frameL_i, out_frameR_r, out_frameR_i, in_frameL_r, in_frameL_i, in_frameR_r, in_frameR_i)
    rnnoise_process_frame(state_L, out_frameL_r, out_frameL_i, in_frameL_r, in_frameL_i);
    rnnoise_process_frame(state_R, out_frameR_r, out_frameR_i, in_frameR_r, in_frameR_i);
    

    for(frame = 0; frame < signal->num_frames; frame++)
    {
        out->buf[0 * num_frames + frame].re = out_frameL_r[frame] * unscale * calib_fac * (1. - remix_factor.data) + signal->buf[0*num_frames + frame].re * remix_factor.data;
        out->buf[0 * num_frames + frame].im = out_frameL_i[frame] * unscale * calib_fac * (1. - remix_factor.data) + signal->buf[0*num_frames + frame].im * remix_factor.data; 
        out->buf[1 * num_frames + frame].re = out_frameR_r[frame] * unscale * calib_fac * (1. - remix_factor.data) + signal->buf[1*num_frames + frame].re * remix_factor.data; 
        out->buf[1 * num_frames + frame].im = out_frameR_i[frame] * unscale * calib_fac * (1. - remix_factor.data) + signal->buf[1*num_frames + frame].im * remix_factor.data;
    }
    
    
    return out;
}



MHAPLUGIN_CALLBACKS(gcfsnet_bin,gcfsnet_bin_t,spec,spec)
MHAPLUGIN_DOCUMENTATION\
(gcfsnet_bin, "DNN-based spatial filter",
 "This plugin implements the binaural GCFSnet, a deep speech-enhancement model"
 " built for low-latency and low-complexity speech enhancenment in hearing"
 " aids."
 "\n\n"
 "The model was trained with Keras and the weights were exported to"
 " file rnn\\_data.c in the plugin's source code directory. "
 "The binaural GCFSnet uses all input channels as features while only using"
 " the ipsilateral channels for filtering."
 "\n\n"
 "This plugin processes short time fourier transform signal in four audio"
 " channels with audio sampling rate 16kHz, FFT length 128, Hanning window"
 " length 64 and hop size 32 samples."
)

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
