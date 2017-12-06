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

#include <cmath>
#include "lpc_burg-lattice.h"

#define PATCH_VAR(var) patchbay.connect(&var.valuechanged, this, &lpc_burglattice::update_cfg)
#define INSERT_PATCH(var) insert_member(var); PATCH_VAR(var)

lpc_burglattice_config::lpc_burglattice_config(algo_comm_t &iac, const mhaconfig_t in_cfg, lpc_burglattice *_lpc)
    : ac(iac)
    , forward(_lpc->lpc_order.data, in_cfg.channels)
    , backward(_lpc->lpc_order.data, in_cfg.channels * 2)
    , kappa(_lpc->lpc_order.data, in_cfg.channels)
    , kappa_block(ac, _lpc->name_kappa.data, _lpc->lpc_order.data * in_cfg.fragsize, in_cfg.channels, true)
    , dm(_lpc->lpc_order.data, in_cfg.channels)
    , nm(_lpc->lpc_order.data, in_cfg.channels)
    , lambda(_lpc->lambda.data)
    , lpc_order(_lpc->lpc_order.data)
    , name_f(_lpc->name_f.data)
    , name_b(_lpc->name_b.data)
{
    //initialize plugin state for a new configuration
    forward.assign(EPSILON);
    backward.assign(EPSILON);
    dm.assign(EPSILON);
    nm.assign(EPSILON);
}

lpc_burglattice_config::~lpc_burglattice_config()
{
}

//the actual processing implementation
mha_wave_t *lpc_burglattice_config::process(mha_wave_t *wave)
{
    //do actual processing here using configuration state

    // Get the forward and backward prediction values from the AC space
    s_f = MHA_AC::get_var_waveform(ac, name_f);
    s_b = MHA_AC::get_var_waveform(ac, name_b);

    // Compute the lattice filter coefficients for each channel of the input signal
    for (unsigned int chan = 0; chan < wave->num_channels; chan++) {

        for (unsigned int sample = 0; sample < wave->num_frames; sample++) {

            forward(0, chan) = value(s_f, sample, chan);

            backward(0, chan * 2 + 1) = backward(0, chan * 2);
            backward(0,chan * 2) = value(s_b, sample, chan);

            kappa_block(sample * lpc_order, chan) = kappa(0, chan);

            // Computation of the lattice filter coefficients, kappa will be saved in the AC space for the linear prediction
            for (int m = 1; m < lpc_order; m++) {

                backward(m, chan * 2 + 1)  = backward(m, chan * 2);
                kappa_block(sample * lpc_order + m, chan) = kappa(m, chan);

                // Burg Lattice Algorithm
                dm(m-1, chan) = lambda * dm(m-1, chan) + (1-lambda) * (pow(forward(m-1, chan), 2) + pow(backward(m-1,chan * 2 + 1), 2));
                nm(m-1, chan) = lambda * nm(m-1, chan) + (1-lambda) * (-2)*(forward(m-1, chan) * backward(m-1, chan * 2 + 1));
                kappa(m, chan) = nm(m-1, chan) / (dm(m-1, chan));

                // Adaptive Lattice Predictor
                forward(m, chan)    = forward(m-1, chan) + kappa(m, chan) * backward(m-1, chan * 2 + 1);
                backward(m, chan * 2)  = backward(m-1, chan * 2 + 1) + kappa(m, chan) * forward(m-1, chan);
            }
        }
    }

    //return current fragment
    return wave;
}

/** Constructs our plugin. */
lpc_burglattice::lpc_burglattice(algo_comm_t & ac,
                                   const std::string & chain_name,
                                   const std::string & algo_name)
    : MHAPlugin::plugin_t<lpc_burglattice_config>(
          "This plugin estimates the linear predictive coding coefficients for estimating the next sample value of a time series using the Burg-Lattice approach.\n\n"
          "The estimated parameters are saved in the AC space.\n",ac)
    , lpc_order("LPC order defines the number of coffecients to be estimated", "21", "]0,]")
    , name_kappa("Name of the kappa parameter of the Burg-Lattice algorithm in the AC domain to be used for the joint estimation of more than one time series", "km")
    , name_f("Name of the forward linear prediction parameter", "")
    , name_b("Name of the backward linear prediction parameter", "")
    , lambda("Forgetting factor for the linear predictor", "0.99375", "[0,1]")
{
    //add parser variables and connect them to methods here
    INSERT_PATCH(lpc_order);
    INSERT_PATCH(name_kappa);
    INSERT_PATCH(name_f);
    INSERT_PATCH(name_b);
    INSERT_PATCH(lambda);
}

lpc_burglattice::~lpc_burglattice() {}

/** Plugin preparation.
 *  An opportunity to validate configuration parameters before instantiating a configuration.
 * @param signal_info
 *   Structure containing a description of the form of the signal (domain,
 *   number of channels, frames per block, sampling rate.
 */
void lpc_burglattice::prepare(mhaconfig_t & signal_info)
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

void lpc_burglattice::update_cfg()
{
    if ( is_prepared() ) {

        //when necessary, make a new configuration instance
        //possibly based on changes in parser variables
        lpc_burglattice_config *config;
        config = new lpc_burglattice_config( ac, input_cfg(), this );
        push_config( config );
    }
}

/**
 * Checks for the most recent configuration and defers processing to it.
 */
mha_wave_t * lpc_burglattice::process(mha_wave_t * signal)
{
    //this stub method defers processing to the configuration class
    return poll_config()->process( signal );
}

/*
 * This macro connects the plugin1_t class with the MHA plugin C interface
 * The first argument is the class name, the other arguments define the
 * input and output domain of the algorithm.
 */
MHAPLUGIN_CALLBACKS(lpc_burg-lattice, lpc_burglattice, wave, wave)

/*
 * This macro creates code classification of the plugin and for
 * automatic documentation.
 *
 * The first argument to the macro is a space separated list of
 * categories, starting with the most relevant category. The second
 * argument is a LaTeX-compatible character array with some detailed
 * documentation of the plugin.
 */
MHAPLUGIN_DOCUMENTATION(lpc_burg-lattice,
        "adaptive",
        "This plugin estimates the parameters for the forward and backward linear prediction using the Burg - Lattice algorithm. The previous estimate of the $\\kappa$ parameter is saved in the AC space for future use in the {\\tt lpc\\_bl\\_predictor} plugin to estimate several time-series sharing the same $\\kappa$ values.\n\n"
        "For the estimation of $\\kappa$ the following series of equations are used: "
        "For each $\\kappa$ in $[2 \\cdots P]$, $P$ being the lpc order"
        "\\begin{eqnarray}\n"
        "dm(m-1) &=& \\lambda * dm(m-1) + (1-\\lambda) * (f(m-1)^2 + b(m-1,2)^2)\\\\\n"
        "nm(m-1) &=& \\lambda * nm(m-1) + (1-\\lambda) * -2*f(m-1)*b(m-1,2)\\\\\n"
        "km(m,1) &=& \\frac{nm(m-1)}{dm(m-1)}.\n"
        "\\end{eqnarray}\n"
        "Note that the previous estimate of $\\kappa$, which is given by $\\kappa(m,2)$ is saved in the AC space."
        )
