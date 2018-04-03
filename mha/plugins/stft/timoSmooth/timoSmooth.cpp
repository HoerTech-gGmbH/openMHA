// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2014 2015 2017 HörTech gGmbH
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
 * Single channel noise reduction algorithm from Breithaupt et al,
 * based on cepstral smoothing. This is the MHA interface class.
 */

#include "mha_plugin.hh"

#include "timoconfig.h"
#include "timosmooth.h"

#define INSERT_VAR(var) insert_item(#var, &var)
#define PATCH_VAR(var) patchbay.connect(&var.valuechanged, this, &timoSmooth::on_model_param_valuechanged)
#define INSERT_PATCH(var) INSERT_VAR(var); PATCH_VAR(var)

/** Constructs the beamforming plugin. */
timoSmooth::timoSmooth(algo_comm_t & ac,
                 const std::string & chain_name,
                 const std::string & algo_name)
    : MHAPlugin::plugin_t<timoConfig>("Cepstral smoothing single-channel noise reduction",ac),
      xi_min_db("Minimum a priori SNR for a bin in dB(power)","-27.0","[-50,50]"),
      f0_low("Lower limit for F0 detection in Hz","70.0","[0,400]"),
      f0_high("Upper limit for F0 detection in Hz","300","[0,400]"),
      delta_pitch("Quefrency half-width of pitch-set in samps","2","[0,20]"),
      lambda_thresh("Pitch detection threshold for smooth cepstrum in magnitude","0.2","[0,3]"),
      alpha_pitch("Alpha value to set for pitch range","0.15","[0,4]"),
      beta_const("AR coeff for smoothing of alphas(smoothing-factors)","0.96",""),
      kappa_const("Exponential bias correction constant for a priori SNR estimate","0.2886","[0,1]"),
      gain_min_db("Minimum gain in dB for a frequency bin", "-17", "[-30,0]"),
      win_f0("Window coefficients for cepstral smoothing window",
             "[0.0207 0.0656 0.1664 0.2473 0.2473 0.1664 0.0656 0.0207]","[0,1]"),
      alpha_const_vals("Piecewise values for steady-state alphas", "[0.2 0.4 0.92]","[0,2]"),
      alpha_const_limits_hz("Limits for steady-state alphas given in Hz","[93.75 625.0]","[0,10000]"),
      noisePow_name("Name of est. noise spectrum in AC space","noisePowProposedScale"),
      spp("Subparser for exporting SPP"),
      prior_q("priorQ for computing GLR and SPP from local SNR","0.5","[0,2]"),
      xi_opt_db("xiOpt in dB for computing GLR and SPP from local SNR","15","[0,40]"),
      prepared(false)
{
    INSERT_PATCH(xi_min_db);
    INSERT_PATCH(f0_low);
    INSERT_PATCH(f0_high);
    INSERT_PATCH(delta_pitch);
    INSERT_PATCH(lambda_thresh);
    INSERT_PATCH(alpha_pitch);
    INSERT_PATCH(beta_const);
    INSERT_PATCH(kappa_const);
    INSERT_PATCH(gain_min_db);
    INSERT_PATCH(win_f0);
    INSERT_PATCH(alpha_const_vals);
    INSERT_PATCH(alpha_const_limits_hz);
    INSERT_PATCH(noisePow_name);

    INSERT_VAR(spp);
    spp.INSERT_VAR(prior_q);
    spp.INSERT_VAR(xi_opt_db);

    PATCH_VAR(prior_q);
    PATCH_VAR(xi_opt_db);
}

timoSmooth::~timoSmooth()
{
    //delete initialized AC variables for export

}


/** Plugin preparation. This plugin checks that the input signal has the
   * spectral domain and contains at least one channel
   * @param signal_info
   *   Structure containing a description of the form of the signal (domain,
   *   number of channels, frames per block, sampling rate.
   */
void timoSmooth::prepare(mhaconfig_t & signal_info)
{
    if (signal_info.domain != MHA_SPECTRUM)
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin can only process spectrum signals.");

    //tell the plugin that it's ok to prepare configurations
    prepared = true;

    /* remember the transform configuration (i.e. channel numbers): */
    tftype = signal_info;
    /* make sure that a valid runtime configuration exists: */
    update_cfg();
}

/* when one of the angles or radii changes, recompute the head model */
void timoSmooth::on_model_param_valuechanged()
{
    //only push configurations if prepare has already been called
    if ( prepared ) update_cfg();
}

void timoSmooth::update_cfg()
{
    timoConfig *config;

    timo_params params( input_cfg(), xi_min_db.data, f0_low.data, f0_high.data,
                        delta_pitch.data, lambda_thresh.data, alpha_pitch.data,
                        beta_const.data, kappa_const.data, prior_q.data,
                        xi_opt_db.data, gain_min_db.data,
                        win_f0.data,
                        alpha_const_vals.data, alpha_const_limits_hz.data,
                        noisePow_name.data );

    config = new timoConfig( ac, params );

    push_config( config );
}

/** This plugin implements noise reduction using spectral
   * subtraction: by nonnegative subtraction from the output magnitude
   * of the estimated noise magnitude spectrum.
   * @param signal
   *   Pointer to the input signal structure.
   * @return
   *   Returns a pointer to the input signal structure,
   *   with a the signal modified by this plugin.
   */
mha_spec_t * timoSmooth::process(mha_spec_t * signal)
{
    poll_config();
    return cfg->process(signal);
}

/*
 * This macro connects the plugin1_t class with the MHA plugin C interface
 * The first argument is the class name, the other arguments define the 
 * input and output domain of the algorithm.
 */
MHAPLUGIN_CALLBACKS(timoSmooth,timoSmooth,spec,spec)

/*
 * This macro creates code classification of the plugin and for
 * automatic documentation.
 *
 * The first argument to the macro is a space separated list of
 * categories, starting with the most relevant category. The second
 * argument is a LaTeX-compatible character array with some detailed
 * documentation of the plugin.
 */
MHAPLUGIN_DOCUMENTATION(timoSmooth,
    "noise reduction",
    "Implements the single-channel noise reduction scheme found in "
        "Breithaupt,  Gerkmann, and Martin, "
        "A Novel A Priori SNR Estimation Approach Based on Selective "
        "Cepstro-temporal Smoothing.\n\n"
    )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
