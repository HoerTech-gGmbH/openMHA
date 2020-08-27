// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2013 2014 2015 2018 2020 HörTech gGmbH
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

#include "mha_plugin.hh"

#include "gsc_adaptive_stage.hh"
#include "gsc_adaptive_stage_if.hh"


/** Constructs the interface to the adaptive filter plugin.
 * @param ac Handle to the ac space
 */
gsc_adaptive_stage::gsc_adaptive_stage_if::gsc_adaptive_stage_if(algo_comm_t & ac,
                                           const std::string &,
                                           const std::string &)
  : MHAPlugin::plugin_t<gsc_adaptive_stage>("Frequency-domain block-adaptive filter"
                                            " specialised for usage as gsc adaptive stage",ac),
    lenOldSamps("how many old samples to buffer", "1024", "[0,5000]"),
    doCircularComp("whether to compensate for circular convolution", "no"),
    mu("step size for gradient computation", "0.2", "[0,2]"),
    alp("autoregressive coefficient for estimating PSD", "0.5", "[0,1]"),
    useVAD("whether to use the VAD given in AC-variable", "no"),
    vadName("Name of VAD AC-variable","VAD")
{
  insert_item("lenOldSamps", &lenOldSamps);
  insert_item("doCircularComp", &doCircularComp);
  insert_item("mu", &mu);
  insert_item("alp", &alp);
  insert_item("useVAD", &useVAD);
  insert_item("vadName", &vadName);
  patchbay.connect(&lenOldSamps.valuechanged, this,
                   &gsc_adaptive_stage_if::on_model_param_valuechanged);
  patchbay.connect(&doCircularComp.valuechanged, this,
                   &gsc_adaptive_stage_if::on_model_param_valuechanged);
  patchbay.connect(&mu.valuechanged, this,
                   &gsc_adaptive_stage_if::on_model_param_valuechanged);
  patchbay.connect(&alp.valuechanged, this,
                   &gsc_adaptive_stage_if::on_model_param_valuechanged);
  patchbay.connect(&useVAD.valuechanged, this,
                   &gsc_adaptive_stage_if::on_model_param_valuechanged);
  patchbay.connect(&vadName.valuechanged, this,
                   &gsc_adaptive_stage_if::on_model_param_valuechanged);
}

/** Plugin preparation. This plugin checks that the input signal has the
 * spectral domain and contains at least one channel
 * @param signal_info
 *   Structure containing a description of the form of the signal (domain,
 *   number of channels, frames per block, sampling rate.
 */
void gsc_adaptive_stage::gsc_adaptive_stage_if::prepare(mhaconfig_t & signal_info)
{
  if (signal_info.domain != MHA_WAVEFORM)
    throw MHA_Error(__FILE__, __LINE__,
                    "This plugin can only process waveform signals.");

  //there is only one output channel, the error signal
  signal_info.channels = 1;

  /* remember the transform configuration (i.e. channel numbers): */
  tftype = signal_info;

  /* make sure that a valid runtime configuration exists: */
  update_cfg();
}

/*When one of the angles or radii changes, recompute the head model */
void gsc_adaptive_stage::gsc_adaptive_stage_if::on_model_param_valuechanged()
{
  //only push configurations if prepare has already been called
    if ( is_prepared() ) update_cfg();
}

/** Update the rt config */
void gsc_adaptive_stage::gsc_adaptive_stage_if::update_cfg()
{
  gsc_adaptive_stage *config = new gsc_adaptive_stage( ac, input_cfg(),
                                                 lenOldSamps.data, doCircularComp.data,
                                                 mu.data, alp.data, useVAD.data, vadName.data );
  push_config( config );
}

/** This plugin implements noise reduction using spectral
 * subtraction: By nonnegative subtraction from the output magnitude
 * of the estimated noise magnitude spectrum.
 * @param signal
 *   Pointer to the input signal structure.
 * @return
 *   Returns a pointer to the input signal structure,
 *   with a the signal modified by this plugin.
 */
mha_wave_t * gsc_adaptive_stage::gsc_adaptive_stage_if::process(mha_wave_t * signal)
{
  poll_config();
  return cfg->process(signal);
}

/*
 * This macro connects the gsc_adaptive_stage_if class with the MHA plugin C interface
 * The first argument is the class name, the other arguments define the
 * input and output domain of the algorithm.
 */
MHAPLUGIN_CALLBACKS(gsc_adaptive_stage,gsc_adaptive_stage::gsc_adaptive_stage_if,wave,wave)

/*
 * This macro creates code classification of the plugin for
 * automatic documentation.
 *
 * The first argument to the macro is a space separated list of
 * categories, starting with the most relevant category. The second
 * argument is a LaTeX-compatible character array with some detailed
 * documentation of the plugin.
 */
MHAPLUGIN_DOCUMENTATION(gsc_adaptive_stage,
                        "adaptive filter",
                        "Implements the FIR-filter block-adaptation scheme based on the NLMS optimization"
                        " found in John J. Shynk, "
                        "Frequency-Domain and Multirate Adaptive Filtering, "
                        "IEEE Signal Processing Magazine, 1992, with specialisations"
                        " to be used as the adaptive filter stage of an adaptive MVDR beamformer as"
                        " described in Baumgaertel, et al. 2015."
                        " Comparing Binaural Pre-processing Strategies I: Instrumental Evaluation. "
                        " Trends in hearing, 19, 2331216515617916."
                        "\n\n"
                        )
// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
