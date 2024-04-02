// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2024 Hörzentrum Oldenburg gGmbH
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


#ifndef CI_AURALIZATION_CIS_HH
#define CI_AURALIZATION_CIS_HH


#include "mha_plugin.hh"


class Ci_auralization_cis;


/**
 * Runtime configuration class for generating an auralized audio signal from
 * the specified AC variable, using a stimulation strategy similar to a typical
 * CIS (continuous interleaved sampling) coding strategy with 12 channels
 */
class Ci_auralization_cis_cfg
{
public:
    /**
     * Constructor of the runtime configuration class
     *
     * @param ac_name
     *   Name of the AC variable containing the electrodogram (cannot be
     *   changed at runtime)
     * @param compression_coefficient
     *   Compression coefficient of the loudness growth function
     * @param base_level
     *   Base level of the input (acoustic) dynamic range / Pa
     * @param saturation_level
     *   Saturation level of the input (acoustic) dynamic range / pa
     * @param threshold_level
     *   Vector containing the threshold level of the output (electric) dynamic
     *   range for each electrode / cu
     * @param maximum_comfortable_level
     *   Vector containing the maximum comfortable level of the output
     *   (electric) dynamic range for each electrode / cu
     * @param electrode_distance
     *   Distance of the electrodes / m
     * @param lambda
     *   Length constant of exponential spread of excitation / m
     * @param phase_duration
     *   Duration of one phase of a biphasic pulse / s
     * @param interphase_gap
     *   Duration of the gap between the phases of a biphasic pulse / s
     * @param phase_order
     *   Order of the phases of a biphasic pulse
     * @param electrodogram
     *   Electrodogram (contains a matrix with dimensions m x m, where
     *   m = total number of electrodes = number of active electrodes, and
     *   where the order of the active electrodes corrresponds to the temporal
     *   sequence of their activation)
     * @param n_electrodes
     *   Number of active electrodes
     * @param m_electrodes
     *   Total number of electrodes per side
     * @param Ci_auralization_cis
     *   Pointer to the current instance of the plugin interface class
     */
    Ci_auralization_cis_cfg(std::string ac_name,
                            mha_real_t steepness,
                            mha_real_t base_level,
                            mha_real_t saturation_level,
                            std::vector<mha_real_t> threshold_level,
                            std::vector<mha_real_t> comfort_level,
                            mha_real_t electrode_distance,
                            mha_real_t lambda,
                            mha_real_t phase_duration,
                            mha_real_t interphase_gap,
                            unsigned int phase_order,
                            mha_wave_t electrodogram,
                            unsigned int n_electrodes,
                            unsigned int m_electrodes,
                            Ci_auralization_cis *Ci_auralization_cis);
    /**
     * Process function of the runtime configuration class (main signal
     * processing function). It leaves the input signal fragment unchanged; its
     * purpose is to compute the AC variable stimulation_signal_ac, which
     * contains all the electrode-specific information (including concrete time
     * specifications) necessary for auralization, for further processing in
     * the plugin chain
     *
     * @param signal
     *   Pointer to the current input signal fragment
     * @param Ci_auralization_cis
     *   Pointer to the current instance of the plugin interface class
     * @param stimulation_signal_ac
     *   AC variable containing a pointer to the stimulation signal
     * @return
     *   Pointer to the (unchanged) output signal fragment
     */
    mha_wave_t * process(mha_wave_t *signal,
                         Ci_auralization_cis *Ci_auralization_cis,
                         MHA_AC::waveform_t *stimulation_signal_ac);
private:
    /**
     * Name of the AC variable containing the electrodogram (cannot be changed
     * at runtime)
     */
    std::string ac_name_cfg;
    /**
     * Compression coefficient of the loudness growth function
     */
    mha_real_t compression_coefficient_cfg;
    /**
     * Base level of the input (acoustic) dynamic range / Pa
     */
    mha_real_t base_level_cfg;
    /**
     * Saturation level of the input (acoustic) dynamic range / Pa
     */
    mha_real_t saturation_level_cfg;
    /**
     * Vector containing the threshold level of the output (electric) dynamic
     * range for each electrode / cu
     */
    std::vector<mha_real_t> threshold_level_cfg;
    /**
     * Vector containing the maximum comfortable level of the output (electric)
     * dynamic range for each electrode / cu
     */
    std::vector<mha_real_t> maximum_comfortable_level_cfg;
    /**
     * Distance of the electrodes / m
     */
    mha_real_t electrode_distance_cfg;
    /**
     * Length constant of exponential spread of excitation / m
     */
    mha_real_t lambda_cfg;
    /**
     * Duration of one phase of a biphasic pulse / s
     */
    mha_real_t phase_duration_cfg;
    /**
     * Duration of the gap between the phases of a biphasic pulse / s
     */
    mha_real_t interphase_gap_cfg;
    /**
     * Order of the phases of a biphasic pulse
     */
    unsigned int phase_order_cfg;
    /**
     *   Electrodogram (contains a matrix with dimensions m x m, where
     *   m = total number of electrodes = number of active electrodes, and
     *   where the order of the active electrodes corrresponds to the temporal
     *   sequence of their activation)
     */
    mha_wave_t electrodogram_cfg;
    /**
     * Number of active electrodes
     */
    unsigned int n_electrodes_cfg;
    /**
     * Total number of electrodes per side
     */
    unsigned int m_electrodes_cfg;
};


