// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2014 2015 2016 2018 2019 2020 HörTech gGmbH
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

/*
 * This plugin computes the GCC-PHAT given the stereo time domain
 * signal, which is forwarded to the next plugin in the chain
 * unaltered.
 */

#include "doasvm_feature_extraction.h"

#define PATCH_VAR(var) patchbay.connect(&var.valuechanged, this, &doasvm_feature_extraction::update_cfg)
#define INSERT_PATCH(var) insert_member(var); PATCH_VAR(var)

doasvm_feature_extraction_config::doasvm_feature_extraction_config(algo_comm_t &ac, const mhaconfig_t in_cfg, doasvm_feature_extraction *_doagcc)
    : doagcc(_doagcc)
    , wndlen(in_cfg.fragsize)
    , fftlen(_doagcc->fftlen.data)
    , G_length(fftlen / 2 + 1)
    , vGCC_ac(ac, _doagcc->vGCC_name.data.c_str(), 2*_doagcc->max_lag.data * _doagcc->nupsample.data + 1, 1, true)
    , hifftwin_sum(0)
    , proc_wave(fftlen, 2)
    , hwin(fftlen, 1)
    , hifftwin(G_length, 1)
    , vGCC(fftlen * _doagcc->nupsample.data, 1)
    , in_spec(G_length, 2)
    , G(fftlen * _doagcc->nupsample.data, 1)
{
    //initialize plugin state for a new configuration

    // initialize the Hanning window for the FFT
    mha_wave_t _hwin;
    _hwin.num_channels = 1;
    _hwin.num_frames = fftlen;

    if( (_hwin.buf = (mha_real_t *)hannf(fftlen)) == 0 )
        throw MHA_Error(__FILE__, __LINE__, "Error initialising the window function.");

    hwin.copy(_hwin);
    delete[] _hwin.buf;

    // initialize the Hanning window for the IFFT
    mha_wave_t _hifftwin;
    _hifftwin.num_channels = 1;
    _hifftwin.num_frames = 2 * G_length;
    if( (_hifftwin.buf = (mha_real_t *)hannf(2 * G_length)) == 0 )
        throw MHA_Error(__FILE__, __LINE__, "Error initialising the IFFT window function.");

    hifftwin.copy_from_at(0, G_length, _hifftwin, G_length);
    hifftwin.scale_channel(0, G_length / hifftwin.sum_channel(0) * _doagcc->nupsample.data);
    delete[] _hifftwin.buf;

    // initialize the input waveform for processing with zeros (including the zero padding)
    proc_wave.assign(0);

    // initialize the GCC output matrix with zeros (including the upscaling)
    for (unsigned int f=0; f < G.num_frames; ++f)
        G(f,0) = mha_complex(0,0);

    // initialize the FFT function
    if( (fft = mha_fft_new(fftlen)) == 0 )
        throw MHA_Error(__FILE__, __LINE__, "Error initialising the FFT function.");

    // intialize the IFFT function
    if( (ifft = mha_fft_new(G.num_frames)) == 0 )
        throw MHA_Error(__FILE__, __LINE__, "Error initialising the IFFT function.");

    // compute the portion of the full GCC matrix, which is within the max_lag interval
    GCC_start = (vGCC.num_frames + 1)/2 - _doagcc->max_lag.data * _doagcc->nupsample.data;
    GCC_end = (vGCC.num_frames + 1)/2 + _doagcc->max_lag.data * _doagcc->nupsample.data;
}

doasvm_feature_extraction_config::~doasvm_feature_extraction_config() {}

