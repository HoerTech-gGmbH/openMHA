// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2017 2018 2019 2020 2021 HörTech gGmbH
// Copyright © 2022 Hörzentrum Oldenburg gGmbH
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
//  Ann Spriet, Ian Proudler, Marc Moonen and Jan Wouters,
// "Adaptive Feedback Cancellation in Hearing Aids With Linear Prediction of the Desired Signal",
// IEEE Transactions on Signal Processing, vol 53, no 10, October 2005.
//
// Henning Schepker and Simon Doclo,
//"A Semidefinite Programming Approach to Min-Max Estimation of the Common Part of the Acoustic
// Feedback Paths in Hearing Aids", IEEE/ACM Transactions on Audio,
// Speech and Language Processing, vol 24, no 2, February 2016.

/* This plugin implements an adaptive feedback canceller based on the paper by Ann Spriet
 * 'Feedback control in hearing aids'. */

#include "adaptive_feedback_canceller.h"
#include <iostream>

#define PATCH_VAR(var) patchbay.connect(&var.valuechanged, this, &adaptive_feedback_canceller::update_cfg)
#define INSERT_PATCH(var) insert_member(var); PATCH_VAR(var)

std::vector<int> calcDelayValues(const std::vector<int>& raw_latency, const unsigned int correction){
    std::vector<int> delay_values;
    for(const int& elem : raw_latency)
        delay_values.push_back(elem - correction);
    return delay_values;
}

adaptive_feedback_canceller_config::adaptive_feedback_canceller_config(algo_comm_t &ac,
                                                                       const mhaconfig_t in_cfg,
                                                                       adaptive_feedback_canceller *afc)
    : ntaps(afc->filter_length.data),
      frames(in_cfg.fragsize),
      channels(in_cfg.channels),
      n_no_update_(afc->blocks_no_update.data),
      no_update_count(0),
      fragsize((afc->fragsize.data == 0) ? frames : afc->fragsize.data),
      stepsize(afc->stepsize.data),
      min_const(afc->min_const.data),
      forward_sig(frames, channels),
      LSsig_initializer(frames, channels),
      LSsig(&LSsig_initializer),
      LSsig_output(frames, channels),
      delay_forward_path(afc->delay_forward_path.data, channels),
      forward_path_proc(afc->plugloader),
      delay_roundtrip(calcDelayValues(afc->measured_roundtrip_latency.data, fragsize), channels),
      delay_update(calcDelayValues(afc->measured_roundtrip_latency.data, 1), channels),
      FBfilter_estim(channels, MHAFilter::filter_t(1,1,ntaps)),
      FBfilter_estim_ac(ac, "FBfilter_estim", ntaps, channels, false),
      FBsig_estim(frames, channels),
      ERRsig(frames, channels),
      ERRsig_ac(ac, "ERRsig", frames, channels,false),
      use_lpc_decorr(afc->use_lpc_decorr.data),
      lpc_filter(channels, MHAFilter::filter_t(1,1,ntaps)),
      white_LSsig(frames, channels),
      white_LSsig_smpl(1, channels),
      rb_white_LSsig(ntaps, channels, ntaps),
      current_power(channels, mha_real_t(0.0f)),
      white_MICsig(frames, channels),
      white_FBsig_estim(frames, channels),
      white_ERRsig(frames, channels),
      debug_mode(afc->debug_mode.data),
      current_power_ac(ac, "current_power", frames, channels, false),
      estim_err_ac(ac, "estim_err", frames, channels, false)
{
    /* MHAFilter::filter_t is initialized with 1.0 at the first B-coefficient, since the adaption works by
     * adding new values to the previous coefficients there will always be an offset of 1.0 at the first
     * B-coefficient. That is why it is set to 0.0 here */
    for (unsigned ch{0U}; ch < channels; ch++)
        FBfilter_estim[ch].B[0] = 0.0;
}

adaptive_feedback_canceller_config::~adaptive_feedback_canceller_config() {

}

inline void make_friendly_number_by_limiting( double& x )
{
    if( x > 1.0e20 )
        x = 1.0e20;
    if( x < -1.0e20 )
        x = -1.0e20;
}

