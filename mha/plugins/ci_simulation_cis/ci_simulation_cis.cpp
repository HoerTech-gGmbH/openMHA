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


#include "ci_simulation_cis.hh"

#include <chrono>


const unsigned int CHANNELS = 1;
const mha_real_t SRATE = 48000;
const unsigned int M_ELECTRODES = 12;

Ci_simulation_cis_cfg::Ci_simulation_cis_cfg(std::vector<mha_real_t> weights,
                                             mha_real_t compression_coefficient,
                                             mha_real_t base_level,
                                             mha_real_t saturation_level,
                                             std::vector<mha_real_t> threshold_level,
                                             std::vector<mha_real_t> maximum_comfortable_level,
                                             std::vector<int> disabled_electrodes,
                                             unsigned int stimulation_order,
                                             std::default_random_engine random_number_generator,
                                             Ci_simulation_cis *Ci_simulation_cis)
    : weights_cfg(weights),
      compression_coefficient_cfg(compression_coefficient),
      base_level_cfg(base_level),
      saturation_level_cfg(saturation_level),
      threshold_level_cfg(threshold_level),
      maximum_comfortable_level_cfg(maximum_comfortable_level),
      disabled_electrodes_cfg(disabled_electrodes),
      stimulation_order_cfg(stimulation_order),
      random_number_generator_cfg(random_number_generator)
{
    if (weights_cfg.size() != M_ELECTRODES)
    {
        throw MHA_Error(__FILE__, __LINE__,
                        "The number of weights must be equal to the total "
                        "number of electrodes (%u).",
                        M_ELECTRODES);
    }

    if (saturation_level_cfg <= base_level_cfg)
    {
        throw MHA_Error(__FILE__, __LINE__,
                        "Saturation level (%f) must be greater than base "
                        "level (%f).",
                        saturation_level_cfg,
                        base_level_cfg);
    }

    if (threshold_level_cfg.size() == 1 && maximum_comfortable_level_cfg.size() == 1)
    {
        for (unsigned int m = 0; m < M_ELECTRODES; m++)
        {
            if (maximum_comfortable_level_cfg.at(0) <= threshold_level_cfg.at(0))
            {
                throw MHA_Error(__FILE__, __LINE__,
                                "Maximum comfortable level (%f) must be "
                                "greater than threshold level (%f).",
                                maximum_comfortable_level_cfg.at(0),
                                threshold_level_cfg.at(0));
            }
        }
    }
    else if (threshold_level_cfg.size() == 1 && maximum_comfortable_level_cfg.size() > 1)
    {
        if (maximum_comfortable_level_cfg.size() != M_ELECTRODES)
        {
            throw MHA_Error(__FILE__, __LINE__,
                            "Maximum comfortable level must have either 1 or "
                            "%u elements.",
                            M_ELECTRODES);
        }
        for (unsigned int m = 0; m < M_ELECTRODES; m++)
        {
            if (maximum_comfortable_level_cfg.at(m) <= threshold_level_cfg.at(0))
            {
                throw MHA_Error(__FILE__, __LINE__,
                                "Maximum comfortable level (%f) must be "
                                "greater than threshold level (%f) for "
                                "electrode %u.",
                                maximum_comfortable_level_cfg.at(m),
                                threshold_level_cfg.at(0), m);
            }
        }
    }
    else if (threshold_level_cfg.size() > 1 && maximum_comfortable_level_cfg.size() == 1)
    {
        if (threshold_level_cfg.size() != M_ELECTRODES)
        {
            throw MHA_Error(__FILE__, __LINE__,
                            "Threshold level must have either 1 or %u "
                            "elements.",
                            M_ELECTRODES);
        }
        for (unsigned int m = 0; m < M_ELECTRODES; m++)
        {
            if (maximum_comfortable_level_cfg.at(0) <= threshold_level_cfg.at(m))
            {
                throw MHA_Error(__FILE__, __LINE__,
                                "Maximum comfortable level (%f) must be "
                                "greater than threshold level (%f) for "
                                "electrode %u.",
                                maximum_comfortable_level_cfg.at(0),
                                threshold_level_cfg.at(m), m);
            }
        }
    }
    else if (threshold_level_cfg.size() > 1 && maximum_comfortable_level_cfg.size() > 1)
    {
        if (threshold_level_cfg.size() != M_ELECTRODES)
        {
            throw MHA_Error(__FILE__, __LINE__,
                            "Threshold level must have either 1 or %u "
                            "elements.",
                            M_ELECTRODES);
        }
        if (maximum_comfortable_level_cfg.size() != M_ELECTRODES)
        {
            throw MHA_Error(__FILE__, __LINE__,
                            "Maximum comfortable level must have either 1 or "
                            "%u elements.",
                            M_ELECTRODES);
        }
        for (unsigned int m = 0; m < M_ELECTRODES; m++)
        {
            if (maximum_comfortable_level_cfg.at(m) <= threshold_level_cfg.at(m))
            {
                throw MHA_Error(__FILE__, __LINE__,
                                "Maximum comfortable level (%f) must be "
                                "greater than threshold level (%f) for "
                                "electrode %u.",
                                maximum_comfortable_level_cfg.at(m),
                                threshold_level_cfg.at(m), m);
            }
        }
    }

    if (size(disabled_electrodes_cfg) > M_ELECTRODES)
    {
        throw MHA_Error(__FILE__, __LINE__,
                        "At most %u electrodes can be disabled.",
                        M_ELECTRODES);
    }

    signal_out = new MHASignal::waveform_t(Ci_simulation_cis->input_cfg().fragsize, CHANNELS);
}


