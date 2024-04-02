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


#include "get_rms.hh"


Get_rms_cfg::Get_rms_cfg(mha_real_t tau,
                         Get_rms *Get_rms,
                         std::vector<mha_real_t> rms_prev)
    : tau_cfg(tau),
      rms_prev_cfg(rms_prev)
{
    if (tau < Get_rms->input_cfg().fragsize/Get_rms->input_cfg().srate)
    {
        throw MHA_Error(__FILE__, __LINE__,
                        "The time constant tau (%f s) must be greater than or "
                        "equal to fragsize/srate \n(%u/%f = %f s).",
                        tau,
                        Get_rms->input_cfg().fragsize,
                        Get_rms->input_cfg().srate,
                        Get_rms->input_cfg().fragsize/Get_rms->input_cfg().srate);
    }
}


// Process function of the runtime configuration class (main signal processing
// function). It leaves the input signal fragment unchanged; its purpose is to
// compute the AC variable rms_ac for further processing in the plugin chain:

mha_wave_t * Get_rms_cfg::process(mha_wave_t *signal,
                                  Get_rms *Get_rms,
                                  MHA_AC::waveform_t *rms_ac)
{
    // Compute the smoothing coefficient alpha from fragment size, time
    // constant and sampling rate:
    mha_real_t alpha = Get_rms->input_cfg().fragsize/
        (tau_cfg * Get_rms->input_cfg().srate);

    // Loop over all channels (= electrodes):
    for (unsigned int channel = 0; channel < signal->num_channels; channel++)
    {
        // Compute the averaged RMS from the current and previous RMS values:
        rms_ac->value(0, channel) = alpha * MHASignal::rmslevel(*signal, channel) +
            (1 - alpha) * rms_prev_cfg.at(channel);
        // Ensure that the averaged RMS cannot take on any subnormal values:
        MHAFilter::make_friendly_number(rms_ac->value(0, channel));
        // Set the previous RMS to the value of the current averaged RMS:
        rms_prev_cfg.at(channel) = rms_ac->value(0, channel);
    }

    // Return the (unchanged) output signal fragment:
    return signal;
}


Get_rms::Get_rms(algo_comm_t &ac,
                 const std::string &algo_name)
    : MHAPlugin::plugin_t<Get_rms_cfg>("Get RMS", ac),
      algo_name(algo_name),
      tau("Time constant for the exponential average / s (must be greater "
          "than or equal to fragsize/srate; if equal, no averaging is "
          "performed, i.e. alpha = 1)",
          "1",
          "]0,["),
      rms_ac(nullptr)
{
    insert_item("tau", &tau);

    patchbay.connect(&tau.writeaccess, this, &Get_rms::update_cfg);
}


void Get_rms::prepare(mhaconfig_t &signal_info)
{
    if (signal_info.domain != MHA_WAVEFORM)
    {
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin can only process waveform signals.");
    }

    rms_prev.resize(signal_info.channels, 0.0);
    rms_ac = new MHA_AC::waveform_t(ac,
                                    algo_name,
                                    1,
                                    signal_info.channels,
                                    true);

    update_cfg();
}


mha_wave_t * Get_rms::process(mha_wave_t *signal)
{
    poll_config();

    return cfg->process(signal, this, rms_ac);
}


void Get_rms::release()
{
    delete rms_ac;
    rms_ac = nullptr;
}


void Get_rms::update_cfg()
{
    if (is_prepared())
    {
        push_config(new Get_rms_cfg(tau.data,
                                    this,
                                    rms_prev));
    }
}


MHAPLUGIN_CALLBACKS(get_rms, Get_rms, wave, wave)

MHAPLUGIN_DOCUMENTATION(get_rms, "ci-vocoder",
"This plugin computes the exponentially averaged RMS of the channels of an "
"input signal and stores it in an AC variable. It operates in the time domain.")
