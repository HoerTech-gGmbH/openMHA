// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2016 2017 HörTech gGmbH
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

//This plugin performs samplewise processing of a given input signal for estimating the next value of the input signal using LPC with Burg-Lattice algorithm.

#include "lpc_bl_predictor.h"

#define PATCH_VAR(var) patchbay.connect(&var.valuechanged, this, &lpc_bl_predictor::update_cfg)
#define INSERT_PATCH(var) insert_member(var); PATCH_VAR(var)

lpc_bl_predictor_config::lpc_bl_predictor_config(algo_comm_t &iac, const mhaconfig_t in_cfg, lpc_bl_predictor *_lpc)
    : ac(iac)
    , f_est(ac, _lpc->name_lpc_f.data, in_cfg.fragsize, in_cfg.channels, true)
    , b_est(ac, _lpc->name_lpc_b.data, in_cfg.fragsize, in_cfg.channels, true)
    , forward( _lpc->lpc_order.data, in_cfg.channels)
    , backward(_lpc->lpc_order.data, in_cfg.channels * 2)
    , lpc_order(_lpc->lpc_order.data)
    , name_km(_lpc->name_kappa.data)
    , name_f(_lpc->name_f.data)
    , name_b(_lpc->name_b.data)
{
    //initialize plugin state for a new configuration
    forward.assign(EPSILON);
    backward.assign(EPSILON);
}

lpc_bl_predictor_config::~lpc_bl_predictor_config()
{
}

//the actual processing implementation
mha_wave_t *lpc_bl_predictor_config::process(mha_wave_t *wave)
{
    //do actual processing here using configuration state
    static int k = 0;

    // Get the kappa value from the AC space
    km = MHA_AC::get_var_waveform(ac, name_km);

    // Get the forward and backward prediction values from the AC space
    s_f = MHA_AC::get_var_waveform(ac, name_f);
    s_b = MHA_AC::get_var_waveform(ac, name_b);

    // perform filtering using the km from last iteration
    for (unsigned int chan = 0; chan < wave->num_channels; chan++) {
        for (unsigned int sample = 0; sample < wave->num_frames; sample++) {

            forward(0, chan)    = value(s_f, sample, chan);

            backward(0, chan * 2 + 1)  = backward(0, chan * 2);
            backward(0, chan * 2)  = value(s_b, sample, chan);

            for (int m = 1; m < lpc_order; m++) {

                backward(m, chan * 2 + 1)  = backward(m, chan * 2);

                if (k >= 1) {
                    // Weighted Lattice Predictor for Updating Input Vector
                    forward(m, chan)    = forward(m-1, chan) + value(km, sample * lpc_order + m, chan) * backward(m-1, chan * 2 + 1);
                    backward(m, chan * 2)  = backward(m-1, chan * 2 + 1) + value(km, sample * lpc_order + m, chan) * forward(m-1, chan);

                } else {
                    forward(m, chan)    = forward(m-1, chan);
                    backward(m,chan * 2)  = backward(m-1, chan * 2 + 1);

                    k++;
                }
            }

            f_est(sample, chan) = forward(lpc_order - 1, chan);
            b_est(sample, chan) = backward(lpc_order - 1, chan);
        }
    }

    //return current fragment
    return wave;
}

/** Constructs our plugin. */
lpc_bl_predictor::lpc_bl_predictor(algo_comm_t & ac,
                                   const std::string & chain_name,
                                   const std::string & algo_name)
    : MHAPlugin::plugin_t<lpc_bl_predictor_config>(
          "This plugin performs forward and backward linear prediction using the Burg - Lattice algorithm for computing the next value of a given time series.\n\n"
          "The estimated forward and backward linear predictionn parameters are saved in th AC space.\n",ac)
    , lpc_order("LPC order defines the number of coffecients to be estimated", "21", "]0,]")
    , name_kappa("Name of the kappa parameter of the Burg-Lattice algorithm in the AC domain to be used for the joint estimation of more than one time series", "km")
    , name_lpc_f("Name of the forward LPC estimate of the Burg-Lattice algorithm in the AC domain", "name_lpc_f")
    , name_lpc_b("Name of the backward LPC estimate of the Burg-Lattice algorithm in the AC domain", "name_lpc_b")
    , name_f("Name of the forward linear prediction parameter", "")
    , name_b("Name of the backward linear prediction parameter", "")
{
    //add parser variables and connect them to methods here
    //INSERT_PATCH(foo_parser);

    INSERT_PATCH(lpc_order);
    INSERT_PATCH(name_kappa);
    INSERT_PATCH(name_lpc_f);
    INSERT_PATCH(name_lpc_b);
    INSERT_PATCH(name_f);
    INSERT_PATCH(name_b);
}

lpc_bl_predictor::~lpc_bl_predictor() {}

/** Plugin preparation.
 *  An opportunity to validate configuration parameters before instantiating a configuration.
 * @param signal_info
 *   Structure containing a description of the form of the signal (domain,
 *   number of channels, frames per block, sampling rate.
 */
void lpc_bl_predictor::prepare(mhaconfig_t & signal_info)
{
    //good idea: restrict input type and dimension
    /*
    if (signal_info.channels != 2)
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin must have 2 input channels: (%d found)\n"
                        "[Left, Right].", signal_info.channels);
                        */

    if (signal_info.domain != MHA_WAVEFORM)
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin can only process waveform signals.");

    /* make sure that a valid runtime configuration exists: */
    update_cfg();
}

void lpc_bl_predictor::update_cfg()
{
    if ( is_prepared() ) {

        //when necessary, make a new configuration instance
        //possibly based on changes in parser variables
        lpc_bl_predictor_config *config;
        config = new lpc_bl_predictor_config( ac, input_cfg(), this );
        push_config( config );
    }
}

/**
 * Checks for the most recent configuration and defers processing to it.
 */
mha_wave_t * lpc_bl_predictor::process(mha_wave_t * signal)
{
    //this stub method defers processing to the configuration class
    return poll_config()->process( signal );
}

/*
 * This macro connects the plugin1_t class with the MHA plugin C interface
 * The first argument is the class name, the other arguments define the
 * input and output domain of the algorithm.
 */
MHAPLUGIN_CALLBACKS(lpc_bl_predictor,lpc_bl_predictor, wave,wave)

/*
 * This macro creates code classification of the plugin and for
 * automatic documentation.
 *
 * The first argument to the macro is a space separated list of
 * categories, starting with the most relevant category. The second
 * argument is a LaTeX-compatible character array with some detailed
 * documentation of the plugin.
 */
MHAPLUGIN_DOCUMENTATION(lpc_bl_predictor,
        "adaptive",
        "This plugin computes the forward and backward LPC estimates using the Burg-Lattice algorithm given the $\\kappa$ (sometimes also called $\\mu$) "
        "parameter precomputed using the {\\tt lpc\\_burg-lattice} plugin. The estimation of the forward and backward linear prediction parameters is performed using the following equations:\n"
        "For each forward and backward linear prediction parameter $f(m)$ and $b(m)$, where $m \\textrm{ in } [2 \\cdots P]$, $P$ being the lpc order"
        "\\begin{eqnarray}\n"
        "f(m) &=& f(m-1) + \\kappa(m,2)*b(m-1,2)\\\\\n"
        "b(m,1) &=& b(m-1,2) + \\kappa(m,2)*f(m-1)\n"
        "\\end{eqnarray}.\n"
        "In this implementation $\\kappa$ from the previous is used. Note that the second index of $\\kappa$ is $2$."
        )