mha_wave_t *adaptive_feedback_canceller_config::process(mha_wave_t *MICsig) {
    /* Compute the error signal */
    ERRsig.copy(*MICsig);
    ERRsig -= FBsig_estim;
    /* Copy error signal into other waveform_t objects to be processed individually */
    if (debug_mode)
        ERRsig_ac.copy(ERRsig);
    forward_sig.copy(ERRsig);

    /* --- FORWARD PATH --- */
    /* Add the current error signal (here forward_sig) into the delay line simulating the hearing aid algorithm delay */
    delay_forward_path.process(&forward_sig);
    /* Put the forward_sig into the plugloader which processes it with the selected plugins and puts the
     * result into LSsig */
    forward_path_proc.process(&forward_sig,&LSsig);
    /* Copy the forward path signal to the output signal. */
    LSsig_output.copy(*LSsig);

    /* --- BACKWARD PATH --- */
    if (use_lpc_decorr) {
        for(unsigned ch{0}; ch < channels; ch++) {
            /* Prewhitening the delayed input and output signals using LPC */
            lpc_filter[ch].filter(white_MICsig.buf,MICsig->buf,MICsig->num_frames,1,1,ch,ch+1);
            lpc_filter[ch].filter(white_LSsig.buf,LSsig->buf,LSsig->num_frames,1,1,ch,ch+1);
        }
    }
    else {
        white_MICsig.copy(*MICsig);
        white_LSsig.copy(*LSsig);
        white_ERRsig.copy(ERRsig);
    }

    /* Add a delay before updating the filter, compensating for the roundtrip delay - 1 */
    delay_update.process(&white_LSsig);

    mha_real_t estim_err;
    for(unsigned kf{0}; kf < frames; kf++) {
        for(unsigned ch{0}; ch < channels; ch++) {
            /* Recalculate power of rb_white_LSsig, starting with the existing buffer state
             * (updating the buffer first and then computing the power will cause the system to explode) */
            current_power[ch] = 0.0f;
            for(unsigned tap{0}; tap < ntaps; tap++)
                current_power[ch] += std::pow(rb_white_LSsig.value(tap,ch),2);
            /* Calculate the estimation power in the NLMS fashion. */
            estim_err = stepsize * value(white_ERRsig,kf,ch) / (current_power[ch] + min_const);
            /* Updating the filter coefficients */
            if (no_update_count >= n_no_update_) {
                for(unsigned tap{0}; tap < ntaps; tap++) {
                    FBfilter_estim[ch].B[tap] += estim_err * rb_white_LSsig.value(ntaps - tap - 1, ch);
                    make_friendly_number_by_limiting(FBfilter_estim[ch].B[tap]);
                }
            }
            /* If you set debug_mode to yes in your configuration this will update AC-variables
             * to be monitored later. */
            if (debug_mode) {
                current_power_ac.assign(kf,ch,current_power[ch]);
                estim_err_ac.assign(kf,ch,estim_err);
            }
            /* Add new value to a one-sample wave_t object that will eventually be written into
             * rb_white_LSsig. */
            white_LSsig_smpl.buf[ch] = white_LSsig.value(kf,ch);
        }
        /* The contents of rb_white_LSsig are updated here because we still needed the oldest value to compute
         * current_power. */
        rb_white_LSsig.discard(1);
        rb_white_LSsig.write(white_LSsig_smpl);
    }

    /* Add a delay before filtering the loudspeaker signal, compensating for the roundtrip delay - fragsize */
    delay_roundtrip.process(LSsig);

    for(unsigned ch{0}; ch < channels; ch++) {
        /* Applying the filter to the delayed output signal */
        FBfilter_estim[ch].filter(FBsig_estim.buf,LSsig->buf,LSsig->num_frames,1,1,ch,ch+1);
        if (debug_mode) {
            for(unsigned tap{0}; tap < ntaps; tap++) {
                /* Writing the filter coefficients to the AC variable to be accessible from outside */
                FBfilter_estim_ac.assign(tap,ch,FBfilter_estim[ch].B[tap]);
            }
        }
    }

    if (no_update_count < n_no_update_)
        no_update_count++;

    /* Insert the updated AC variables into the AC space */
    if (debug_mode)
        insert();

    /* Forward the output signal into the processing chain */
    return &LSsig_output;
}

void adaptive_feedback_canceller_config::insert()
{
    FBfilter_estim_ac.insert();
    ERRsig_ac.insert();
    current_power_ac.insert();
    estim_err_ac.insert();
}

