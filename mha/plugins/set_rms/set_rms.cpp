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


#include "set_rms.hh"


Set_rms_cfg::Set_rms_cfg(std::string ac_name_in,
                         std::string ac_name_out)
    : ac_name_in_cfg(ac_name_in),
      ac_name_out_cfg(ac_name_out)
{
    if (ac_name_in_cfg == "")
    {
        throw MHA_Error(__FILE__, __LINE__,
                        "The name of the AC variable containing the RMS of the "
                        "original input signal must be specified.");
    }

    if (ac_name_out_cfg == "")
    {
        throw MHA_Error(__FILE__, __LINE__,
                        "The name of the AC variable containing the RMS of the "
                        "output signal must be specified.");
    }
}


// Process function of the runtime configuration class (main signal processing
// function). It sets the RMS of the output signal fragment to the value
// specified by the parameter channel_rms_in:

mha_wave_t * Set_rms_cfg::process(mha_wave_t *signal,
                                  mha_wave_t channel_rms_in,
                                  mha_wave_t channel_rms_out)
{
    // Loop over all channels (= electrodes):
    for (unsigned int channel = 0; channel < signal->num_channels; channel++)
    {
        // Loop over all frames:
        for (unsigned int frame = 0; frame < signal->num_frames; frame++)
        {
            // Normalize the RMS by dividing each sample by the output RMS
            // (adding a small constant to the divisor to exclude division by
            // zero) and multiply each sample by the input RMS:
            value(signal, frame, channel) *= value(channel_rms_in, 0, channel)/
                (value(channel_rms_out, 0, channel) + std::numeric_limits<mha_real_t>::min());
        }
    }

    // Return the output signal fragment (with adjusted RMS):
    return signal;
}


Set_rms::Set_rms(algo_comm_t &ac,
                 const std::string &algo_name)
    : MHAPlugin::plugin_t<Set_rms_cfg>("Set RMS", ac),
      algo_name(algo_name),
      ac_name_in("Name of the AC variable containing the (exponentially "
                 "averaged) RMS of the original input signal to be applied to "
                 "the output signal (cannot be changed at runtime)",
                 ""),
      ac_name_out("Name of the AC variable containing the (exponentially "
                  "averaged) RMS of the output signal used for normalization "
                  "(cannot be changed at runtime)",
                  "")
{
    insert_item("ac_name_in", &ac_name_in);
    insert_item("ac_name_out", &ac_name_out);

    patchbay.connect(&ac_name_in.writeaccess, this, &Set_rms::update_cfg);
    patchbay.connect(&ac_name_out.writeaccess, this, &Set_rms::update_cfg);
}


void Set_rms::prepare(mhaconfig_t &signal_info)
{
    if (signal_info.domain != MHA_WAVEFORM)
    {
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin can only process waveform signals.");
    }

    update_cfg();

    // Lock the configuration variables against write access:
    ac_name_in.setlock(true);
    ac_name_out.setlock(true);
}


mha_wave_t * Set_rms::process(mha_wave_t *signal)
{
    poll_config();

    mha_wave_t channel_rms_in = MHA_AC::get_var_waveform(ac, ac_name_in.data);
    mha_wave_t channel_rms_out = MHA_AC::get_var_waveform(ac, ac_name_out.data);

    return cfg->process(signal, channel_rms_in, channel_rms_out);
}


void Set_rms::release()
{
    // Unlock the configuration variables:
    ac_name_in.setlock(false);
    ac_name_out.setlock(false);
}


void Set_rms::update_cfg()
{
    if (is_prepared())
    {
        push_config(new Set_rms_cfg(ac_name_in.data,
                                    ac_name_out.data));
    }
}


MHAPLUGIN_CALLBACKS(set_rms, Set_rms, wave, wave)

MHAPLUGIN_DOCUMENTATION(set_rms, "ci-vocoder",
"This plugin sets the channels of an output signal (e.g. of some signal "
"processing operation) to the RMS values of the original input signal. First, "
"the output signal RMS is normalized; then, it is set to the RMS of the "
"original input signal. It operates in the time domain.")