/**
 * Plugin interface class for generating an auralized audio signal from the
 * specified AC variable, using a stimulation strategy similar to a typical CIS
 * (continuous interleaved sampling) coding strategy with 12 channels
 */
class Ci_auralization_cis : public MHAPlugin::plugin_t<Ci_auralization_cis_cfg>
{
public:
    /**
     * Constructor of the plugin interface class
     *
     * @param ac
     *   Reference to the processing chain structure
     * @param algo_name
     *   Reference to the algorithm name
     */
    Ci_auralization_cis(algo_comm_t &ac,
                        const std::string &algo_name);
    /**
     * Prepare function of the plugin interface class
     *
     * @param signal_info
     *   Reference to the prepare configuration structure
     */
    void prepare(mhaconfig_t &signal_info);
    /**
     * Process function of the plugin interface class
     *
     * @param signal
     *   Pointer to the current input signal fragment
     * @return
     *   Pointer to the (unchanged) output signal fragment
     */
    mha_wave_t * process(mha_wave_t *signal);
    /**
     * Release function of the plugin interface class
     */
    void release();
private:
    /**
     * Runtime configuration update function of the plugin interface class
     */
    void update_cfg();
    /**
     * Name of the algorithm within the plugin chain
     */
    std::string algo_name;
    /**
     * Name of the AC variable containing the electrodogram (cannot be changed
     * at runtime)
     */
    MHAParser::string_t ac_name;
    /**
     * Compression coefficient of the loudness growth function
     */
    MHAParser::float_t compression_coefficient;
    /**
     * Base level of the input (acoustic) dynamic range / Pa
     */
    MHAParser::float_t base_level;
    /**
     * Saturation level of the input (acoustic) dynamic range / Pa
     */
    MHAParser::float_t saturation_level;
    /**
     * Vector containing the threshold level of the output (electric) dynamic
     * range for each electrode / cu
     */
    MHAParser::vfloat_t threshold_level;
    /**
     * Vector containing the maximum comfortable level of the output (electric)
     * dynamic range for each electrode / cu
     */
    MHAParser::vfloat_t maximum_comfortable_level;
    /**
     * Distance of the electrodes / m
     */
    MHAParser::float_t electrode_distance;
    /**
     * Length constant of exponential spread of excitation / m
     */
    MHAParser::float_t lambda;
    /**
     * Duration of one phase of a biphasic pulse / s
     */
    MHAParser::float_t phase_duration;
    /**
     * Duration of the gap between the phases of a biphasic pulse / s
     */
    MHAParser::float_t interphase_gap;
    /**
     * Order of the phases of a biphasic pulse
     */
    MHAParser::kw_t phase_order;
    /**
     * Data member connecting an event emitter (i.e. configuration variable)
     * with a callback function of the plugin interface class
     */
    MHAEvents::patchbay_t<Ci_auralization_cis> patchbay;
    /**
     * Electrodogram (contains a matrix with dimensions m x m, where m = total
     * number of electrodes n = number of active electrodes, and where the
     * order of the n active electrodes corrrespondes to the temporal sequence
     * of their activation)
     */
    mha_wave_t electrodogram;
    /**
     * Number of active electrodes
     */
    unsigned int n_electrodes;
    /**
     * Total number of electrodes per side
     */
    unsigned int m_electrodes;
    /**
     * AC variable containing a pointer to the stimulation signal (containing a
     * matrix with dimensions m x fragsize, where m = total number of
     * electrodes and fragsize = fragment size / samples, representing the
     * actual electrical stimulation per electrode over time for the current
     * fragment)
     */
    MHA_AC::waveform_t *stimulation_signal_ac;
};


#endif  // CI_AURALIZATION_CIS_HH