/** Constructs our plugin. */
adaptive_feedback_canceller::
adaptive_feedback_canceller(MHA_AC::algo_comm_t & iac,
                            const std::string & configured_name)
    : MHAPlugin::plugin_t<adaptive_feedback_canceller_config>
    ("Prediction error method for adaptive feedback cancellation",iac),
      stepsize("Step size","0.01","]0,2]"),
      min_const("Regularization parameter","1e-20","]0,]"),
      filter_length("Length of the feedback path filter in taps","32","]0,]"),
      plugloader(*this,ac),
      fragsize("Fragsize used for internal delay computation, defaults to MHA's fragsize", "0", "[0,["),
      measured_roundtrip_latency("Latency between playback and recording of the same signal", "0", "[0,["),
      use_lpc_decorr("Make use of LPC decorrelation, NOTE: not yet implemented!","no"),
      lpc_order("Length of the lpc filter in taps", "20", "]0, 1024]"),
      delay_forward_path("Delay in the forward path processing in taps", "96", "]0,["),
      blocks_no_update("Number of iterations without updating the filter coefficients", "0", "[0,["),
      debug_mode("Set to true to get variable states from within the processing", "no")
{
    /* make the plug-in findable via "?listid" */
    set_node_id(configured_name);

    /* add parser variables and connect them to methods here INSERT_PATCH(foo_parser); */
    INSERT_PATCH(stepsize);
    INSERT_PATCH(min_const);
    INSERT_PATCH(filter_length);
    INSERT_PATCH(fragsize);
    INSERT_PATCH(measured_roundtrip_latency);
    INSERT_PATCH(delay_forward_path);
    INSERT_PATCH(blocks_no_update);
    INSERT_PATCH(debug_mode);
}

adaptive_feedback_canceller::~adaptive_feedback_canceller() {}

/** Plugin preparation.
 *  An opportunity to validate configuration parameters before instantiating a configuration.
 * @param signal_info
 *   Structure containing a description of the form of the signal (domain,
 *   number of channels, frames per block, sampling rate.
 */
void adaptive_feedback_canceller::prepare(mhaconfig_t & signal_info)
{
    //good idea: restrict input type and dimension
    if (!signal_info.channels)
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin must have at least one input channel: (%u found)\n"
                        , signal_info.channels);

    if (!signal_info.fragsize)
        throw MHA_Error(__FILE__, __LINE__,
                        "Fragment size should be at least one: (%u found)\n"
                        , signal_info.fragsize);

    if (signal_info.domain != MHA_WAVEFORM)
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin can only process waveform signals.");

    plugloader.prepare(signal_info);
    /* make sure that a valid runtime configuration exists: */
    update_cfg();
    poll_config()->insert();
}

void adaptive_feedback_canceller::update_cfg()
{
    if ( is_prepared() ) {

        /* when necessary, make a new configuration instance
         * possibly based on changes in parser variables */
        adaptive_feedback_canceller_config *config;
        config = new adaptive_feedback_canceller_config( ac, input_cfg(), this );
        push_config( config );
    }
}

/* Checks for the most recent configuration and defers processing to it. */
mha_wave_t * adaptive_feedback_canceller::process(mha_wave_t * signal)
{
    /* this stub method defers processing to the configuration class */
    return poll_config()->process( signal );
}

void adaptive_feedback_canceller::release(){
    plugloader.release();
}
/*
 * This macro connects the plugin1_t class with the MHA plugin C interface
 * The first argument is the class name, the other arguments define the
 * input and output domain of the algorithm.
 */
