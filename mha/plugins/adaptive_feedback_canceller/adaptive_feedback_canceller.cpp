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

/*
 * This plugin is an extension of the nlms_wave plugin and computes
 * not only the nlms estimate of the feedback path but also performs
 * the necessary steps for delaying the input and output signals as
 * well as the prewhitening using the LPC coefficients.
 */

#include "adaptive_feedback_canceller.h"

#define PATCH_VAR(var) patchbay.connect(&var.valuechanged, this, &adaptive_feedback_canceller::update_cfg)
#define INSERT_PATCH(var) insert_member(var); PATCH_VAR(var)

adaptive_feedback_canceller_config::
adaptive_feedback_canceller_config(MHA_AC::algo_comm_t & ac,
                                   const mhaconfig_t in_cfg,
                                   adaptive_feedback_canceller *afc)
    : ac(ac),
      ntaps(afc->ntaps.data),
      frames(in_cfg.fragsize),
      channels(in_cfg.channels), //all input channels processed
      s_E(ac, afc->name_e.data, in_cfg.fragsize, in_cfg.channels, false), // Prediction error
      F(ac,afc->name_f.data, afc->ntaps.data,in_cfg.channels,false), // Estimated filter is saved in the AC space
      Pu(afc->ntaps.data,in_cfg.channels),                                // and thereby made accessible to other
      name_lpc_(afc->name_lpc.data),                                      // plugins in the same chain
      n_no_update_(afc->n_no_update.data),
      no_iter(0),
      iter(0),
      v_G(1, channels),
      s_U(frames, channels),
      s_E_afc_delay(afc->afc_delay.data, channels),
      s_W(afc->delay_w.data, in_cfg.channels),
      s_Wflt(afc->ntaps.data, in_cfg.channels, afc->ntaps.data),
      s_U_delay(afc->delay_d.data, channels),
      s_U_delayflt(afc->lpc_order.data + 1, in_cfg.channels, afc->lpc_order.data + 1),
      F_Uflt(1, channels), // Filtered output signal is saved to be recalled in the next iteration to compute the error signal
      s_Y_delay(afc->delay_d.data, in_cfg.channels),
      s_Y_delayflt(afc->lpc_order.data + 1, in_cfg.channels, afc->lpc_order.data + 1),
      UbufferPrew(afc->ntaps.data, in_cfg.channels, afc->ntaps.data)
{
    //initialize plugin state for a new configuration

    if( (afc->gains.data.size() != channels) && (afc->gains.data.size() != 1) )
        throw MHA_Error(__FILE__,__LINE__,
                        "The number of entries in the gain vector must be"
                        " either %u (one per channel) or 1 (same gains for all channels)", channels);

    if( afc->gains.data.size() == 1 ){

        for(unsigned int ch = 0; ch < channels;ch++)
            v_G.value(0, ch) = pow(10.0, 0.05 * afc->gains.data[0]);
    }else{
        for(unsigned int ch = 0; ch < channels; ch++)
            v_G.value(0, ch) = pow(10.0, 0.05 * afc->gains.data[ch]);
    }

    EPrew.buf = new mha_real_t[in_cfg.channels];
    EPrew.num_frames = 1;
    EPrew.num_channels = in_cfg.channels;

    UPrew.buf = new mha_real_t[in_cfg.channels];
    UPrew.num_frames = 1;
    UPrew.num_channels = in_cfg.channels;

    YPrew.buf = new mha_real_t[in_cfg.channels];
    YPrew.num_frames = 1;
    YPrew.num_channels = in_cfg.channels;

    UPrewW.buf = new mha_real_t[in_cfg.channels];
    UPrewW.num_frames = 1;
    UPrewW.num_channels = in_cfg.channels;

    smpl.buf = new mha_real_t[in_cfg.channels];
    smpl.num_frames = 1;
    smpl.num_channels = in_cfg.channels;

    for (unsigned int ch = 0; ch < channels; ch++)
        smpl.buf[ch] = 0;

    s_Usmpl = &smpl;

}

adaptive_feedback_canceller_config::~adaptive_feedback_canceller_config()
{
    delete EPrew.buf;
    delete UPrew.buf;
    delete YPrew.buf;
    delete UPrewW.buf;
    delete smpl.buf;
}

inline void make_friendly_number_by_limiting( mha_real_t& x )
{
    if( x > 1.0e20 )
        x = 1.0e20;
    if( x < -1.0e20 )
        x = -1.0e20;
}

