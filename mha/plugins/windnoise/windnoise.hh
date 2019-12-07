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

#include "mha_plugin.hh"

/// namespace for plugin windnoise which detects and cancels wind noise
namespace windnoise {
    /// Runtime config class for windnoise plugin.
    /// Computes power spectra of incoming STFT spectra, smoothes the power
    /// spectrum over time by low-pass filtering the intensities of each bin
    /// over time, then detects wind noise presence by comparing intensity
    /// at low frequency bins to broadband intensity. 
    class cfg_t {
    public:
        // The names of the runtime parameter members are taken from matlab code

        /// FIXME: documentation for UseChannel_LF_attenuation
        bool UseChannel_LF_attenuation = false;
        /// Filter coefficient for low-pass filtering each bin in the power
        /// spectrum with a first-order recursive low-pass filter.
        float alpha_Lowpass = 0;
        /// Only smoothed power spectrum bins < FrequencyBinLowPass are added
        /// to the low-pass intensity.
        unsigned FrequencyBinLowPass = 0;
        /// The wind noise detection threshold: We have wind noise if
        /// lowFreqIntensity / broadBandIntensity > LowPassFraction.
        float LowPassFraction = 1;
        /// FIXME: documentation for LowPassWindGain
        float LowPassWindGain = 1;

        // filter states

        /// The smoothed-over-time power spectrum.
        MHASignal::waveform_t PSD_Lowpass;

        /// Temporary storage for the power spectrum of the current input spectrum.
        /// Only needed to hold the newest squared magnitudes until they are filtered
        /// into PSD_Lowpass.
        MHASignal::waveform_t powspec;

        /// constructor translates configuration variables to runtime config
        cfg_t(const mhaconfig_t & signal_info,
              bool UseChannel_LF_attenuation,
              float tau_Lowpass,
              float LowPassCutOffFrequency,
              float LowPassFraction_dB,
              float LowPassWindGain_dB);

        /// Detect windnoise. FIXME: cancel it.
        /// The process method calls update_PSD_Lowpass and threshold_compare
        /// to do its work.
        /// @param [in,out] signal The current STFT spectrum.
        /// @param [out]    detected
        ///   This Method changes the elements of the vector but not its size.
        ///   Each element is set to 1 or 0, depending on windnoise being
        ///   detected in the corresponding audio channel.
        /// @param [out]    lowpass_quotient
        ///   This Method changes elements of the vector but not its size.
        ///   Each element is set to the ratio between intensity of the signal,
        ///   at low frequencies and overall intensity, in the corresponding
        ///   audio channel.
        /// @throw MHA_Error
        ///        if windnoise_indicators.size() != signal.num_channels.
        mha_spec_t * process(mha_spec_t * signal,
                             std::vector<int> & detected,
                             std::vector<float> & lowpass_quotient);

        /// Low-pass filters the power spectrum.
        void update_PSD_Lowpass(const mha_spec_t * signal);

        /// Wind noise detection by comparing low-frequency intensity with
        /// broadband intensity.
        /// @param [out]    detected
        ///   This Method changes the elements of the vector but not its size.
        ///   Each element is set to 1 or 0, depending on windnoise being
        ///   detected in the corresponding audio channel.
        /// @param [out]    lowpass_quotient
        ///   This Method changes elements of the vector but not its size.
        ///   Each element is set to the ratio between intensity of the signal,
        ///   at low frequencies and overall intensity, in the corresponding
        ///   audio channel.
        void threshold_compare(std::vector<int> & detected,
                                std::vector<float> & lowpass_quotient);

        int remapping (const std::vector<float> & lowpass_quotient);

        void compensation(mha_spec_t * signal, int best_signal_channel_index);
    };

    /// interface class for windnoise plugin
    class if_t : public MHAPlugin::plugin_t<cfg_t> {
        /// The Event connector
        MHAEvents::patchbay_t<if_t> patchbay;

    public:
        // The names of the following MHA parser variables are taken from the
        // reference matlab code.
        MHAParser::bool_t UseChannel_LF_attenuation = {
            "switch for channelwise LF-attenuation (yes=on, no=off)", "no"
        };
        MHAParser::float_t tau_Lowpass =  {
            "low-pass filter time constant for filtering spectral power / s",
            "1",
            "[0,1]"
        };
        MHAParser::float_t LowPassCutOffFrequency = {
            "cut-off frequency of the spectral weighting windnoise detector",
            "500",
            "[0,]"
        };
        MHAParser::float_t LowPassFraction = {
            "level difference threshold / dB between low and high band.\n"
            "If the level difference between low and high band exceeds this\n"
            "threshold in dB, then wind noise is detected.",
            "-1",
            "[-10,10]"
        };
        MHAParser::float_t LowPassWindGain =  {
            "Gain / dB applied to low frequency part when wind noise is\n"
            "detected and not compensated signal replacement",
            "-10",
            "[-100,0]"
        };
        MHAParser::kw_t WindNoiseDetector = {
            "type of windnoise detector to apply",
            "diffsum",
            "[psd diffsum msc none]"
        };
        MHAParser::vint_mon_t detected = {
            "windnoise detector state, "
            "one element (with value 0 or 1) per audio channel"
        };
        MHAParser::vfloat_mon_t lowpass_quotient = {
            "ratio of intensity at frequencies < LowPassCutOffFrequency\n"
            "to broadband intensity as quotient of intensities after smoothing\n"
            "the power spectrum with tau_Lowpass"
        };

        /// Name of AC variable mirroring the configuration monitor variable
        /// "detector".  Usually "windnoise_detected".
        const std::string detected_acname;

        /// Name of AC variable mirroring the configuration monitor variable
        /// "lowpass_quotient".  Usually "windnoise_lowpass_quotient".
        const std::string lowpass_quotient_acname;

        /// Constructor instantiates one windnoise plugin
        if_t(algo_comm_t ac,
             const std::string & chain_name,
             const std::string & algo_name);

        /// Prepare windnoise plugin for signal processing
        /// @param signal_info signal dimensions, not changed by this plugin
        void prepare(mhaconfig_t & signal_info) override;

        /// Nothing needs to be deallocated on release
        void release(void) override
        {}
        /// signal processing, delegates to cfg_t::process
        mha_spec_t * process(mha_spec_t * signal);

        /// update runtime config when configuration parameters have changed
        void update(void);

        /// inserts the windnoise detection vector into AC space
        void insert() {
            comm_var_t cv = {
                .data_type = MHA_AC_INT,
                .num_entries = unsigned(detected.data.size()),
                .stride = 1,
                .data = detected.data.size() ? &detected.data[0] : nullptr
            };
            ac.insert_var(ac.handle, detected_acname.c_str(), cv);
            cv.data_type = MHA_AC_FLOAT;
            cv.data = cv.num_entries ? &lowpass_quotient.data[0] : nullptr;
            ac.insert_var(ac.handle, lowpass_quotient_acname.c_str(), cv);
        }

        // make poll_config public for unit tests
        using MHAPlugin::plugin_t<cfg_t>::poll_config;
    };
}

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