MHAPLUGIN_CALLBACKS(adaptive_feedback_canceller,adaptive_feedback_canceller,wave,wave)

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
(adaptive_feedback_canceller,
 "feedback-suppression adaptive",
 "This plugin implements an adaptive feedback canceller (AFC) that uses the normalized least mean squares (NLMS) method "
 "for filter estimation. The flowgraph of the algorithm is shown in Figure~\\figref{afcSimple}. "
 "\\MHAfigure[][.8\\linewidth]{Block diagram of the AFC plugin}{afcSimple}\n \\\\"
 "The basic problem of an AFC is that the microphone signal \n"

 "\\begin{equation} \\mathrm{MICsig} = \\mathrm{target} + \\mathrm{FBsig\\_true} \\label{eqn:afcMICsig}\\end{equation}"

 "consists of the \\emph{target} signal, which we want to preserve, and the true feedback signal "

 "\\begin{equation} "
 "\\mathrm{FBsig\\_true} = \\delta(t - \\mathrm{roundtrip\\_latency}) \\ast \\mathrm{FBfilter\\_true}\\left(\\mathrm{LSsig}\\right),"
 "\\end{equation} "

 "which we want to eliminate. We assume that \\emph{FBsig\\_true} is the filtered loudspeaker signal (\\emph{LSsig}), "
 "filtered by the true acoustic feedback path (\\emph{FBfilter\\_true}), which in turn is assumed as an FIR filter and unknown to the algorithm. "
 "$\\delta(t)$ is the Delta-Impulse-Function depending on the time $t$ in samples. "
 "In a real-world setup \\emph{FBsig\\_true} is delayed by the \\emph{roundtrip latency}, describing the timespan "
 "between playing back and receiving the same signal, including the delays induced by the transducers and buffering of the soundcard. "
 "This latency is also unknown but can be measured on the physical system in order to properly configure the plugin parameter "
 "\\emph{delay\\_roundtrip} and \\emph{delay\\_update}. "
 "One way to measure the roundtrip latency is to use \\texttt{jack\\_iodelay}, more info on how to do that "
 "exactly can be found in the example \\textit{31-adaptive-feedback-canceller/README.md}. \n\\\\"
 "After receiving \\emph{MICsig}, the first processing step in the AFC is to calculate the error signal "

 "\\begin{equation} "
 "\\mathrm{ERRsig} = \\mathrm{MICsig} - \\mathrm{FBsig\\_estim}."
 "\\end{equation}"

 "\\emph{ERRsig} is an estimation of the \\emph{target} by subtracting the estimated feedback signal "
 "\\emph{FBsig\\_estim} from \\emph{MICsig}. \\emph{FBsig\\_estim} is computed by filtering the delayed \\emph{LSsig} "
 "with the estimated feedback path filter \\emph{FBfilter\\_estim} "

 "\\begin{equation} "
 "\\mathrm{FBsig\\_estim} = \\mathrm{FBfilter\\_estim}\\left(\\delta(t - \\mathrm{delay\\_roundtrip}) \\ast \\left(\\mathrm{LSsig}\\right)\\right)."
 "\\end{equation} "

 "In order to measure \\emph{ERRsig} accurately, the temporal alignment between \\emph{MICsig} and \\emph{FBsig\\_estim} "
 "has to be about the same as the alignment between \\emph{target} and \\emph{FBsig\\_true} in equation (\\ref{eqn:afcMICsig}). "
 "The internal plugin parameter \\emph{delay\\_roundtrip} provides the right value for the temporal alignment and is directly computed from "
 "the measured roundtrip latency and the \\emph{fragsize}. \\textbf{Note}: This algorithm was developed assuming that the "
 "soundcard's fragsize is equal to the MHA's fragsize. The AFC's variable \\emph{fragsize} defaults to the MHA's fragsize.\n \\\\"
 "In the forward path the intended signal processing is performed. This AFC algorithm provides a pluginloader that "
 "can load a plugin into the forward path processing\\footnote{Plugin \\emph{mhachain} can be used if the forward path "
 "consists of multiple plugins.}. The forward path processing "
 "represents the hearing aid processing. In detail, a copy of \\emph{ERRsig} is used as input for the pluginloader and "
 "the output is saved to \\emph{LSsig}. \\textbf{Note}: All plugins loaded outside of the forward path processing are not "
 "considered by the AFC concerning signal properties and induced delays for the feedback path filter estimation. Additionally, a "
 "delay is added to the signal in the forward path (\\emph{delay\\_forward\\_path}). This delay is used for signal "
 "decorrelation because the filter estimation of the AFC is biased if \\emph{LSsig} and \\emph{target} are correlated. \n \\\\"
 "The backward path handles the update of the feedback filter estimation \\emph{FBfilter\\_estim} and the filtering of "
 "\\emph{LSsig} with \\emph{FBfilter\\_estim}. Both processing steps depend on the measured roundtrip latency, which "
 "is used to compute internal delays. The filter update step in this AFC algorithm is performed using the NLMS method in "
 "the time-domain (refer to [1]) for each sample in a \\texttt{process()} callback. "
 "\\emph{stepsize} is the variable for the adaption speed. Changing the value of "
 "\\emph{stepsize} is a trade-off between convergence speed and estimation error, the higher the value the higher the "
 "convergence speed and estimation error. Different real-world settings demand different stepsize values, as a user you have "
 "to find out what works best, but it is recommended to keep the value below 0.2 (see [2]). "
 "\\emph{min\\_const} is a regularization parameter to avoid division by zero. It should be kept as low as possible to not "
 "interfere with the adaption at low output levels. \n \\\\"
 "This plugin performs feedback cancellation for each channel seperately. A channel here is meant as a loudspeaker-microphone pair. "
 "Therefore, you must have the same number of input and output channels. Please refer to \\textit{openMHA/examples/31-adaptive-feedback-canceller} "
 "for usage examples of the plugin. \n"
 "\n"
 "Reference:\n"
 "\n"
 "[1]: Spriet, A., Doclo, S., Moonen, M., and Wouters, J. (2008). Feedback control in hearing aids. In \\textit{Springer Handbook}"
 "\\textit{of Speech Processing} (pp. 979-1000). Springer, Berlin, Heidelberg.\n "
 "\n"
 "[2]: Moonen, M., and Proudler, I. (1998). Introduction to adaptive signal processing. \\textit{Department of Electrical} "
 "\\textit{Engineering ESAT/SISTA KU Leuven, Leuven, Belgium}, 105-107."
 )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