Ci_simulation_cis_cfg::~Ci_simulation_cis_cfg()
{
    delete signal_out;
    signal_out = nullptr;
}

// Process function of the runtime configuration class (main signal processing
// function). It takes an input signal fragment as well as the real and
// imaginary outputs of a gammatone filterbank with m bands as a parameter,
// resulting in a total of 1 + m * 2 input signal channels. It returns the
// input signal fragment unchanged; the purpose of the function is to compute
// the AC variable electrodogram_ac from the gammatone filterbank output for
// further processing in the plugin chain:

mha_wave_t * Ci_simulation_cis_cfg::process(mha_wave_t *signal,
                                            Ci_simulation_cis *Ci_simulation_cis,
                                            MHA_AC::waveform_t *electrodogram_ac)
{
    // Electrode stimulation order:

    // Vector of pairs (one for each filterbank frequency band, i.e. electrode)
    // for storing the envelope along with the corresponding frequency band
    // index:
    std::vector<std::pair<mha_real_t, unsigned int>> envelope(M_ELECTRODES);

    // Loop over all frequency bands:
    for (unsigned int m = 0; m < M_ELECTRODES; m++)
    {
        // Register the corresponding frequency band index:
        envelope.at(m).second = m;
    }

    // If the electrode stimulation order is random:
    if (stimulation_order_cfg == 0)
    {
        // Randomly shuffle the order of the elements in the envelope vector of
        // pairs (the random number generator must be seeded only once, in the
        // prepare function of the plugin interface class):
        shuffle(std::begin(envelope),
                std::end(envelope),
                random_number_generator_cfg);
    }
    // Else, if the electrode stimulation order is base-to-apex:
    else if (stimulation_order_cfg == 1)
    {
        // Sort the elements in the envelope vector of pairs from greatest to
        // least electrode index, using a lambda expression:
        std::sort(std::begin(envelope), std::end(envelope),
                  [](const std::pair<mha_real_t, unsigned int> &a,
                     const std::pair<mha_real_t, unsigned int> &b) ->
                  bool { return (a.second > b.second); });
    }
    // Else, if the electrode stimulation order is apex-to-base:
    else if (stimulation_order_cfg == 2)
    {
        // Sort the elements in the envelope vector of pairs from least to
        // greatest electrode index, using a lambda expression:
        std::sort(std::begin(envelope), std::end(envelope),
                  [](const std::pair<mha_real_t, unsigned int> &a,
                     const std::pair<mha_real_t, unsigned int> &b) ->
                  bool { return (a.second < b.second); });
    }


    // Envelope extraction:

    // Number of frames per pulse:
    unsigned int frames_per_pulse;

    // Frame index at which the gammatone filterbank output is to be sampled
    // for the computation of the envelope:
    unsigned int frame;

    // Loop over all frequency bands:
    for (unsigned int m = 0; m < M_ELECTRODES; m++)
    {
        // Compute the number of frames per pulse:
        frames_per_pulse = round(mha_real_t(Ci_simulation_cis->input_cfg().fragsize)/mha_real_t(M_ELECTRODES));

        // Specify at which frame to sample the envelope of the current input
        // signal fragment, given the electrode stimulation order:
        frame = frames_per_pulse * envelope.at(m).second;

        // Compute the envelope (= square root of the weighted squared absolute
        // value of the gammatone filterbank output):
        envelope.at(m).first = sqrt(weights_cfg.at(m) * (pow(value(signal, frame, 2*m+1), 2.0) + pow(value(signal, frame, 2*m+2), 2.0)));

        // Set the envelope to zero if the corresponding electrode is disabled:
        if (std::find(std::begin(disabled_electrodes_cfg),
                      std::end(disabled_electrodes_cfg), m) != std::end(disabled_electrodes_cfg)) {
            envelope.at(m).first = 0.0;
        }
    }


    // Loudness growth function:

    // Loop over all envelopes (= active electrodes):
    for (unsigned int m = 0; m < M_ELECTRODES; m++)
    {
        // If the envelope is less than or equal to the base level:
        if (envelope.at(m).first <= base_level_cfg)
        {
            // Set the envelope to the minimum value:
            envelope.at(m).first = 0.0;
        }
        // Else, if the envelope is greater than or equal to the saturation
        // level:
        else if (envelope.at(m).first >= saturation_level_cfg)
        {
            // Set the envelope to the maximum value:
            envelope.at(m).first = 1.0;
        }
        // Else:
        else
        {
            // Apply the loudness growth function to the envelope, mapping the
            // (acoustic) envelope magnitude to compressed electrical
            // magnitude, if the compression coefficient is greater than zero;
            // otherwise, do nothing (linear loudness growth function):
            if (compression_coefficient_cfg > 0)
            {
                envelope.at(m).first =
                    log(1 + compression_coefficient_cfg * (envelope.at(m).first - base_level_cfg)/
                            (saturation_level_cfg - base_level_cfg))/
                    log(1 + compression_coefficient_cfg);
            }
        }
    }


    // Mapping of electrical dynamic range to current units (cu):

    // Loop over all envelopes (= active electrodes):
    for (unsigned int m = 0; m < M_ELECTRODES; m++)
    {
        // If threshold level and maximum comfortable level are both scalars:
        if (threshold_level_cfg.size() == 1 && maximum_comfortable_level_cfg.size() == 1)
        {
            // Convert the electrical magnitudes to current units:
            envelope.at(m).first = threshold_level_cfg.at(0) +
                ((maximum_comfortable_level_cfg.at(0) - threshold_level_cfg.at(0)) *
                 (envelope.at(m).first));
        }
        // Else, if threshold level is a scalar and maximum comfortable level
        // is a vector:
        else if (threshold_level_cfg.size() == 1 && maximum_comfortable_level_cfg.size() > 1)
        {
            // Convert the electrical magnitudes to current units:
            envelope.at(m).first = threshold_level_cfg.at(0) +
                ((maximum_comfortable_level_cfg.at(envelope.at(m).second) - threshold_level_cfg.at(0)) *
                 (envelope.at(m).first));
        }
        // Else, if threshold level is a vector and maximum comfortable level is a scalar:
        else if (threshold_level_cfg.size() > 1 && maximum_comfortable_level_cfg.size() == 1)
        {
            // Convert the electrical magnitudes to current units:
            envelope.at(m).first = threshold_level_cfg.at(envelope.at(m).second) +
            ((maximum_comfortable_level_cfg.at(0) - threshold_level_cfg.at(envelope.at(m).second)) *
             (envelope.at(m).first));
        }
        // Else, if threshold level and maximum comfortable level are both vectors:
        else if (threshold_level_cfg.size() > 1 && maximum_comfortable_level_cfg.size() > 1)
        {
            // Convert the electrical magnitudes to current units:
            envelope.at(m).first = threshold_level_cfg.at(envelope.at(m).second) +
            ((maximum_comfortable_level_cfg.at(envelope.at(m).second) - threshold_level_cfg.at(envelope.at(m).second)) *
             (envelope.at(m).first));
        }
    }


    // Electrodogram generation:

    // Initialize the electrodogram with zeros:
    electrodogram_ac->assign(0.0);

    // Loop over all envelopes (= active electrodes):
    for (unsigned int m = 0; m < M_ELECTRODES; m++)
    {
        // For each "frame" (i.e. time slot) of the electrodogram, set the
        // value for the respective "channel" (i.e. electrode index) to the
        // corresponding envelope value:
        electrodogram_ac->value(envelope.at(m).second, m) = envelope.at(m).first;
    }

    // Return the (unchanged) output signal fragment:
    MHASignal::copy_channel(*signal_out, *signal, 0, 0);
    return signal_out;
}


