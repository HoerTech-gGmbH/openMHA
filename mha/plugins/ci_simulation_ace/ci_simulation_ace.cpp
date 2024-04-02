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


#include "ci_simulation_ace.hh"

#include <chrono>


const unsigned int CHANNELS = 1;
const unsigned int FFTLEN = 128;
const mha_real_t SRATE = 16000;
const unsigned int M_ELECTRODES = 22;
const std::vector<unsigned int> BIN_INDICES =
    { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 22, 25, 29, 33, 38, 43,
      49, 56, 64 };
const std::vector<mha_real_t> WEIGHTS =
    { 0.98, 0.98, 0.98, 0.98, 0.98, 0.98, 0.98, 0.98, 0.98, 0.68, 0.68, 0.68,
      0.68, 0.65, 0.65, 0.65, 0.65, 0.65, 0.65, 0.65, 0.65, 0.65 };


Ci_simulation_ace_cfg::Ci_simulation_ace_cfg(unsigned int n_electrodes,
                                             mha_real_t compression_coefficient,
                                             mha_real_t base_level,
                                             mha_real_t saturation_level,
                                             std::vector<mha_real_t> threshold_level,
                                             std::vector<mha_real_t> comfort_level,
                                             std::vector<int> disabled_electrodes,
                                             unsigned int stimulation_order,
                                             std::default_random_engine random_number_generator)
    : n_electrodes_cfg(n_electrodes),
      compression_coefficient_cfg(compression_coefficient),
      base_level_cfg(base_level),
      saturation_level_cfg(saturation_level),
      threshold_level_cfg(threshold_level),
      comfort_level_cfg(comfort_level),
      disabled_electrodes_cfg(disabled_electrodes),
      stimulation_order_cfg(stimulation_order),
      random_number_generator_cfg(random_number_generator)
{
    if (n_electrodes_cfg > M_ELECTRODES)
    {
        throw MHA_Error(__FILE__, __LINE__,
                        "At most %u electrodes can be stimulated.",
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

    if (threshold_level_cfg.size() == 1 && comfort_level_cfg.size() == 1)
    {
        for (unsigned int m = 0; m < M_ELECTRODES; m++)
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
        if (comfort_level_cfg.size() != M_ELECTRODES)
        {
            throw MHA_Error(__FILE__, __LINE__,
                            "Comfort level must have either 1 or %u elements.",
                            M_ELECTRODES);
        }
        for (unsigned int m = 0; m < M_ELECTRODES; m++)
        {
            if (comfort_level_cfg.at(m) <= threshold_level_cfg.at(0))
            {
                throw MHA_Error(__FILE__, __LINE__,
                                "Comfort level (%f) must be greater than "
                                "threshold level (%f) for electrode %u.",
                                comfort_level_cfg.at(m),
                                threshold_level_cfg.at(0), m);
            }
        }
    }
    else if (threshold_level_cfg.size() > 1 && comfort_level_cfg.size() == 1)
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
            if (comfort_level_cfg.at(0) <= threshold_level_cfg.at(m))
            {
                throw MHA_Error(__FILE__, __LINE__,
                                "Comfort level (%f) must be greater than "
                                "threshold level (%f) for electrode %u.",
                                comfort_level_cfg.at(0),
                                threshold_level_cfg.at(m), m);
            }
        }
    }
    else if (threshold_level_cfg.size() > 1 && comfort_level_cfg.size() > 1)
    {
        if (threshold_level_cfg.size() != M_ELECTRODES)
        {
            throw MHA_Error(__FILE__, __LINE__,
                            "Threshold level must have either 1 or %u "
                            "elements.",
                            M_ELECTRODES);
        }
        if (comfort_level_cfg.size() != M_ELECTRODES)
        {
            throw MHA_Error(__FILE__, __LINE__,
                            "Comfort level must have either 1 or %u elements.",
                            M_ELECTRODES);
        }
        for (unsigned int m = 0; m < M_ELECTRODES; m++)
        {
            if (comfort_level_cfg.at(m) <= threshold_level_cfg.at(m))
            {
                throw MHA_Error(__FILE__, __LINE__,
                                "Comfort level (%f) must be greater than "
                                "threshold level (%f) for electrode %u.",
                                comfort_level_cfg.at(m),
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
}


// Process function of the runtime configuration class (main signal processing
// function). It leaves the input signal fragment unchanged; its purpose is to
// compute the AC variable electrodogram_ac for further processing in the
// plugin chain:

mha_spec_t * Ci_simulation_ace_cfg::process(mha_spec_t *signal,
                                            MHA_AC::waveform_t *electrodogram_ac)
{
    // Envelope extraction:

    // Vector of pairs (one for each filterbank frequency band, i.e. electrode)
    // for storing the envelope along with the corresponding frequency band
    // index:
    std::vector<std::pair<mha_real_t, unsigned int>> envelope(M_ELECTRODES);




    // Loop over all frequency bands:
    for (unsigned int m = 0; m < M_ELECTRODES; m++)
    {
        // Loop over all FFT bin indices for the respective frequency band:
        for (unsigned int bin = BIN_INDICES.at(m); bin < BIN_INDICES.at(m+1); bin++)
        {
            // Compute the weighted sum of the FFT bin powers:
            envelope.at(m).first += WEIGHTS.at(m) * abs2(value(signal, bin, 0));
        }

        // Compute the envelope (= square root of the weighted sum of the FFT
        // bin powers):
        envelope.at(m).first = sqrt(envelope.at(m).first);

        // Register the corresponding frequency band index:
        envelope.at(m).second = m;

        // Set the envelope to zero if the corresponding electrode is disabled:
        if (std::find(std::begin(disabled_electrodes_cfg),
                      std::end(disabled_electrodes_cfg), m) != std::end(disabled_electrodes_cfg)) {
            envelope.at(m).first = 0.0;
        }
    }


    // Electrode selection (n of m):

    // Select the n electrodes with the greatest envelope magnitude by
    // partially sorting the envelopes from greatest to least, so that the n
    // greatest values are at the beginning of the vector:
    std::nth_element(std::begin(envelope),
                     std::begin(envelope)+n_electrodes_cfg-1,
                     std::end(envelope),
                     std::greater<std::pair<mha_real_t, unsigned int>>());


    // Loudness growth function:

    // Loop over the n greatest envelopes (= active electrodes):
    for (unsigned int n = 0; n < n_electrodes_cfg; n++)
    {
        // If the envelope is less than or equal to the base level:
        if (envelope.at(n).first <= base_level_cfg)
        {
            // Set the envelope to the minimum value:
            envelope.at(n).first = 0.0;
        }
        // Else, if the envelope is greater than or equal to the saturation
        // level:
        else if (envelope.at(n).first >= saturation_level_cfg)
        {
            // Set the envelope to the maximum value:
            envelope.at(n).first = 1.0;
        }
        // Else:
        else
        {
            // Apply the loudness growth function to the envelope, mapping the
            // (acoustic) envelope magnitude to compressed electrical magnitude:
            envelope.at(n).first =
                log(1 + compression_coefficient_cfg * (envelope.at(n).first - base_level_cfg)/
                                                      (saturation_level_cfg - base_level_cfg))/
                log(1 + compression_coefficient_cfg);
        }
    }


    // Mapping of electrical dynamic range to clinical current units (CU):

    // Loop over the n greatest envelopes (= active electrodes):
    for (unsigned int n = 0; n < n_electrodes_cfg; n++)
    {
        // If threshold level and comfort level are both scalars:
        if (threshold_level_cfg.size() == 1 && comfort_level_cfg.size() == 1)
        {
            // Convert the electrical magnitudes to clinical current units:
            envelope.at(n).first = threshold_level_cfg.at(0) +
                ((comfort_level_cfg.at(0) - threshold_level_cfg.at(0)) *
                 (envelope.at(n).first));
        }
        // Else, if threshold level is a scalar and comfort level is a vector:
        else if (threshold_level_cfg.size() == 1 && comfort_level_cfg.size() > 1)
        {
            // Convert the electrical magnitudes to clinical current units:
            envelope.at(n).first = threshold_level_cfg.at(0) +
                ((comfort_level_cfg.at(envelope.at(n).second) - threshold_level_cfg.at(0)) *
                 (envelope.at(n).first));
        }
        // Else, if threshold level is a vector and comfort level is a scalar:
        else if (threshold_level_cfg.size() > 1 && comfort_level_cfg.size() == 1)
        {
            // Convert the electrical magnitudes to clinical current units:
            envelope.at(n).first = threshold_level_cfg.at(envelope.at(n).second) +
            ((comfort_level_cfg.at(0) - threshold_level_cfg.at(envelope.at(n).second)) *
             (envelope.at(n).first));
        }
        // Else, if threshold level and comfort level are both vectors:
        else if (threshold_level_cfg.size() > 1 && comfort_level_cfg.size() > 1)
        {
            // Convert the electrical magnitudes to clinical current units:
            envelope.at(n).first = threshold_level_cfg.at(envelope.at(n).second) +
            ((comfort_level_cfg.at(envelope.at(n).second) - threshold_level_cfg.at(envelope.at(n).second)) *
             (envelope.at(n).first));
        }
    }


    // Electrodogram generation:

    // If the electrode stimulation order is random:
    if (stimulation_order_cfg == 0)
    {
        // Randomly shuffle the order of the n greatest elements in the
        // envelope vector of pairs (the random number generator must be
        // seeded only once, in the prepare function of the plugin
        // interface class):
        shuffle(std::begin(envelope),
                std::begin(envelope)+n_electrodes_cfg-1,
                random_number_generator_cfg);
    }
    // Else, if the electrode stimulation order is base-to-apex:
    else if (stimulation_order_cfg == 1)
    {
        // Sort the n greatest elements in the envelope vector of pairs from
        // greatest to least electrode index, using a lambda expression:
        std::sort(std::begin(envelope), std::begin(envelope)+n_electrodes_cfg,
                  [](const std::pair<mha_real_t, unsigned int> &a,
                     const std::pair<mha_real_t, unsigned int> &b) ->
                  bool { return (a.second > b.second); });
    }
    // Else, if the electrode stimulation order is apex-to-base:
    else if (stimulation_order_cfg == 2)
    {
        // Sort the n greatest elements in the envelope vector of pairs from
        // least to greatest electrode index, using a lambda expression:
        std::sort(std::begin(envelope), std::begin(envelope)+n_electrodes_cfg,
                  [](const std::pair<mha_real_t, unsigned int> &a,
                     const std::pair<mha_real_t, unsigned int> &b) ->
                  bool { return (a.second < b.second); });
    }

    // Initialize the electrodogram with zeros:
    electrodogram_ac->assign(0.0);

    // Loop over the n greatest envelopes (= active electrodes):
    for (unsigned int n = 0; n < n_electrodes_cfg; n++)
    {
        // For each "frame" (i.e. time slot) of the electrodogram, set the
        // value for the respective "channel" (i.e. electrode index) to the
        // corresponding envelope value:
        electrodogram_ac->value(n, envelope.at(n).second) = envelope.at(n).first;
    }

    // Return the (unchanged) output signal fragment:
    return signal;
}


Ci_simulation_ace::Ci_simulation_ace(algo_comm_t &ac,
                                     const std::string &algo_name)
    : MHAPlugin::plugin_t<Ci_simulation_ace_cfg>("CI simulation (ACE)",
                                                 ac),
      algo_name(algo_name),
      n_electrodes("Number of active electrodes (cannot be changed at runtime)",
                   "8",
                   "[1,["),
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
      comfort_level("Comfort level of the output (electric) dynamic range "
                    "for each electrode / CU (same level for all electrodes if "
                    "only one value is specified)",
                    "[160]",
                    "[0,255]"),
      disabled_electrodes("Indices of any disabled electrodes (0 = most "
                          "apical, 21 = most basal)",
                          "[]",
                          "[0, 21]"),
      stimulation_order("Electrode stimulation order",
                        "random",
                        "[random base_to_apex apex_to_base]"),
      randomization_seed("Seed for randomization of stimulation order (only "
                         "has an effect with random stimulation order)",
                         "1",
                         "[1,["),
      electrodogram_ac(nullptr)
{
    insert_item("n_electrodes", &n_electrodes);
    insert_item("compression_coefficient", &compression_coefficient);
    insert_item("base_level", &base_level);
    insert_item("saturation_level", &saturation_level);
    insert_item("threshold_level", &threshold_level);
    insert_item("comfort_level", &comfort_level);
    insert_item("disabled_electrodes", &disabled_electrodes);
    insert_item("stimulation_order", &stimulation_order);
    insert_item("randomization_seed", &randomization_seed);

    patchbay.connect(&n_electrodes.writeaccess, this, &Ci_simulation_ace::update_cfg);
    patchbay.connect(&compression_coefficient.writeaccess, this, &Ci_simulation_ace::update_cfg);
    patchbay.connect(&base_level.writeaccess, this, &Ci_simulation_ace::update_cfg);
    patchbay.connect(&saturation_level.writeaccess, this, &Ci_simulation_ace::update_cfg);
    patchbay.connect(&threshold_level.writeaccess, this, &Ci_simulation_ace::update_cfg);
    patchbay.connect(&comfort_level.writeaccess, this, &Ci_simulation_ace::update_cfg);
    patchbay.connect(&disabled_electrodes.writeaccess, this, &Ci_simulation_ace::update_cfg);
    patchbay.connect(&stimulation_order.writeaccess, this, &Ci_simulation_ace::update_cfg);
    patchbay.connect(&randomization_seed.writeaccess, this, &Ci_simulation_ace::update_cfg);
}


void Ci_simulation_ace::prepare(mhaconfig_t &signal_info)
{
    if (signal_info.channels != CHANNELS)
    {
        throw MHA_Error(__FILE__, __LINE__,
                        "The number of input channels on each side must be %u.",
                        CHANNELS);
    }

    if (signal_info.domain != MHA_SPECTRUM)
    {
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin can only process signals in the spectral "
                        "domain.");
    }

    if (signal_info.wndlen != 2 * input_cfg().fragsize)
    {
        throw MHA_Error(__FILE__, __LINE__,
                        "The window length (%u) must be 2 * fragment size (%u) = %u samples.",
                        signal_info.wndlen,
                        input_cfg().fragsize,
                        2 * input_cfg().fragsize);
    }

    if (signal_info.fftlen != FFTLEN)
    {
        throw MHA_Error(__FILE__, __LINE__,
                        "The FFT length must be %u bins.",
                        FFTLEN);
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
                                              n_electrodes.data,
                                              M_ELECTRODES,
                                              true);

    update_cfg();

    // Lock the configuration variable against write access:
    n_electrodes.setlock(true);
}


mha_spec_t * Ci_simulation_ace::process(mha_spec_t *signal)
{
    poll_config();

    return cfg->process(signal, electrodogram_ac);
}


void Ci_simulation_ace::release()
{
    // Unlock the configuration variable:
    n_electrodes.setlock(false);

    delete electrodogram_ac;
    electrodogram_ac = nullptr;
}


void Ci_simulation_ace::update_cfg()
{
    if (is_prepared())
    {
        push_config(new Ci_simulation_ace_cfg(n_electrodes.data,
                                              compression_coefficient.data,
                                              base_level.data,
                                              saturation_level.data,
                                              threshold_level.data,
                                              comfort_level.data,
                                              disabled_electrodes.data,
                                              stimulation_order.data.get_index(),
                                              random_number_generator));
    }
}


MHAPLUGIN_CALLBACKS(ci_simulation_ace, Ci_simulation_ace, spec, spec)

MHAPLUGIN_DOCUMENTATION(ci_simulation_ace, "ci-vocoder",
"This plugin generates an electrodogram from an audio signal, using a "
"stimulation strategy similar to a typical ACE (advanced combination encoder, "
"n-of-m) coding strategy with 22 channels. It operates in the spectral domain.")
