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


#include "ci_auralization_ace.hh"


Ci_auralization_ace_cfg::Ci_auralization_ace_cfg(std::string ac_name,
                                                 mha_real_t compression_coefficient,
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
                                                 Ci_auralization_ace *Ci_auralization_ace)
    : ac_name_cfg(ac_name),
      compression_coefficient_cfg(compression_coefficient),
      base_level_cfg(base_level),
      saturation_level_cfg(saturation_level),
      threshold_level_cfg(threshold_level),
      comfort_level_cfg(comfort_level),
      electrode_distance_cfg(electrode_distance),
      lambda_cfg(lambda),
      phase_duration_cfg(phase_duration),
      interphase_gap_cfg(interphase_gap),
      phase_order_cfg(phase_order),
      electrodogram_cfg(electrodogram),
      n_electrodes_cfg(n_electrodes),
      m_electrodes_cfg(m_electrodes)
{
    unsigned int phase_duration_samples =
        std::max(int(round(phase_duration_cfg * Ci_auralization_ace->input_cfg().srate)), 1);
    unsigned int interphase_gap_samples =
        round(interphase_gap_cfg * Ci_auralization_ace->input_cfg().srate);

    unsigned int pulse_duration_samples =
        2 * phase_duration_samples + interphase_gap_samples;

    if (ac_name_cfg == "")
    {
        throw MHA_Error(__FILE__, __LINE__,
                        "The name of the AC variable containing the "
                        "electrodogram must be specified.");
    }

    if (saturation_level_cfg <= base_level_cfg)
    {
        throw MHA_Error(__FILE__, __LINE__,
                        "Saturation level (%f) must be greater than base "
                        "level (%f).",
                        saturation_level_cfg,
                        base_level_cfg);
    }

    if (threshold_level_cfg.size() == 1 && comfort_level_cfg.size() == 1)
    {
        for (unsigned int m = 0; m < m_electrodes; m++)
        {
            if (comfort_level_cfg.at(0) <= threshold_level_cfg.at(0))
            {
                throw MHA_Error(__FILE__, __LINE__,
                                "Comfort level (%f) must be greater than "
                                "threshold level (%f).",
                                comfort_level_cfg.at(0),
                                threshold_level_cfg.at(0));
            }
        }
    }
    else if (threshold_level_cfg.size() == 1 && comfort_level_cfg.size() > 1)
    {
        if (comfort_level_cfg.size() != m_electrodes)
        {
            throw MHA_Error(__FILE__, __LINE__,
                            "Comfort level must have either 1 or %u elements.",
                            m_electrodes);
        }
        for (unsigned int m = 0; m < m_electrodes; m++)
        {
            if (comfort_level_cfg.at(m) <= threshold_level_cfg.at(0))
            {
                throw MHA_Error(__FILE__, __LINE__,
                                "Comfort level (%f) must be greater than "
                                "threshold level (%f) for electrode %u.",
                                comfort_level_cfg.at(m),
                                threshold_level_cfg.at(0),
                                m);
            }
        }
    }
    else if (threshold_level_cfg.size() > 1 && comfort_level_cfg.size() == 1)
    {
        if (threshold_level_cfg.size() != m_electrodes)
        {
            throw MHA_Error(__FILE__, __LINE__,
                            "Threshold level must have either "
                            "1 or %u elements.",
                            m_electrodes);
        }
        for (unsigned int m = 0; m < m_electrodes; m++)
        {
            if (comfort_level_cfg.at(0) <= threshold_level_cfg.at(m))
            {
                throw MHA_Error(__FILE__, __LINE__,
                                "Comfort level (%f) must be greater than "
                                "threshold level (%f) for electrode %u.",
                                comfort_level_cfg.at(0),
                                threshold_level_cfg.at(m),
                                m);
            }
        }
    }
    else if (threshold_level_cfg.size() > 1 && comfort_level_cfg.size() > 1)
    {
        if (threshold_level_cfg.size() != m_electrodes)
        {
            throw MHA_Error(__FILE__, __LINE__,
                            "Threshold level must have either "
                            "1 or %u elements.",
                            m_electrodes);
        }
        if (comfort_level_cfg.size() != m_electrodes)
        {
            throw MHA_Error(__FILE__, __LINE__,
                            "Comfort level must have either 1 or %u elements.",
                            m_electrodes);
        }
        for (unsigned int m = 0; m < m_electrodes; m++)
        {
            if (comfort_level_cfg.at(m) <= threshold_level_cfg.at(m))
            {
                throw MHA_Error(__FILE__, __LINE__,
                                "Comfort level (%f) must be greater than "
                                "threshold level (%f) for electrode %u.",
                                comfort_level_cfg.at(m),
                                threshold_level_cfg.at(m),
                                m);
            }
        }
    }

    if (Ci_auralization_ace->input_cfg().fragsize < pulse_duration_samples)
    {
        throw MHA_Error(__FILE__, __LINE__,
                        "The pulse duration in samples, given by "
                        "2 * phase duration + interphase gap \n"
                        "(2 * %u + %u = %u samples), must not exceed the "
                        "fragment size (%u samples).",
                        phase_duration_samples,
                        interphase_gap_samples,
                        pulse_duration_samples,
                        Ci_auralization_ace->input_cfg().fragsize);
    }
}


