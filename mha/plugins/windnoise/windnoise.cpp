// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2019 HörTech gGmbH
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
 * This plugin detects channels that are affected by microphone windnoise
 * and replaces their signal with the signal from unaffected channels.
 * @todo: Cite Bitzer/Franz IWAENC paper
 */

#include "windnoise.hh"

namespace windnoise {
    cfg_t::cfg_t(const mhaconfig_t & signal_info,
                 bool UseChannel_LF_attenuation,
                 float tau_Lowpass,
                 float LowPassCutOffFrequency,
                 float LowPassFraction_dB,
                 float LowPassWindGain_dB)
        : UseChannel_LF_attenuation(UseChannel_LF_attenuation)
        , alpha_Lowpass(expf(signal_info.fragsize /
                        -(tau_Lowpass * signal_info.srate)))
        , FrequencyBinLowPass(ceilf(MHASignal::freq2bin(LowPassCutOffFrequency,
                                                        signal_info.fftlen,
                                                        signal_info.srate)))
        , LowPassFraction(powf(10, LowPassFraction_dB / 20))
        , LowPassWindGain(powf(10, LowPassWindGain_dB / 20))
        , PSD_Lowpass(signal_info.fftlen / 2 + 1, signal_info.channels)
        , powspec(signal_info.fftlen / 2 + 1, signal_info.channels)
    {}

    if_t::if_t(algo_comm_t ac,
               const std::string & chain_name,
               const std::string & algo_name)
        : MHAPlugin::plugin_t<cfg_t>("This plugin detects which microphone "
                                     "channels are affected by wind noise and\n"
                                     "replaces their signal with signal from "
                                     "unaffected channels.", ac)
        , detected_acname(algo_name + "_detected")
        , lowpass_quotient_acname(algo_name + "_lowpass_quotient")
    {
#define register_configuration_variable(v) insert_member(v); \
        patchbay.connect(&v.writeaccess, this, &if_t::update)

        register_configuration_variable(UseChannel_LF_attenuation);
        register_configuration_variable(tau_Lowpass);
        register_configuration_variable(LowPassCutOffFrequency);
        register_configuration_variable(LowPassFraction);
        register_configuration_variable(LowPassWindGain);
        register_configuration_variable(WindNoiseDetector);
        insert_member(detected);
        insert_member(lowpass_quotient);
    }

    void if_t::prepare(mhaconfig_t & signal_info)
    {
        if (signal_info.domain != MHA_SPECTRUM)
            throw MHA_Error(__FILE__, __LINE__,
                            "This plugin can only process spectrum signals.");
        detected.data.assign(signal_info.channels, 0);
        lowpass_quotient.data.assign(signal_info.channels, 0.0f);
        update();
        insert();
    }

    void if_t::update(void)
    {
        if (is_prepared())
            push_config(new cfg_t(input_cfg(),
                                  UseChannel_LF_attenuation.data,
                                  tau_Lowpass.data,
                                  LowPassCutOffFrequency.data,
                                  LowPassFraction.data,
                                  LowPassWindGain.data));
    }

    mha_spec_t * if_t::process(mha_spec_t * s_in)
    {
        poll_config();
        mha_spec_t * s_out = cfg->process(s_in, detected.data, lowpass_quotient.data);
        insert();
        return s_out;
    }

    mha_spec_t * cfg_t::process(mha_spec_t * signal,
                                std::vector<int> & detected,
                                std::vector<float> & lowpass_quotient)
    {
        update_PSD_Lowpass(signal);
        threshold_compare(detected, lowpass_quotient);
        compensation(signal, remapping(lowpass_quotient));
        return signal;
    }

    void cfg_t::update_PSD_Lowpass(const mha_spec_t * signal)
    {
        powspec.powspec(*signal);
        powspec *= (1-alpha_Lowpass);
        PSD_Lowpass *= alpha_Lowpass;
        PSD_Lowpass += powspec;
    }

