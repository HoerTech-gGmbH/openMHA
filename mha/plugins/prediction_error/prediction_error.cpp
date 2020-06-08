// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2017 2018 2019 2020 HörTech gGmbH
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

#include "prediction_error.h"

#define PATCH_VAR(var) patchbay.connect(&var.valuechanged, this, &prediction_error::update_cfg)
#define INSERT_PATCH(var) insert_member(var); PATCH_VAR(var)

prediction_error_config::prediction_error_config(algo_comm_t &ac, const mhaconfig_t in_cfg, prediction_error *pred_err)
    : ac(ac),
      ntaps(pred_err->ntaps.data),
      frames(in_cfg.fragsize),
      channels(in_cfg.channels), //all input channels processed
      s_E(ac, pred_err->name_e.data, in_cfg.fragsize, in_cfg.channels, false), // Prediction error
      F(ac,pred_err->name_f.data, pred_err->ntaps.data,in_cfg.channels,false), // Estimated filter is saved in the AC space
      Pu(pred_err->ntaps.data,in_cfg.channels),                                // and thereby made accessible to other
      name_lpc_(pred_err->name_lpc.data),                                      // plugins in the same chain
      n_no_update_(pred_err->n_no_update.data),
      no_iter(0),
      iter(0),
      v_G(1, channels),
      s_U(frames, channels),
      s_E_pred_err_delay(pred_err->pred_err_delay.data, channels),
      s_W(pred_err->delay_w.data, in_cfg.channels),
      s_Wflt(pred_err->ntaps.data, in_cfg.channels, pred_err->ntaps.data),
      s_U_delay(pred_err->delay_d.data, channels),
      s_U_delayflt(pred_err->lpc_order.data + 1, in_cfg.channels, pred_err->lpc_order.data + 1),
      F_Uflt(1, channels), // Filtered output signal is saved to be recalled in the next iteration to compute the error signal
      s_Y_delay(pred_err->delay_d.data, in_cfg.channels),
      s_Y_delayflt(pred_err->lpc_order.data + 1, in_cfg.channels, pred_err->lpc_order.data + 1),
      UbufferPrew(pred_err->ntaps.data, in_cfg.channels, pred_err->ntaps.data)
{
    //initialize plugin state for a new configuration

    if( (pred_err->gains.data.size() != channels) && (pred_err->gains.data.size() != 1) )
        throw MHA_Error(__FILE__,__LINE__,
                        "The number of entries in the gain vector must be"
                        " either %u (one per channel) or 1 (same gains for all channels)", channels);

    if( pred_err->gains.data.size() == 1 ){

        for(unsigned int ch = 0; ch < channels;ch++)
            v_G.value(0, ch) = pow(10.0, 0.05 * pred_err->gains.data[0]);
    }else{
        for(unsigned int ch = 0; ch < channels; ch++)
            v_G.value(0, ch) = pow(10.0, 0.05 * pred_err->gains.data[ch]);
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

prediction_error_config::~prediction_error_config()
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
mha_wave_t *prediction_error_config::process(mha_wave_t *s_Y, mha_real_t rho, mha_real_t c)
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
        s_Usmpl = s_E_pred_err_delay.process(&smpl);

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

void prediction_error_config::insert()
{
    F.insert();
    s_E.insert();
}

/** Constructs our plugin. */
prediction_error::prediction_error(algo_comm_t & ac,
                                   const std::string & chain_name,
                                   const std::string & algo_name)
    : MHAPlugin::plugin_t<prediction_error_config>("Prediction error method for adaptive feedback cancellation",ac),
    rho("Step size","0.01","]0,2]"),
    c("Regularization parameter","1e-5","]0,]"),
    ntaps("Length of the feedback path filter in taps","32","]0,]"),
    gains("Gain in dB","[0]","[-60,60]"),
    name_e("Name of the AC variable for saving the prediction error", "E"),
    name_f("Name of the AC variable for saving the adapive filter", "F"),
    name_lpc("Name of the AC variable for the LPC coefficients", "lpc"),
    lpc_order("Length of the lpc filter in taps", "20", "]0, 1024]"),
    pred_err_delay("Delay in the forward path in taps", "96", "]0,["),
    delay_w("Delay in the adaptive filtering path due to the microphone and loudspeaker transducers in taps", "130", "[0,["),
    delay_d("Delay in the adaptive filtering path for the LPC in taps", "161", "[0,["),
    n_no_update("Number of iterations without updating the filter coefficients", "0", "[0,1024[")
{
    // make the plug-in findable via "?listid"
    set_node_id(algo_name);

    //add parser variables and connect them to methods here
    //INSERT_PATCH(foo_parser);
    insert_member(rho);
    insert_member(c);
    INSERT_PATCH(ntaps);
    INSERT_PATCH(gains);
    INSERT_PATCH(name_e);
    INSERT_PATCH(name_f);
    INSERT_PATCH(name_lpc);
    INSERT_PATCH(lpc_order);
    INSERT_PATCH(pred_err_delay);
    INSERT_PATCH(delay_w);
    INSERT_PATCH(delay_d);
    insert_member(n_no_update);

}

prediction_error::~prediction_error() {}

/** Plugin preparation.
 *  An opportunity to validate configuration parameters before instantiating a configuration.
 * @param signal_info
 *   Structure containing a description of the form of the signal (domain,
 *   number of channels, frames per block, sampling rate.
 */
void prediction_error::prepare(mhaconfig_t & signal_info)
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

void prediction_error::update_cfg()
{
    if ( is_prepared() ) {

        //when necessary, make a new configuration instance
        //possibly based on changes in parser variables
        prediction_error_config *config;
        config = new prediction_error_config( ac, input_cfg(), this );
        push_config( config );
    }
}

/**
 * Checks for the most recent configuration and defers processing to it.
 */
mha_wave_t * prediction_error::process(mha_wave_t * signal)
{
    //this stub method defers processing to the configuration class
    return poll_config()->process( signal, rho.data, c.data );
}

/*
 * This macro connects the plugin1_t class with the MHA plugin C interface
 * The first argument is the class name, the other arguments define the
 * input and output domain of the algorithm.
 */
MHAPLUGIN_CALLBACKS(prediction_error,prediction_error,wave,wave)

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
(prediction_error,
 "feedback-suppression adaptive",
 "This plugin implements the prediction error method to perform adaptive feedback cancellation."
 "The prediction error method estimates the feedback path by minimizing the error between the desired and the"
 " predicted output signals. The algorithm handles each input sample within each input channel separately.\n"
 "\n"
 "The plugin computes not only the least mean squares (NLMS) estimate of the feedback path but also performs the"
 " necessary steps for delaying the input and output signals "
 "as well as the prewhitening using the linear predictive coding (LPC) coefficients. The flow graph of the whole"
 " processing chain is shown in Figure~\\figref{FlowgraphAFC}.\n"
 "\n"
 "\\MHAfigure[][.8\\linewidth]{This figure shows the signal flow and the applied operations on the signal to perform "
 " adaptive feedback cancellation.}{FlowgraphAFC}\n"
 "\n"
 "In the forward path, the error signal \\emph{vE} is estimated by subtracting the estimated feedback signal of the output"
 " signal \\emph{vU} of the last iteration from the current input signal \\emph{vY} as shown "
 " in Equation~\\ref{eqn:predictionerror}."
 " This error signal is delayed a configurable number of taps, which is set using the configuration"
 " variable \\textbf{pred\\_err\\_delay} (\\emph{dG} in the figure)."
 " Subsequently a configurable amount of gain is applied to the delayed error signal to compute the output signal."
 " The amount of gain can be set using the configuration variable \\textbf{gains}.\n"
 "\\begin{equation}\n"
 "  e[k] = y[k] - \\mathbf{vWfull}^T[k-1] \\cdot \\mathbf{u}[k-1],\n"
 "\\label{eqn:predictionerror}\n"
 "\\end{equation}\n"
 "In order to compute the feedback signal (the second term in Equation~\\ref{eqn:predictionerror}),"
 " the output signal is delayed a configurable amount of taps, filtered using the last estimate of the"
 " feedback path \\emph{vWfull}."
 " The aforementioned delay is set using the configuration variable \\textbf{delay\\_w} (\\emph{dW} in the figure).\n"
 "\n"
 "In the closed loop identification of the feedback path, the LPC coefficients are used for prewhitening the input signal"
 " as well as the output signal."
 " The LPC coefficients are estimated in the plugin called LPC and saved in the AC space, which the prediction\\_error "
 "plugin reads in each iteration (\\emph{vLPC} in the figure)."
 " The name of the corresponding AC variable must be set using the configuration variable \\textbf{name\\_lpc}."
 " Furthermore, the dimensions of the LPC coefficients must be set using the configuration variable \\textbf{lpc\\_order}."
 " Prior to the prewhitening step, the input as well as the output signal are delayed the same amount of taps,"
 " which can be set by the configuration variable \\textbf{delay\\_d} (\\emph{dLp} in the figure).\n"
 "\n"
 "In the following step, the prewhitened error signal is computed as shown in Equation~\\ref{eqn:predictionerror}."
 " For this, the prewhitened output signal is filtered by the last estimate of the feedback path and subtracted from"
 " the prewhitened input signal.\n"
 "\n"
 "By using the prewhitened output signal and error signal, the NLMS algorithm re-estimates the coefficients of the"
 " feedback path in each iteration.\n"
 "\n"
 "The estimated filter coefficients are saved in an AC variable having the same name as the plugin in"
 " the current configuration."
 " The name of this AC variable can also be set differently by setting the configuration variable"
 " \\textbf{name\\_f} (\\emph{vWfull} in the figure).\n"
 "\n"
 "The estimation of the filter coefficients of the feedback path is performed using the update rule"
 " given as in the following:\n"
 "\\begin{equation}\n"
 "  \\mathbf{vWfull}[k] = \\mathbf{vWfull}[k-1] + \\rho \\cdot \\frac{\\mathbf{u}[k-1]}{(|\\mathbf{u}^T[k-1]"
 " \\cdot \\mathbf{u}[k-1]|+c)} \\cdot e[k],\n"
 "\\label{eqn:predictionupdate}\n"
 "\\end{equation}\n"
 "where $e$ is the prewhitened error (\\emph{EPrew}) signal, and $u$ (\\emph{vUBufPrew}) is the output signal."
 " The step size \\textbf{rho} ($\\rho$ in the equation) and the regularization parameter \\textbf{c} can be"
 " set using the corresponding configuration variables.\n"
 "\n"
 "The default values of the numeric configuration variables are optimized for a sampling rate of 16KHz."
 " They have to be adjusted for other sampling rates for optimal feedback cancellation performance. \n"
 )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