// Process function of the runtime configuration class (main signal processing
// function). It leaves the input signal fragment unchanged; its purpose is to
// compute the AC variable stimulation_signal_ac, which contains all the
// electrode-specific information (including concrete time specifications)
// necessary for auralization, for further processing in the plugin chain:

mha_wave_t * Ci_auralization_ace_cfg::process(mha_wave_t *signal,
                                              Ci_auralization_ace *Ci_auralization_ace,
                                              MHA_AC::waveform_t *stimulation_signal_ac)
{
    // Deep copy the electrodogram to a new variable containing the so-called
    // stimulation sequence, which will contain all information needed for
    // auralization except for concrete time specifications:
    mha_wave_t stimulation_sequence = electrodogram_cfg;
    stimulation_sequence.buf = new mha_real_t[m_electrodes_cfg*n_electrodes_cfg];
    for (unsigned int i = 0; i < m_electrodes_cfg*n_electrodes_cfg; i++)
    {
        stimulation_sequence.buf[i] = electrodogram_cfg.buf[i];
    }
    stimulation_sequence.num_channels = m_electrodes_cfg;
    stimulation_sequence.num_frames = n_electrodes_cfg;
    stimulation_sequence.channel_info = nullptr;


    // Remapping of electrical dynamic range to compressed electrical
    // magnitude:

    // Loop over all n active electrodes:
    for (unsigned int n = 0; n < n_electrodes_cfg; n++)
    {
        // Loop over all m electrodes:
        for (unsigned int m = 0; m < m_electrodes_cfg; m++)
        {
            // If the stimulation sequence value is not equal to zero
            // (otherwise, do nothing):
            if (value(stimulation_sequence, n, m) != 0.0)
            {
                // If threshold level and comfort level are both scalars:
                if (threshold_level_cfg.size() == 1 && comfort_level_cfg.size() == 1)
                {
                    // Remap the clinical current units to compressed
                    // electrical magnitude:
                    value(stimulation_sequence, n, m) =
                        (value(stimulation_sequence, n, m) - threshold_level_cfg.at(0))/
                        (comfort_level_cfg.at(0) - threshold_level_cfg.at(0));
                }
                // Else, if threshold level is a scalar and comfort level is a
                // vector:
                else if (threshold_level_cfg.size() == 1 && comfort_level_cfg.size() > 1)
                {
                    // Remap the clinical current units to compressed
                    // electrical magnitude:
                    value(stimulation_sequence, n, m) =
                        (value(stimulation_sequence, n, m) - threshold_level_cfg.at(0))/
                        (comfort_level_cfg.at(m) - threshold_level_cfg.at(0));
                }
                // Else, if threshold level is a vector and comfort level is a
                // scalar:
                else if (threshold_level_cfg.size() > 1 && comfort_level_cfg.size() == 1)
                {
                    // Remap the clinical current units to compressed
                    // electrical magnitude:
                    value(stimulation_sequence, n, m) =
                        (value(stimulation_sequence, n, m) - threshold_level_cfg.at(m))/
                        (comfort_level_cfg.at(0) - threshold_level_cfg.at(m));
                }
                // Else, if threshold level and comfort level are both vectors:
                else if (threshold_level_cfg.size() > 1 && comfort_level_cfg.size() > 1)
                {
                    // Remap the clinical current units to compressed
                    // electrical magnitude:
                    value(stimulation_sequence, n, m) =
                        (value(stimulation_sequence, n, m) - threshold_level_cfg.at(m))/
                        (comfort_level_cfg.at(m) - threshold_level_cfg.at(m));
                }
            }
        }
    }


    // Inversion of loudness growth function:

    // Loop over all n active electrodes:
    for (unsigned int n = 0; n < n_electrodes_cfg; n++)
    {
        // Loop over all m electrodes:
        for (unsigned int m = 0; m < m_electrodes_cfg; m++)
        {
            // If the stimulation sequence value is not equal to zero
            // (otherwise, do nothing):
            if (value(stimulation_sequence, n, m) != 0.0)
            {
                // Apply the inverted loudness growth function:
                value(stimulation_sequence, n, m) =
                    (pow(1.0 + compression_coefficient_cfg, value(stimulation_sequence, n, m)) - 1.0) *
                    (saturation_level_cfg - base_level_cfg)/
                    compression_coefficient_cfg +
                    base_level_cfg;
            }
        }
    }


    // Spatial spread of excitation (channel interaction):

    // Loop over all n active electrodes:
    for (unsigned int n = 0; n < n_electrodes_cfg; n++)
    {
        // Loop over the m electrodes until finding the index of the current
        // active electrode (= pulse, i.e. a value not equal to zero); if all
        // values are equal to zero, the last electrode is arbitrarily chosen
        // as the pulse:
        unsigned int pulse_idx = 0;
        while (value(stimulation_sequence, n, pulse_idx) == 0.0 &&
               pulse_idx < m_electrodes_cfg - 1)
        {
            pulse_idx++;
        }
        // Register the value of the pulse:
        mha_real_t pulse_val = value(stimulation_sequence, n, pulse_idx);

        // If the length constant of the exponential spread of excitation is
        // zero, just retain the value of the pulse; else, looping down over
        // the electrodes with indices less than that of the pulse, set the
        // values of the remaining (inactive) electrodes according to the
        // exponential decay function and the distance between the active
        // electrode and the respective inactive electrode:
        unsigned int m_down = pulse_idx;
        if (lambda_cfg == 0.0)
        {
            value(stimulation_sequence, n, m_down) = pulse_val;
        }
        else
        {
            while (m_down > 0)
            {
                m_down--;
                mha_real_t idx_distance = pulse_idx - m_down;
                value(stimulation_sequence, n, m_down) =
                    pulse_val * exp(-idx_distance * electrode_distance_cfg/lambda_cfg);
            }
        }

        // If the length constant of the exponential spread of excitation is
        // zero, just retain the value of the pulse; else, looping up over
        // the electrodes with indices less than that of the pulse, set the
        // values of the remaining (inactive) electrodes according to the
        // exponential decay function and the distance between the active
        // electrode and the respective inactive electrode:
        unsigned int m_up = pulse_idx;
        if (lambda_cfg == 0.0)
        {
            value(stimulation_sequence, n, m_down) = pulse_val;
        }
        else
        {
            while (m_up < m_electrodes_cfg - 1)
            {
                m_up++;
                mha_real_t idx_distance = m_up - pulse_idx;
                value(stimulation_sequence, n, m_up) =
                    pulse_val * exp(-idx_distance * electrode_distance_cfg/lambda_cfg);
            }
        }
    }


    // Stimulation signal generation:

    // Initialize the stimulation signal with zeros:
    stimulation_signal_ac->assign(0.0);

    // Compute the duration of one phase of a biphasic pulse in samples:
    unsigned int phase_duration_samples =
        std::max(int(round(phase_duration_cfg * Ci_auralization_ace->input_cfg().srate)), 1);
    // Compute the duration of the gap between the phases of a biphasic pulse
    // in samples:
    unsigned int interphase_gap_samples =
        round(interphase_gap_cfg * Ci_auralization_ace->input_cfg().srate);

    // Loop over all n active electrodes:
    for (unsigned int n = 0; n < n_electrodes_cfg; n++)
    {
        // Loop over all m electrodes:
        for (unsigned int m = 0; m < m_electrodes_cfg; m++)
        {
            // If the stimulation sequence value is not equal to zero
            // (otherwise, do nothing):
            if (value(stimulation_sequence, n, m) != 0.0)
            {
                // If the order of the phases of a biphasic pulse is
                // cathodic-first:
                if (phase_order_cfg == 0)
                {
                    // Loop over all samples of the cathodic (= negative) phase:
                    for (unsigned int frame = 0; frame < phase_duration_samples; frame++)
                    {
                        // Shift the starting point to the first sample of the
                        // current total stimulation period
                        // (= n * fragsize/n_electrodes, where n is the index
                        // of the currently processed active electrode and
                        // fragsize/n_electrodes is the total stimulation
                        // period in samples); then set as many samples as the
                        // duration of one phase of a biphasic pulse to the
                        // corresponding value of the stimulation sequence
                        // (with a negative sign for the cathodic phase):
                        stimulation_signal_ac->assign(round(n*mha_real_t(signal->num_frames)/mha_real_t(n_electrodes_cfg))+
                                                      frame,
                                                      m,
                                                      -value(stimulation_sequence, n, m));
                    }
                    // Loop over all samples of the anodic (= positive) phase:
                    for (unsigned int frame = 0; frame < phase_duration_samples; frame++)
                    {
                        // Repeat the procedure, starting from a position
                        // shifted by the duration of one phase plus the
                        // interphase gap in samples (with a positive sign for
                        // the anodic phase):
                        stimulation_signal_ac->assign(round(n*mha_real_t(signal->num_frames)/mha_real_t(n_electrodes_cfg))+
                                                      phase_duration_samples+interphase_gap_samples+frame,
                                                      m,
                                                      value(stimulation_sequence, n, m));
                    }
                }
                // Else, if the order of the phases of a biphasic pulse is
                // anodic-first:
                else if (phase_order_cfg == 1)
                {
                    // Loop over all samples of the anodic (= positive) phase:
                    for (unsigned int frame = 0; frame < phase_duration_samples; frame++)
                    {
                        // Shift the starting point to the first sample of the
                        // current total stimulation period
                        // (= n * fragsize/n_electrodes, where n is the index
                        // of the currently processed active electrode and
                        // fragsize/n_electrodes is the total stimulation
                        // period in samples); then set as many samples as the
                        // duration of one phase of a biphasic pulse to the
                        // corresponding value of the stimulation sequence
                        // (with a positive sign for the anodic phase):
                        stimulation_signal_ac->assign(round(n*mha_real_t(signal->num_frames)/mha_real_t(n_electrodes_cfg))+
                                                      frame,
                                                      m,
                                                      value(stimulation_sequence, n, m));
                    }
                    // Loop over all samples of the cathodic (= negative) phase:
                    for (unsigned int frame = 0; frame < phase_duration_samples; frame++)
                    {
                        // Repeat the procedure, starting from a position
                        // shifted by the duration of one phase plus the
                        // interphase gap in samples (with a negative sign for
                        // the cathodic phase):
                        stimulation_signal_ac->assign(round(n*mha_real_t(signal->num_frames)/mha_real_t(n_electrodes_cfg))+
                                                      phase_duration_samples+interphase_gap_samples+frame,
                                                      m,
                                                      -value(stimulation_sequence, n, m));
                    }
                }
            }
        }
    }

    // Deallocate the memory allocated when copying the electrodogram to the
    // stimulation sequence, and set the corresponding pointer to NULL:
    delete[] stimulation_sequence.buf;
    stimulation_sequence.buf = nullptr;

    // Return the (unchanged) output signal fragment:
    return signal;
}