    void cfg_t::threshold_compare(std::vector<int> & detected,
                                  std::vector<float> & lowpass_quotient)
    {
        auto factor_broadband = [&](unsigned k) {
            return (k>0 && k<PSD_Lowpass.num_frames-1) ? 1.0f : 0.5f;
        };
        auto factor_lowfreq = [&](unsigned k) {
            return (k<FrequencyBinLowPass) ? factor_broadband(k) : 0.0f;
        };
        for (unsigned ch = 0; ch < PSD_Lowpass.num_channels; ++ch) {
            float sum_lowfreq = 0, sum_broadband = 0;
            for (unsigned k = 0; k < PSD_Lowpass.num_frames; ++k) {
                sum_broadband += PSD_Lowpass.value(k,ch) * factor_broadband(k);
                sum_lowfreq += PSD_Lowpass.value(k,ch) * factor_lowfreq(k);
            }
            lowpass_quotient[ch] = sum_lowfreq / sum_broadband;
            detected[ch] = lowpass_quotient[ch] > LowPassFraction;
        }
    }

    int cfg_t::remapping (const std::vector<float> & lowpass_quotient)
    {   
        int best_signal_channel_index;
        //For multiple channels, checks to see if all lowpass quotients are equal
        if (std::adjacent_find( lowpass_quotient.begin(), lowpass_quotient.end(), 
                        std::not_equal_to<>() ) == lowpass_quotient.end() ){
            //If all are equal, -1 index will make next function do nothing
                best_signal_channel_index = -1;
        }
        else {
            //Saves index of lowest lowpass_quotient value
            best_signal_channel_index = std::min_element(lowpass_quotient.begin(), lowpass_quotient.end()) - 
                lowpass_quotient.begin();
        }
        return best_signal_channel_index;
    }
    void cfg_t::compensation(mha_spec_t * signal, int best_signal_channel_index)
    {
        if (best_signal_channel_index > -1){
            for (unsigned ch=0; ch < signal->num_channels; ++ch) {
            MHASignal::copy_channel (*signal,*signal, best_signal_channel_index, ch);
            }
        }
    }
}

MHAPLUGIN_CALLBACKS(windnoise,windnoise::if_t,spec,spec)

MHAPLUGIN_DOCUMENTATION                         \
(windnoise,
 "noise-suppression feature-extraction",
 "The windnoise plugins smoothes power spectra over time and detects wind\n"
 "noise in the audio by computing the ratio of sound energy at low\n"
 "frequencies with respect to the overall energy in the smoothed power \n"
 "spectra. The presence of wind noise is then detected when this ratio \n"
 "exceeds a configurable threshold criterium. This criterium as well as \n"
 "the cut-off frequency are configurable, and may need to be adapted to \n"
 "the microphones used. Users can inspect the monitor variable \n"
 "lowpass quotient which is also published as an AC variable for downstream\n"
 "plugins to check the value of the low frequency energy ratio and derive\n"
 "a suitable threshold for their acoustic configuration. \n"
 "\n"
 "As an example for setting a suitable threshold, a voice sample\n"
 "was generated using an AKG K-501 microphone with \n"
 "the waveform shown top of\n" 
 "Fig.\\ \\ref{fig:Windnoise_1}. \n"
 "\\MHAfigure{Top: Waveform of test file\n"
 "Middle: Low pass quotient generated from AC varialbes.\n"
 "Bottom: Detected AC variable with windnoise showing as peaks.\n"
 "}{Windnoise_1}\n"
 "Therefore, the value of LowPassFraction needs to be 0.96 which \n"
 "translates to -0.3 dB in decibel scale and this value is set using \n"
 "the configuration variable. With this set value, wind noise is \n"
 "correctly detected as can be seen bottom of Fig.\\ \\ref{fig:Windnoise_1}\n"
 "\n"
 "Artificial test signals can be used to test the technical feature \n"
 "extraction performed by the windnoise algorithm: The low-the pass quotient\n"
 "can be influenced by adding low-frequency or high-frequency sinusoids, and\n"
 "the smoothing over time of the power spectra can be observed by \n"
 "introducing transient changes into an otherwise constant signal \n"
 "and then observing the extracted features over time.\n"
 "An artificial sine wave is generated as shown top of \n"
 "Fig.\\ \\ref{fig:Windnoise_2}. A low frequency signal (40 Hz) is followed\n"
 "by a high frequency (1 kHz) which is above the threshold of 500 Hz.\n"
 "\\MHAfigure{Top: Waveform of sine wave generated with 40 Hz at start \n"
 "and 1 kHz later. Bottom: Low pass quotient AC variable.\n"
 "}{Windnoise_2}\n"
 "It can be observed in bottom of Fig.\\ \\ref{fig:Windnoise_2} how the \n"
 "Low pass ratio immediately drops the moment a high frequency signal comes\n"
 " in, thereby not reducing speech intelligibility."
 )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