Ci_simulation_cis::Ci_simulation_cis(algo_comm_t &ac,
                                     const std::string &algo_name)
    : MHAPlugin::plugin_t<Ci_simulation_cis_cfg>("CI simulation (CIS)",
                                                 ac),
      algo_name(algo_name),
      weights("Weights for the analysis filterbank bands",
              "[1 0.752066 0.569214 0.431813 0.327594 0.248486 0.18827 0.142332 0.107405 0.0808826 0.0608164 0.0456657]",
              "]0,1]"),
      compression_coefficient("Compression coefficient of the loudness growth "
                              "function",
                              "500",
                              "[0,["),
      base_level("Base level of the input (acoustic) dynamic range / Pa",
                 "0.000355656",
                 "[0,["),
      saturation_level("Saturation level of the input (acoustic) dynamic "
                       "range / Pa",
                       "2",
                       "[0,["),
      threshold_level("Threshold level of the output (electric) dynamic range "
                      "for each electrode / cu (same level for all electrodes "
                      "if only one value is specified)",
                      "[60]",
                      "[0,1200]"),
      maximum_comfortable_level("Maximum comfortable level of the output "
                                "(electric) dynamic range for each "
                                "electrode / cu (same level for all electrodes "
                                "if only one value is specified)",
                                "[600]",
                                "[0,1200]"),
      disabled_electrodes("Indices of any disabled electrodes (0 = most "
                          "apical, 11 = most basal)",
                          "[]",
                          "[0, 11]"),
      stimulation_order("Electrode stimulation order",
                        "random",
                        "[random base_to_apex apex_to_base]"),
      randomization_seed("Seed for randomization of stimulation order (only "
                         "has an effect with random stimulation order)",
                         "1",
                         "[1,["),
      electrodogram_ac(nullptr)
{
    insert_item("weights", &weights);
    insert_item("compression_coefficient", &compression_coefficient);
    insert_item("base_level", &base_level);
    insert_item("saturation_level", &saturation_level);
    insert_item("threshold_level", &threshold_level);
    insert_item("maximum_comfortable_level", &maximum_comfortable_level);
    insert_item("disabled_electrodes", &disabled_electrodes);
    insert_item("stimulation_order", &stimulation_order);
    insert_item("randomization_seed", &randomization_seed);

    patchbay.connect(&weights.writeaccess, this, &Ci_simulation_cis::update_cfg);
    patchbay.connect(&compression_coefficient.writeaccess, this, &Ci_simulation_cis::update_cfg);
    patchbay.connect(&base_level.writeaccess, this, &Ci_simulation_cis::update_cfg);
    patchbay.connect(&saturation_level.writeaccess, this, &Ci_simulation_cis::update_cfg);
    patchbay.connect(&threshold_level.writeaccess, this, &Ci_simulation_cis::update_cfg);
    patchbay.connect(&maximum_comfortable_level.writeaccess, this, &Ci_simulation_cis::update_cfg);
    patchbay.connect(&disabled_electrodes.writeaccess, this, &Ci_simulation_cis::update_cfg);
    patchbay.connect(&stimulation_order.writeaccess, this, &Ci_simulation_cis::update_cfg);
    patchbay.connect(&randomization_seed.writeaccess, this, &Ci_simulation_cis::update_cfg);
}