Ci_auralization_ace::Ci_auralization_ace(algo_comm_t &ac,
                                         const std::string &algo_name)
    : MHAPlugin::plugin_t<Ci_auralization_ace_cfg>("CI auralization (ACE)",
                                                   ac),
      algo_name(algo_name),
      ac_name("Name of the AC variable containing the electrodogram (cannot be "
              "changed at runtime)",
              ""),
      compression_coefficient("Compression coefficient of the loudness growth "
                              "function",
                              "416.2",
                              "]0,["),
      base_level("Base level of the input (acoustic) dynamic range / Pa",
                 "0.000355656",
                 "[0,["),
      saturation_level("Saturation level of the input (acoustic) dynamic "
                       "range / Pa",
                       "0.0355656",
                       "[0,["),
      threshold_level("Threshold level of the output (electric) dynamic range "
                      "for each electrode / CU (same level for all electrodes "
                      "if only one value is specified)",
                      "[96]",
                      "[0,255]"),
      comfort_level("Comfort level of the output (electric) dynamic range for "
                    "each electrode / CU (same level for all electrodes if "
                    "only one value is specified)",
                    "[160]",
                    "[0,255]"),
      electrode_distance("Distance of the electrodes / m",
                         "0.000714286",
                         "]0,["),
      lambda("Length constant of exponential spread of excitation / m",
             "0.0031021",
             "[0,["),
      phase_duration("Duration of one phase of a biphasic pulse / s",
                     "25e-6",
                     "]0,["),
      interphase_gap("Duration of the gap between the phases of a biphasic "
                     "pulse / s",
                     "8e-6",
                     "[0,["),
      phase_order("Order of the phases of a biphasic pulse",
                  "cathodic_first",
                  "[cathodic_first anodic_first]"),
      stimulation_signal_ac(nullptr)
{
    insert_item("ac_name", &ac_name);
    insert_item("compression_coefficient", &compression_coefficient);
    insert_item("base_level", &base_level);
    insert_item("saturation_level", &saturation_level);
    insert_item("threshold_level", &threshold_level);
    insert_item("comfort_level", &comfort_level);
    insert_item("electrode_distance", &electrode_distance);
    insert_item("lambda", &lambda);
    insert_item("phase_duration", &phase_duration);
    insert_item("interphase_gap", &interphase_gap);
    insert_item("phase_order", &phase_order);

    patchbay.connect(&ac_name.writeaccess, this, &Ci_auralization_ace::update_cfg);
    patchbay.connect(&compression_coefficient.writeaccess, this, &Ci_auralization_ace::update_cfg);
    patchbay.connect(&base_level.writeaccess, this, &Ci_auralization_ace::update_cfg);
    patchbay.connect(&saturation_level.writeaccess, this, &Ci_auralization_ace::update_cfg);
    patchbay.connect(&threshold_level.writeaccess, this, &Ci_auralization_ace::update_cfg);
    patchbay.connect(&comfort_level.writeaccess, this, &Ci_auralization_ace::update_cfg);
    patchbay.connect(&electrode_distance.writeaccess, this, &Ci_auralization_ace::update_cfg);
    patchbay.connect(&lambda.writeaccess, this, &Ci_auralization_ace::update_cfg);
    patchbay.connect(&phase_duration.writeaccess, this, &Ci_auralization_ace::update_cfg);
    patchbay.connect(&interphase_gap.writeaccess, this, &Ci_auralization_ace::update_cfg);
    patchbay.connect(&phase_order.writeaccess, this, &Ci_auralization_ace::update_cfg);
}