// the actual processing implementation
// In the input buffer (s_Y), the oldest sample ist the first sample (0th) of the buffer
mha_wave_t *adaptive_feedback_canceller_config::process(mha_wave_t *s_Y, mha_real_t rho, mha_real_t c)
{
    //do actual processing here using configuration state

    // Read in the LPC coefficients
    s_LPC = MHA_AC::get_var_waveform(ac, name_lpc_);

    if ( s_LPC.num_channels != channels )
        {
            throw MHA_Error(__FILE__,__LINE__,"The number of input channels %u doesn't match"
                            " the number of channels %u in name_d:%s",
                            channels, s_LPC.num_channels, name_lpc_.c_str());
        }

    unsigned int ch, kf, tap, fidx;
    mha_real_t err;


    for(kf = 0; kf < frames; kf++) {

        iter++;

        if (no_iter < n_no_update_)
            no_iter++;


        // Discard the oldest sample of the buffer compensating for the transducer delays
        s_Wflt.discard(1);

        // Discard the oldest sample of the delayed buffer of the desired signal
        s_Y_delayflt.discard(1);

        // Discard the oldest sample of the buffer output signal
        s_U_delayflt.discard(1);

        // Add the current output sample into the delayline for prewhitening with the LPC coefficients
        s_U_delayflt.write(*s_U_delay.process(s_Usmpl));


        // Add the current input sample into the delayline for prewhitening with the LPC coefficients
        for(ch = 0; ch < channels; ch++)
            value(smpl, 0, ch) = value(s_Y, kf, ch);

        s_Y_delayflt.write(*s_Y_delay.process(&smpl));


        // Forward path
        for(ch = 0; ch < channels; ch++) {

            // Compute the power of the prewhitened output buffer without the oldest sample
            // The buffer will be shifted and this sample will be removed from the buffer to make place for the new sample
            Pu[ch] = 0;
            for(unsigned int i = 1; i < ntaps; i++) {
                PSD_val = UbufferPrew.value(i, ch) * UbufferPrew.value( i, ch); // index = 0 corresponds to the oldest sample
       
            Pu[ch] += PSD_val;
            }
       
            // Compute the a priori prediction error
            s_E.value(kf, ch) = value(s_Y, kf, ch) - F_Uflt.value(0, ch);
            smpl.buf[ch] = s_E.value(kf,ch);
        }

        // Add the current prediction error into the prediction error delay line
        // Get the delayed sample out of it
        s_Usmpl = s_E_afc_delay.process(&smpl);

        // Compute the gain in the forward path
        for (ch = 0; ch < channels; ch++)
            s_Usmpl->buf[ch] *= v_G.value(0, ch);

        s_U.copy_from_at(kf, 1, *s_Usmpl, 0);

        // Add the output sample into the delay line compensating for the transducer delays
        s_Usmpl = s_W.process(s_Usmpl);

        // Add the current output sample into the buffer for computing the a priori prediction error
        s_Wflt.write(*s_Usmpl);

        // Backward Path
        for(ch = 0; ch < channels; ch++) {
            // Prewhitening the delayed input and output signals using LPC
            value(YPrew, 0, ch) = 0;
            value(UPrew, 0, ch) = 0;

            for (unsigned int i = 0; i < s_LPC.num_frames; i++) {
                value(YPrew, 0, ch) += value(s_LPC, s_LPC.num_frames - i - 1, ch) * s_Y_delayflt.value(i, ch);
                value(UPrew, 0, ch) += value(s_LPC, s_LPC.num_frames - i - 1, ch) * s_U_delayflt.value(i, ch);
            }

            // Updating the power of the prewhitened output signal
            PSD_val = value(UPrew, 0, ch) * value(UPrew, 0, ch);
            Pu[ch] += PSD_val;

            // Updating the prewhitened filtered output signal
            // by filtering the delayed prewhitened output signal with the last estimate of the feedback path
            // Note that UbufferPrew should first be shifted, before doing the scalar product.
            // Here we simulate the shift by using indexing (tap - 1)
            value(UPrewW, 0, ch) = F.value(0, ch) * value(UPrew, 0, ch);
            for(tap = 1; tap < ntaps; tap++)
                value(UPrewW, 0, ch) += F.value(tap, ch) * UbufferPrew.value(ntaps - tap, ch);

            // Computing the prewhitened error signal
            value(EPrew, 0, ch) = value(YPrew, 0, ch) - value(UPrewW, 0, ch);
             
            // err = rho * EPrew
            err = rho * value(EPrew, 0, ch);

            // err = err / (UbufferPrew' * uBufferPrew + epsilon)
            err /= (Pu[ch] + c);
            
            // Initialize the filtered output signal to 0
            F_Uflt.value(0, ch) = 0;

            // Updating the filter coefficients and applying it to the input signal
            for(tap = ntaps - 1; tap > 0; tap--) {

                fidx = ch+channels*tap;

                // Updating the filter coefficients
                if (no_iter >= n_no_update_) {
                    F.value(tap, ch) += err * UbufferPrew.value(ntaps - tap, ch);
                    make_friendly_number_by_limiting( F.buf[fidx] );
                }

                // Applying the filter to the delayed output signal, which compensates for the transducer delays:
                F_Uflt.value(0, ch) += F.value(tap, ch) * s_Wflt.value(ntaps - tap - 1, ch);
            }

            // Updating the filter coefficients for the current input sample
            if (no_iter >= n_no_update_) {
                F.value(0, ch) += err * value(UPrew, 0, ch);
                make_friendly_number_by_limiting( F.buf[ch] );
            }

            // Applying the filter to the delayed output signal, which compensates for the transducer delays:
            F_Uflt.value(0, ch) += F.value(0, ch) * s_Wflt.value(ntaps - 1, ch);
        }

        // Discard the last sample of the prewhitened output signal
        UbufferPrew.discard(1);
        UbufferPrew.write(UPrew);
    }

    // Insert the updated AC variables into the AC space
    insert();

    // Forward the output signal into the processing chain
    return &s_U;
}

