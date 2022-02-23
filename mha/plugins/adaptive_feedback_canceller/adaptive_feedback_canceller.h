// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2017 2018 2020 2021 HörTech gGmbH
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

#ifndef ADAPTIVE_FEEDBACK_CANCELLER_H
#define ADAPTIVE_FEEDBACK_CANCELLER_H

#include "mha_filter.hh"
#include "mha_plugin.hh"
#include "mhapluginloader.h"


class adaptive_feedback_canceller;

/** This is the runtime configuration, the main processing will be done in this class.
 *  During runtime AC variables are published by this class, mainly for debbugging purposes. */
class adaptive_feedback_canceller_config {

public:
    /** @param ac Algorithm communication variable space
     *  @param in_cfg MHA signal dimensions for this plugin
     *  @param adaptive_feedback_canceller Pointer to plugin interface instance,
     *                                     used to extract user configuration values. */
    adaptive_feedback_canceller_config(algo_comm_t &ac, const mhaconfig_t in_cfg, adaptive_feedback_canceller *afc);
    ~adaptive_feedback_canceller_config();
    /** The process() method contains the actual AFC algorithm, the output is stored in LSsig_output.
     *  @param MICsig The microphone signal which contains the target signal + the feedback signal.
     *                The input signal is not altered in place.
     *  @return A pointer to LSsig_output which contains the loudspeaker signal that is actually channeled to the output. */
    mha_wave_t* process(mha_wave_t* MICsig);
    /** Insert all AC-variables into the AC-space. */
    void insert();

private:
    /** Length of the estimated filter */
    const unsigned int ntaps;
    /** Length of a block in samples (fragsize) */
    const unsigned int frames;
    /** Number of channels. This plugin assumes that the number of input channels is equal to the
     *  number of output channels. Each input is paired with an output and one pair is called a
     *  channel. This is necessary because the AFC needs input and output information to work.
     */
    const unsigned int channels;

    /** Number of blocks after startup where the feedback filter is not updated */
    const int n_no_update_;
    /** Index counting no-update-blocks up to n_no_update_ to check in process() whether a
     *  filter update shall be performed or not.
     */
    int no_update_count;

    /** Fragsize for computing @ref delay_roundtrip and @ref delay_update, defaults to the MHA's fragsize.
     *  It is assumed that the soundcard's and the MHA's fragsize are equal. In special cases, the user
     *  can set this variable to an individual value.
     */
    const mha_real_t fragsize;

    /** Normalized stepsize of the NLMS-Algorithm */
    const mha_real_t stepsize;
    /** Minimum constant to prevent division by zero in the NLMS-Algorithm */
    const mha_real_t min_const;

    /** Copy of error signal that is channeled into the forward path processing */
    MHASignal::waveform_t forward_sig;
    /** Signal consisting of zeros, it is only used to initialize the loudspeaker signal (LSsig) for the first
     *  iteration (before forward_path_proc.process() is called for the first time).
     */
    MHASignal::waveform_t LSsig_initializer;
    /** Pointer to the loudspeaker (LS) signal. The destination of this pointer will be altered by
     *  the plugin loaded by the pluginloader in the forward path processing.
     */
    mha_wave_t* LSsig;
    /* Copy of LSsig* which is returned by the process() method */
    MHASignal::waveform_t LSsig_output;
    /** Delay line for additional decorrelation in the forward path */
    MHASignal::delay_t delay_forward_path;
    /** Pluginloader to load the plugins that represent the hearing aid processing in the forward path */
    MHAParser::mhapluginloader_t& forward_path_proc;

    /** Delay line equal to the measured roundtrip latency - @ref fragsize. The roundtrip latency
     *  describes the timespan between playing back and receiving the same signal, including
     *  delays induced by soundcard buffering and the transducers.
     *  Can be measured via jack_iodelay.
     *  It is used on the loudspeaker signal LSsig before the filtering with FBfilter_estim,
     *  to achieve about the same temporal alignment to @ref MICsig as the alignment of the target
     *  signal and the true feedback signal.
     */
    MHASignal::delay_t delay_roundtrip;
    /* Delay line equal to @ref delay_roundtrip + @ref fragsize - 1. It is used on the loudspeaker
     * signal white_LSsig before using it to update the filter estimation.
     */
    MHASignal::delay_t delay_update;
    /* Vector of estimated feedback filters, one filter per audio channel.
     * (See @ref channels for more information)
     */
    std::vector<MHAFilter::filter_t> FBfilter_estim;
    /* AC variable publishing the current estimated feedback filter coefficients if @ref debug_mode == true */
    MHA_AC::waveform_t FBfilter_estim_ac;
    /* Filtered loudspeaker signal (via estimated AFC-filter) that represents the estimated feedback signal */
    MHASignal::waveform_t FBsig_estim;
    /* Error signal (difference of microphone signal and estimated feedback signal) */
    MHASignal::waveform_t ERRsig;
    /* AC variable publishing the current error signal if @ref debug_mode == true */
    MHA_AC::waveform_t ERRsig_ac;

