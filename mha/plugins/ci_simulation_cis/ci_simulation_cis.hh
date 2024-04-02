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


#ifndef CI_SIMULATION_CIS_HH
#define CI_SIMULATION_CIS_HH

#include <random>

#include "mha_plugin.hh"


class Ci_simulation_cis;


/**
 * Constant describing the required number of input audio channels per side
 */
extern const unsigned int CHANNELS;
/**
 * Constant describing the required sampling rate / Hz
 */
extern const mha_real_t SRATE;
/**
 * Constant describing the required total number of electrodes per side
 */
extern const unsigned int M_ELECTRODES;


/**
 * Runtime configuration class for generating an electrodogram from an audio
 * signal, using a stimulation strategy similar to a typical CIS (continuous
 * interleaved sampling) coding strategy with 12 channels
 */
class Ci_simulation_cis_cfg
{
public:
    /**
     * Constructor of the runtime configuration class
     *
     * @param weights
     *   Vector containing the weights for the analysis filterbank bands
     * @param compression_coefficient
     *   Compression coefficient of the loudness growth function
     * @param base_level
     *   Base level of the input (acoustic) dynamic range / Pa
     * @param saturation_level
     *   Saturation level of the input (acoustic) dynamic range / Pa
     * @param threshold_level
     *   Vector containing the threshold level of the output (electric) dynamic
     *   range for each electrode / cu
     * @param maximum_comfortable_level
     *   Vector containing the maximum comfortable level of the output
     *   (electric) dynamicrange for each electrode / cu
     * @param disabled_electrodes
     *   Vector containing the indices of any disabled electrodes
     * @param stimulation_order
     *   Electrode stimulation order
     * @param random_number_generator
     *   Random number generator for randomization of stimulation order
     * @param Ci_simulation_cis
     *   Pointer to the current instance of the plugin interface class
     */
    Ci_simulation_cis_cfg(std::vector<mha_real_t> weights,
                          mha_real_t compression_coefficient,
                          mha_real_t base_level,
                          mha_real_t saturation_level,
                          std::vector<mha_real_t> threshold_level,
                          std::vector<mha_real_t> maximum_comfortable_level,
                          std::vector<int> disabled_electrodes,
                          unsigned int stimulation_order,
                          std::default_random_engine random_number_generator,
                          Ci_simulation_cis *Ci_simulation_cis);
    /**
     * Destructor of the runtime configuration class
     */
    ~Ci_simulation_cis_cfg();
    /**
     * Process function of the runtime configuration class (main signal
     * processing function). It takes an input signal fragment as well as the
     * real and imaginary outputs of a gammatone filterbank with m bands as a
     * parameter, resulting in a total of 1 + m * 2 input signal channels. It
     * returns the input signal fragment unchanged; the purpose of the function
     * is to compute the AC variable electrodogram_ac from the gammatone
     * filterbank output for further processing in the plugin chain
     *
     * @param signal
     *   Pointer to the current input signal fragment + gammatone filterbank
     *   output
     * @param Ci_simulation_cis
     *   Pointer to the current instance of the plugin interface class
     * @param electrodogram_ac
     *   AC variable containing a pointer to the electrodogram
     * @return
     *   Pointer to the (unchanged) output signal fragment
     */
    mha_wave_t * process(mha_wave_t *signal,
                         Ci_simulation_cis *Ci_simulation_cis,
                         MHA_AC::waveform_t *electrodogram_ac);
private:
    /**
     * Vector containing the weights for the analysis filterbank bands
     */
    std::vector<mha_real_t> weights_cfg;
    /**
     * Compression coefficient of the loudness growth function
     */
    mha_real_t compression_coefficient_cfg;
    /**
     * Base level of the input (acoustic) dynamic range
     */
    mha_real_t base_level_cfg;
    /**
     * Saturation level of the input (acoustic) dynamic range
     */
    mha_real_t saturation_level_cfg;
    /**
     * Vector containing the threshold level of the output (electric) dynamic
     * range for each electrode
     */
    std::vector<mha_real_t> threshold_level_cfg;
    /**
     * Vector containing the maximum comfortable level of the output
     * (electric) dynamicrange for each electrode
     */
    std::vector<mha_real_t> maximum_comfortable_level_cfg;
    /**
     * Vector containing the indices of any disabled electrodes
     */
    std::vector<int> disabled_electrodes_cfg;
    /**
     * Electrode stimulation order
     */
    unsigned int stimulation_order_cfg;
    /**
     * Random number generator for randomization of stimulation order
     */
    std::default_random_engine random_number_generator_cfg;
    /**
     * Pointer to the current output signal fragment
     */
    MHASignal::waveform_t *signal_out;
};


/**
 * Plugin interface class for generating an electrodogram from an audio signal,
 * using a stimulation strategy similar to a typical CIS (continuous interleaved
 * sampling) coding strategy with 12 channels
 */
class Ci_simulation_cis : public MHAPlugin::plugin_t<Ci_simulation_cis_cfg>
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
    Ci_simulation_cis(algo_comm_t &ac,
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
     * Vector containing the weights for the analysis filterbank bands
     */
    MHAParser::vfloat_t weights;
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
     * Vector containig the indices of any disabled electrodes
     */
    MHAParser::vint_t disabled_electrodes;
    /**
     * Electrode stimulation order
     */
    MHAParser::kw_t stimulation_order;
    /**
     * Seed for randomization of stimulation order
     */
    MHAParser::int_t randomization_seed;
    /**
     * Data member connecting an event emitter (i.e. configuration variable)
     * with a callback function of the plugin interface class
     */
    MHAEvents::patchbay_t<Ci_simulation_cis> patchbay;
    /**
     * Random number generator for randomization of stimulation order
     */
    std::default_random_engine random_number_generator;
    /**
     * AC variable containing a pointer to the electrodogram (containing a
     * matrix with dimensions m x m, where m = total number of electrodes =
     * number of active electrodes, and where the order of the active
     * electrodes corrresponds to the temporal sequence of their activation)
     */
    MHA_AC::waveform_t *electrodogram_ac;
};


#endif  // CI_SIMULATION_CIS_HH