void adaptive_feedback_canceller_config::insert()
{
    F.insert();
    s_E.insert();
}

/** Constructs our plugin. */
adaptive_feedback_canceller::
adaptive_feedback_canceller(MHA_AC::algo_comm_t & iac,
                            const std::string & configured_name)
    : MHAPlugin::plugin_t<adaptive_feedback_canceller_config>
    ("Prediction error method for adaptive feedback cancellation",iac),
    rho("Step size","0.01","]0,2]"),
    c("Regularization parameter","1e-5","]0,]"),
    ntaps("Length of the feedback path filter in taps","32","]0,]"),
    gains("Gain in dB","[0]","[-60,60]"),
    name_e("Name of the AC variable for saving the prediction error", "E"),
    name_f("Name of the AC variable for saving the adapive filter", "F"),
    name_lpc("Name of the AC variable for the LPC coefficients", "lpc"),
    lpc_order("Length of the lpc filter in taps", "20", "]0, 1024]"),
    afc_delay("Delay in the forward path in taps", "96", "]0,["),
    delay_w("Delay in the adaptive filtering path due to the microphone and loudspeaker transducers in taps", "130", "[0,["),
    delay_d("Delay in the adaptive filtering path for the LPC in taps", "161", "[0,["),
    n_no_update("Number of iterations without updating the filter coefficients", "0", "[0,1024[")
{
    // make the plug-in findable via "?listid"
    set_node_id(configured_name);

    insert_member(rho);
    insert_member(c);
    INSERT_PATCH(ntaps);
    INSERT_PATCH(gains);
    INSERT_PATCH(name_e);
    INSERT_PATCH(name_f);
    INSERT_PATCH(name_lpc);
    INSERT_PATCH(lpc_order);
    INSERT_PATCH(afc_delay);
    INSERT_PATCH(delay_w);
    INSERT_PATCH(delay_d);
    insert_member(n_no_update);

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

    /* make sure that a valid runtime configuration exists: */
    update_cfg();
    poll_config()->insert();
}

void adaptive_feedback_canceller::update_cfg()
{
    if ( is_prepared() ) {

        //when necessary, make a new configuration instance
        //possibly based on changes in parser variables
        adaptive_feedback_canceller_config *config;
        config = new adaptive_feedback_canceller_config( ac, input_cfg(), this );
        push_config( config );
    }
}

/**
 * Checks for the most recent configuration and defers processing to it.
 */
mha_wave_t * adaptive_feedback_canceller::process(mha_wave_t * signal)
{
    //this stub method defers processing to the configuration class
    return poll_config()->process( signal, rho.data, c.data );
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