    /* This bool determines whether a prewhitening of the signals using LPC shall be performed prior
     * to the filter adaption step */
    bool use_lpc_decorr;
    /* LPC coefficients that were calculated from the previous block of the error signal */
    std::vector<MHAFilter::filter_t> lpc_filter;
    /** Loudspeaker signal, whitened by filtering with LPC coefficients. If @ref use_lpc_decorr
     * is false then this is just a copy of @ref LSsig before the delays.
     */
    MHASignal::waveform_t white_LSsig;
    /** Single sample (for each channel) of white_LSsig that will be written to
     * rb_white_LSsig next. For the filter estimation we have to use a ringbuffer with the
     * size of @ref filter_length to buffer @ref white_LSsig. For each filter tap we have to update
     * the buffer and it is only possible to update a ringbuffer with a whole waveform_t.
     * That is why we have to use this single sample variable.
     */
    MHASignal::waveform_t white_LSsig_smpl;
    /** Ringbuffer containing the values used to determine the power of @ref white_LSsig in each
     *  channel (see @ref channels) and update @ref FBfilter_estim. The ringbuffer has a size equal
     *  to @ref filter_length.
     */
    MHASignal::ringbuffer_t rb_white_LSsig;
    /* Vector containing the power of rb_white_LSsig for each channel */
    std::vector<mha_real_t> current_power;
    /* Microphone signal, whitened by filtering with LPC coefficients. If @ref use_lpc_decorr
     * is false then this is just a copy of @ref MICsig.
     */
    MHASignal::waveform_t white_MICsig;
    /* Signal that resulted after filtering the whitened loudspeaker signal with the estimated
     * feedback filter */
    MHASignal::waveform_t white_FBsig_estim;
    /* whitened error signal (difference of white_MICsig and white_FBsig_estim) */
    MHASignal::waveform_t white_ERRsig;
    /** When executing the code for debug purposes this flag can be set to true in order to capture
     *  variable states and provide them as AC variables to the user. The following variables are
     *  captured: @ref FBfilter_estim_ac, @ref ERRsig_ac, @ref current_power_ac, @ref estim_err_ac
     */
    bool debug_mode;
    /* AC-variable publishing the state of @ref current_power if @ref debug_mode == true */
    MHA_AC::waveform_t current_power_ac;
    /** AC-variable publishing the state of 'estim_err' if @ref debug_mode == true
     *  'estim_err' is a variable local to this plugin's process() method. It represents a part of
     *  the NLMS equation, namely the @ref stepsize normalized by @ref current_power times @ref ERRsig.
     */
    MHA_AC::waveform_t estim_err_ac;
};

class adaptive_feedback_canceller : public MHAPlugin::plugin_t<adaptive_feedback_canceller_config> {

public:
    adaptive_feedback_canceller(MHA_AC::algo_comm_t & ac,
                                const std::string & configured_name);
    ~adaptive_feedback_canceller();
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
    void release();

    /* declare MHAParser variables here */

    /* normalized stepsize of the NLMS algorithm */
    MHAParser::float_t stepsize;
    /* minimum constant that prevents division by zero in the NLMS algorithm */
    MHAParser::float_t min_const;
    /* length of the estimated filter */
    MHAParser::int_t filter_length;
    /** Plugin loader that loads the plugin needed to simulate the hearing aid in the
     *  forward processing path.
     */
    MHAParser::mhapluginloader_t plugloader;
    /** Fragsize for computing @ref adaptive_feedback_canceller::delay_roundtrip and @ref
     *  adaptive_feedback_canceller::delay_update, defaults to the MHA's fragsize.
     *  It is assumed that the soundcard's and the MHA's fragsize are equal. In special cases, the user
     *  can set this variable to an individual value.
     */
    MHAParser::int_t fragsize;
    /** The roundtrip latency
     *  describes the timespan between playing back and receiving the same signal, including
     *  delays induced by soundcard buffering and the transducers.
     *  Can be measured via jack_iodelay. It is used to compute @ref delay_roundtrip and @ref delay_update
     *  for computations in the backward path.
     */
    MHAParser::vint_t measured_roundtrip_latency;
    /** Boolean defining whether decorrelation using LPC-filtering of
     *  microphone and loudspeaker signal should be performed.
     *  NOTE: The algorithm did not yet implement the decorrelation via LPC coefficients.
     *  If you set this variable to 'true' in the current state, the plugin will not work.
     */
    MHAParser::bool_t use_lpc_decorr;
    /* filter order of LPC filter */
    MHAParser::int_t lpc_order;
    /* Forward path processing delay (for decorrelation, at least 2 * filter_length) */
    MHAParser::vint_t delay_forward_path;
    /* Number of process() callbacks after start where no filter adaption is performed */
    MHAParser::int_t blocks_no_update;
    /** @ref debug_mode == true provides more variables in the AC-space that are useful for debugging,
     *  including @ref FBfilter_estim_ac, @ref ERRsig_ac, @ref current_power_ac, @ref estim_err_ac
     */
    MHAParser::bool_t debug_mode;

private:
    void update_cfg();

    /* patch bay for connecting configuration parser
       events with local member functions: */
    MHAEvents::patchbay_t<adaptive_feedback_canceller> patchbay;

};

#endif /* ADAPTIVE_FEEDBACK_CANCELLER_H */

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