void Ci_simulation_cis::prepare(mhaconfig_t &signal_info)
{
    if (signal_info.channels != CHANNELS + M_ELECTRODES * 2)
    {
        throw MHA_Error(__FILE__, __LINE__,
                        "The number of input channels on each side must be %u.",
                        CHANNELS+M_ELECTRODES*2);
    }

    if (signal_info.domain != MHA_WAVEFORM)
    {
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin can only process waveform signals.");
    }

    if (signal_info.srate != SRATE)
    {
        throw MHA_Error(__FILE__, __LINE__,
                        "The sampling rate must be %f Hz.",
                        SRATE);
    }

    random_number_generator.seed(randomization_seed.data);

    electrodogram_ac = new MHA_AC::waveform_t(ac,
                                              algo_name,
                                              M_ELECTRODES,
                                              M_ELECTRODES,
                                              true);

    update_cfg();
}


mha_wave_t * Ci_simulation_cis::process(mha_wave_t *signal)
{
    poll_config();

    return cfg->process(signal, this, electrodogram_ac);
}


void Ci_simulation_cis::release()
{
    delete electrodogram_ac;
    electrodogram_ac = nullptr;
}


void Ci_simulation_cis::update_cfg()
{
    if (is_prepared())
    {
        push_config(new Ci_simulation_cis_cfg(weights.data,
                                              compression_coefficient.data,
                                              base_level.data,
                                              saturation_level.data,
                                              threshold_level.data,
                                              maximum_comfortable_level.data,
                                              disabled_electrodes.data,
                                              stimulation_order.data.get_index(),
                                              random_number_generator,
                                              this));
    }
}


MHAPLUGIN_CALLBACKS(ci_simulation_cis, Ci_simulation_cis, wave, wave)

MHAPLUGIN_DOCUMENTATION(ci_simulation_cis, "ci-vocoder",
"This plugin generates an electrodogram from an audio signal, using a "
"stimulation strategy similar to a typical CIS (continuous interleaved "
"sampling) coding strategy with 12 channels. It operates in the time domain.")
