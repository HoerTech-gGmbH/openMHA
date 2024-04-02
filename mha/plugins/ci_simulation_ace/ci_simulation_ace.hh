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


#ifndef CI_SIMULATION_ACE_HH
#define CI_SIMULATION_ACE_HH

#include <random>

#include "mha_plugin.hh"


/**
 * Constant describing the required number of input audio channels per side
 */
extern const unsigned int CHANNELS;
/**
 * Constant describing the required FFT length / bins
 */
extern const unsigned int FFTLEN;
/**
 * Constant describing the required sampling rate / Hz
 */
extern const mha_real_t SRATE;
/**
 * Constant describing the required total number of electrodes per side
 */
extern const unsigned int M_ELECTRODES;
/**
 * Constant vector describing the required FFT bin indices for composing the
 * filterbank bands
 */
extern const std::vector<unsigned int> BIN_INDICES;
/**
 * Constant vector describing the required weights for the filterbank bands
 */
extern const std::vector<mha_real_t> WEIGHTS;


/**
 * Runtime configuration class for generating an electrodogram from an audio
 * signal, using a stimulation strategy similar to a typical ACE (advanced
 * combination encoder, n-of-m) coding strategy with 22 channels
 */
class Ci_simulation_ace_cfg
{
public:
    /**
     * Constructor of the runtime configuration class
     *
     * @param n_electrodes
     *   Number of active electrodes (cannot be changed at runtime)
     * @param compression_coefficient
     *   Compression coefficient of the loudness growth function
     * @param base_level
     *   Base level of the input (acoustic) dynamic range / Pa
     * @param saturation_level
     *   Saturation level of the input (acoustic) dynamic range / Pa
     * @param threshold_level
     *   Vector containing the threshold level of the output (electric) dynamic
     *   range for each electrode / CU
     * @param comfort_level
     *   Vector containing the comfort level of the output (electric) dynamic
     *   range for each electrode / CU
     * @param disabled_electrodes
     *   Vector containing the indices of any disabled electrodes
     * @param stimulation_order
     *   Electrode stimulation order
     * @param random_number_generator
     *   Random number generator for randomization of stimulation order
     */
    Ci_simulation_ace_cfg(unsigned int n_electrodes,
                          mha_real_t compression_coefficient,
                          mha_real_t base_level,
                          mha_real_t saturation_level,
                          std::vector<mha_real_t> threshold_level,
                          std::vector<mha_real_t> comfort_level,
                          std::vector<int> disabled_electrodes,
                          unsigned int stimulation_order,
                          std::default_random_engine random_number_generator);
    /**
     * Process function of the runtime configuration class (main signal
     * processing function). It leaves the input signal fragment unchanged; its
     * purpose is to compute the AC variable electrodogram_ac for further
     * processing in the plugin chain
     *
     * @param signal
     *   Pointer to the current input signal fragment
     * @param electrodogram_ac
     *   AC variable containing a pointer to the electrodogram
     * @return
     *   Pointer to the (unchanged) output signal fragment
     */
    mha_spec_t * process(mha_spec_t *signal,
                         MHA_AC::waveform_t *electrodogram_ac);
private:
    /**
     * Number of active electrodes (cannot be changed at runtime)
     */
    unsigned int n_electrodes_cfg;
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
     * range for each electrode / CU
     */
    std::vector<mha_real_t> threshold_level_cfg;
    /**
     * Vector containing the comfort level of the output (electric) dynamic
     * range for each electrode / CU
     */
    std::vector<mha_real_t> comfort_level_cfg;
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
};


/**
 * Plugin interface class for generating an electrodogram from an audio signal,
 * using a stimulation strategy similar to a typical ACE (advanced combination
 * encoder, n-of-m) coding strategy with 22 channels
 */
class Ci_simulation_ace : public MHAPlugin::plugin_t<Ci_simulation_ace_cfg>
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
    Ci_simulation_ace(algo_comm_t &ac,
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
    mha_spec_t * process(mha_spec_t *signal);
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
     * Number of active electrodes (cannot be changed at runtime)
     */
    MHAParser::int_t n_electrodes;
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
     * range for each electrode / CU
     */
    MHAParser::vfloat_t threshold_level;
    /**
     * Vector containing the comfort level of the output (electric) dynamic
     * range for each electrode / CU
     */
    MHAParser::vfloat_t comfort_level;
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
    MHAEvents::patchbay_t<Ci_simulation_ace> patchbay;
    /**
     * Random number generator for randomization of stimulation order
     */
    std::default_random_engine random_number_generator;
    /**
     * AC variable containing a pointer to the electrodogram (containing a
     * matrix with dimensions m x n, where m = total number of electrodes and
     * n = number of active electrodes, and where the order of the n active
     * electrodes corrresponds to the temporal sequence of their activation)
     */
    MHA_AC::waveform_t *electrodogram_ac;
};


#endif  // CI_SIMULATION_ACE_HH
