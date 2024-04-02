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


#ifndef GET_RMS_HH
#define GET_RMS_HH


#include "mha_plugin.hh"
#include "mha_filter.hh"


class Get_rms;


/**
 * Runtime configuration class for computing the exponentially averaged RMS of
 * the channels of an input signal and storing it in an AC variable
 */
class Get_rms_cfg
{
public:
    /**
     * Constructor of the runtime configuration class
     *
     * @param tau
     *   Time constant for the exponential average / s
     * @param Get_rms
     *   Pointer to the current instance of the plugin interface class
     * @param rms_prev
     *   Vector of RMS values for each channel of the previous input signal
     *   fragment
     */
    Get_rms_cfg(mha_real_t tau,
                Get_rms *Get_rms,
                std::vector<mha_real_t> rms_prev);
    /**
     * Process function of the runtime configuration class (main signal
     * processing function). It leaves the input signal fragment unchanged; its
     * purpose is to compute the AC variable rms_ac for further processing in
     * the plugin chain
     *
     * @param signal
     *   Pointer to the current input signal fragment
     * @param Get_rms
     *   Pointer to the current instance of the plugin interface class
     * @param rms_ac
     *   AC variable containing a pointer to the RMS values for each channel
     *   of the current input signal fragment
     * @return
     *   Pointer to the (unchanged) output signal fragment
     */
    mha_wave_t * process(mha_wave_t *signal,
                         Get_rms *Get_rms,
                         MHA_AC::waveform_t *rms_ac);
private:
    /**
     * Time constant for the exponential average / s
     */
    mha_real_t tau_cfg;
    /**
     * Vector of RMS values for each channel of the previous input signal
     * fragment
     */
    std::vector<mha_real_t> rms_prev_cfg;
};


/**
 * Plugin interface class for computing the exponentially averaged RMS of the
 * channels of an input signal and storing it in an AC variable
 */
class Get_rms : public MHAPlugin::plugin_t<Get_rms_cfg>
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
    Get_rms(algo_comm_t &ac,
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
     * Time constant for the exponential average / s
     */
    MHAParser::float_t tau;
    /**
     * Data member connecting an event emitter (i.e. configuration variable)
     * with a callback function of the plugin interface class
     */
    MHAEvents::patchbay_t<Get_rms> patchbay;
    /**
     * Vector of RMS values for each channel of the previous input signal
     * fragment
     */
    std::vector<mha_real_t> rms_prev;
    /**
     * AC variable containing a pointer to the RMS values for each channel of
     * the current input signal fragment
     */
    MHA_AC::waveform_t *rms_ac;
};


#endif  // GET_RMS_HH