void Ci_auralization_ace::prepare(mhaconfig_t &signal_info)
{
    if (signal_info.domain != MHA_WAVEFORM)
    {
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin can only process waveform signals.");
    }

    electrodogram = MHA_AC::get_var_waveform(ac, ac_name.data);

    n_electrodes = electrodogram.num_frames;
    m_electrodes = electrodogram.num_channels;

    stimulation_signal_ac = new MHA_AC::waveform_t(ac,
                                                   algo_name,
                                                   input_cfg().fragsize,
                                                   m_electrodes,
                                                   true);

    update_cfg();

    // Lock the configuration variable against write access:
    ac_name.setlock(true);
}


mha_wave_t * Ci_auralization_ace::process(mha_wave_t *signal)
{
    poll_config();

    return cfg->process(signal, this, stimulation_signal_ac);
}


void Ci_auralization_ace::release()
{
    // Unlock the configuration variable:
    ac_name.setlock(false);

    delete stimulation_signal_ac;
    stimulation_signal_ac = nullptr;
}


void Ci_auralization_ace::update_cfg()
{
    if (is_prepared())
    {
        push_config(new Ci_auralization_ace_cfg(ac_name.data,
                                                compression_coefficient.data,
                                                base_level.data,
                                                saturation_level.data,
                                                threshold_level.data,
                                                comfort_level.data,
                                                electrode_distance.data,
                                                lambda.data,
                                                phase_duration.data,
                                                interphase_gap.data,
                                                phase_order.data.get_index(),
                                                electrodogram,
                                                n_electrodes,
                                                m_electrodes,
                                                this));
    }
}


MHAPLUGIN_CALLBACKS(ci_auralization_ace, Ci_auralization_ace, wave, wave)

MHAPLUGIN_DOCUMENTATION(ci_auralization_ace, "ci-vocoder",
"This plugin generates an auralized audio signal from the specified "
"AC variable, using a stimulation strategy similar to a typical ACE (advanced "
"combination encoder, n-of-m) coding strategy with 22 channels. It operates in "
"the time domain.")