//the actual processing implementation
mha_wave_t *doasvm_feature_extraction_config::process(mha_wave_t *wave)
{
    //do actual processing here using configuration state
    memcpy(proc_wave.buf, wave->buf, wave->num_frames * wave->num_channels*sizeof(float));

    for( unsigned int i = 0; i < wave->num_frames; ++i ) {
        proc_wave.buf[i * 2] *= hwin[i];
        proc_wave.buf[i * 2 + 1] *= hwin[i];
    }

    mha_fft_wave2spec_scale(fft, &proc_wave, &in_spec);

    for( unsigned int i = 0; i < G_length; ++i ) {
        // compute the cross correlation
        G(i, 0) = value(in_spec, i, 0) * _conjugate(value(in_spec, i, 1));

        /* apply PHAT weighting
         *
         * Does the zero guard make sense, or is it better to explicitly deal
         * with NaNs?
         */
        G[i] /= abs(G[i]) + 1e-15;

        // apply ifft window
        G(i, 0) = value(G, i, 0) * value(hifftwin, i, 0);
    }

    // reconstruct mirror frequencies
    const unsigned int G_offset = fftlen % 2; // even = 0, odd = 1
    for( unsigned int i = G_length - 2 + G_offset; i > 0; --i )
        G(G.num_frames - i, 0) = _conjugate(value(G, i, 0));

    // ifft and swap halves of csd to compensate for fftshift
    mha_fft_spec2wave_scale(ifft, &G, &vGCC);

    ifftshift(&vGCC);

    // compute bins according to physically expedient range of delay values and
    // keep only that part of the GCC fucntion
    for( unsigned int i = GCC_start; i <= GCC_end; ++i )
        vGCC_ac[i-GCC_start] = vGCC.buf[i];

    //return current fragment
    return wave;
}

/** Constructs our plugin. */
doasvm_feature_extraction::doasvm_feature_extraction(algo_comm_t & ac,
                                                     const std::string & chain_name,
                                                     const std::string & algo_name)
    : MHAPlugin::plugin_t<doasvm_feature_extraction_config>("Plugin for computing the generalized cross correlation with phase transform (GCC-PHAT)",ac)
    , fftlen("The length of the FFT window", "160", "[0,[")
    , max_lag("Maximum lag in samples between microphones (setup-dependent)", "20", "[0,[")
    , nupsample("The amount the GCC-PHAT spectrum is oversampled", "4", "[0,[")
    , vGCC_name("The name of the AC variable for saving the GCC matrix in", "vGCC_ac")
{
    // make the plug-in findable via "?listid"
    set_node_id(algo_name);

    //add parser variables and connect them to methods here
    //INSERT_PATCH(foo_parser);

    INSERT_PATCH(fftlen);
    INSERT_PATCH(max_lag);
    INSERT_PATCH(nupsample);
    INSERT_PATCH(vGCC_name);
}

doasvm_feature_extraction::~doasvm_feature_extraction() {}

/** Plugin preparation.
 *  An opportunity to validate configuration parameters before instantiating a configuration.
 * @param signal_info
 *   Structure containing a description of the form of the signal (domain,
 *   number of channels, frames per block, sampling rate.
 */
void doasvm_feature_extraction::prepare(mhaconfig_t & signal_info)
{
    //good idea: restrict input type and dimension
    if (signal_info.channels != 2)
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin must have 2 input channels: (%u found)\n"
                        "[Left, Right].", signal_info.channels);

    if (signal_info.domain != MHA_WAVEFORM)
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin can only process spectrum signals.");

     /* make sure that a valid runtime configuration exists: */
    update_cfg();
}

void doasvm_feature_extraction::update_cfg()
{
    if ( is_prepared() ) {

        //when necessary, make a new configuration instance
        //possibly based on changes in parser variables
        doasvm_feature_extraction_config *config;
        config = new doasvm_feature_extraction_config( ac, input_cfg(), this );
        push_config( config );
    }
}

/**
 * Checks for the most recent configuration and defers processing to it.
 */
mha_wave_t * doasvm_feature_extraction::process(mha_wave_t * signal)
{
    //this stub method defers processing to the configuration class
    return poll_config()->process( signal );
}

/*
 * This macro connects the plugin1_t class with the MHA plugin C interface
 * The first argument is the class name, the other arguments define the
 * input and output domain of the algorithm.
 */
MHAPLUGIN_CALLBACKS(doasvm_feature_extraction,doasvm_feature_extraction,wave,wave)

/*
 * This macro creates code classification of the plugin and for
 * automatic documentation.
 *
 * The first argument to the macro is a space separated list of
 * categories, starting with the most relevant category. The second
 * argument is a LaTeX-compatible character array with some detailed
 * documentation of the plugin.
 */
MHAPLUGIN_DOCUMENTATION\
(doasvm_feature_extraction,
 "spatial feature-extraction binaural",
 "This plugin computes the generalized cross correlation with phase"
 " transform (GCC-PHAT)."
 " The input to this plugin is a stereo time domain signal."
 " The GCC-PHAT matrix is saved into the AC space."
 )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
