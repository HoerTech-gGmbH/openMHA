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


#ifndef SET_RMS_HH
#define SET_RMS_HH


#include "mha_plugin.hh"


/**
 * Runtime configuration class for setting the channels of an output signal
 * (e.g. of some signal processing operation) to the RMS values of the original
 * input signal
 */
class Set_rms_cfg
{
public:
    /**
     * Constructor of the runtime configuration class
     *
     * @param ac_name_in
     *   Name of the AC variable containing the (exponentially averaged) RMS of
     *   the original input signal to be applied to the output signal (cannot
     *   be changed at runtime)
     * @param ac_name_out
     *   Name of the AC variable containing the (exponentially averaged) RMS of
     *   the output signal used for normalization (cannot be changed at runtime)
     */
    Set_rms_cfg(std::string ac_name_in,
                std::string ac_name_out);
    /**
     * Process function of the runtime configuration class (main signal
     * processing function). It sets the RMS of the output signal fragment to
     * the value specified by the parameter channel_rms_in
     *
     * @param signal
     *   Pointer to the current input signal fragment
     * @param channel_rms_in
     *   Contains a vector of RMS values for each channel of the original input
     *   signal to be applied to the output signal
     * @param channel_rms_out
     *   Contains a vector of RMS values for each channel of the output signal
     *   used for normalization
     * @return
     *   Pointer to the output signal fragment (with adjusted RMS)
     */
    mha_wave_t * process(mha_wave_t *signal,
                         mha_wave_t channel_rms_in,
                         mha_wave_t channel_rms_out);
private:
    /**
     *   Name of the AC variable containing the (exponentially averaged) RMS of
     *   the original input signal to be applied to the output signal
     */
    std::string ac_name_in_cfg;
    /**
     *   Name of the AC variable containing the (exponentially averaged) RMS of
     *   the output signal used for normalization
     */
    std::string ac_name_out_cfg;
};


/**
 * Plugin interface class for seting the channels of an output signal(e.g. of
 * some signal processing operation) to the RMS values of the original input
 * signal
 */
class Set_rms : public MHAPlugin::plugin_t<Set_rms_cfg>
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
    Set_rms(algo_comm_t &ac,
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
     *   Pointer to the output signal fragment (with adjusted RMS)
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
     * Name of the AC variable containing the (exponentially averaged) RMS of
     * the original input signal to be applied to the output signal
     */
    MHAParser::string_t ac_name_in;
    /**
     * Name of the AC variable containing the (exponentially averaged) RMS of
     * the output signal used for normalization
     */
    MHAParser::string_t ac_name_out;
    /**
     * Data member connecting an event emitter (i.e. configuration variable)
     * with a callback function of the plugin interface class
     */
    MHAEvents::patchbay_t<Set_rms> patchbay;
};


#endif  // SET_RMS_HH
