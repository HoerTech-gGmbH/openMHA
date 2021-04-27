// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2015 2016 2018 2019 2021 HörTech gGmbH
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

/*
 * This plugin has been implemented as part of the multi-channel DOA-SVM localization system consisting of the plugins doasvm_feature_extraction, doasvm_classification, acPooling_wave and acTransform_wave, but can be used in arbitrary contexts for concatenating two or more waveforms.
 */

#include "acConcat_wave.h"

#define PATCH_VAR(var) patchbay.connect(&var.valuechanged, this, &acConcat_wave::update_cfg)
#define INSERT_PATCH(var) insert_member(var); PATCH_VAR(var)

acConcat_wave_config::acConcat_wave_config(algo_comm_t &ac, const mhaconfig_t in_cfg, acConcat_wave *_concat):
    ac(ac),
    strNames_AC(_concat->num_AC.data, _concat->prefix_names_AC.data)
{
    numSamples_AC = _concat->samples_AC.data;

    for (int i = 0; i < _concat->num_AC.data; i++) {
        stringstream ss;
        ss << i + 1;
        strNames_AC[i] += "_" + ss.str();
        ss.flush();
    }

    //initialize plugin state for a new configuration

    int numsamples = 0;
    for(std::vector<int>::iterator it = numSamples_AC.begin(); it != numSamples_AC.end(); ++it)
        numsamples += *it;

    vGCC_con = new MHA_AC::waveform_t(ac, _concat->name_con_AC.data.c_str(), numsamples, _concat->numchannels.data, true);
}

acConcat_wave_config::~acConcat_wave_config()
{
    delete vGCC_con;
}

//the actual processing implementation
mha_wave_t *acConcat_wave_config::process(mha_wave_t *wave)
{
    int sample = 0;
    int chan_sample = 0;

    //do actual processing here using configuration state
    for (std::vector<std::string>::iterator it = strNames_AC.begin() ; it != strNames_AC.end(); ++it) {

        vGCC = MHA_AC::get_var_waveform(ac, (*it).c_str());
        if (vGCC.num_channels != vGCC_con->num_channels)
            throw MHA_Error(__FILE__, __LINE__, "Number of channels does not match.");

        for (unsigned int chan = 0; chan < vGCC.num_channels; chan++) {
            chan_sample = sample;

            for (unsigned int s = 0; s < vGCC.num_frames; s++)
                vGCC_con->assign(chan_sample++, chan, value(vGCC, s, chan));
        }

        sample = chan_sample;
    }

    //return current fragment
    return wave;
}

/** Constructs our plugin. */
acConcat_wave::acConcat_wave(algo_comm_t iac, const std::string &)
    : MHAPlugin::plugin_t<acConcat_wave_config>("Concatenating two or more waveforms into one", iac)
    , num_AC("Number of waveforms to be concatenated", "15", "[1, 28]")
    , prefix_names_AC("Prefix of the names of the waveforms to be concatenated", "vGCC_ac")
    , samples_AC("Lengths of the waveforms to be concatenated", "[]")
    , name_con_AC("Name of the concatenated waveform", "vGCC_con_AC")
    , numchannels("Number of channels in each waveform to be concatenated", "1", "[1,[")
{
    INSERT_PATCH(num_AC);
    INSERT_PATCH(prefix_names_AC);
    INSERT_PATCH(samples_AC);
    INSERT_PATCH(name_con_AC);
    INSERT_PATCH(numchannels);
}

acConcat_wave::~acConcat_wave() {}

/** Plugin preparation.
 *  An opportunity to validate configuration parameters before instantiating a configuration.
 * @param signal_info
 *   Structure containing a description of the form of the signal (domain,
 *   number of channels, frames per block, sampling rate.
 */
void acConcat_wave::prepare(mhaconfig_t & signal_info)
{
    //good idea: restrict input type and dimension
    /*
    if (signal_info.channels != 2)
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin must have 2 input channels: (%d found)\n"
                        "[Left, Right].", signal_info.channels);

    if (signal_info.domain != MHA_SPECTRUM)
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin can only process spectrum signals.");
                        */

    /* make sure that a valid runtime configuration exists: */
    update_cfg();
}

void acConcat_wave::update_cfg()
{
    if ( is_prepared() ) {

        //when necessary, make a new configuration instance
        //possibly based on changes in parser variables
        acConcat_wave_config *config;
        config = new acConcat_wave_config( ac, input_cfg(), this );
        push_config( config );
    }
}

/**
 * Checks for the most recent configuration and defers processing to it.
 */
mha_wave_t * acConcat_wave::process(mha_wave_t * signal)
{
    //this stub method defers processing to the configuration class
    return poll_config()->process( signal );
}

/*
 * This macro connects the plugin1_t class with the MHA plugin C interface
 * The first argument is the class name, the other arguments define the
 * input and output domain of the algorithm.
 */
MHAPLUGIN_CALLBACKS(acConcat_wave,acConcat_wave,wave,wave)

/*
 * This macro creates code classification of the plugin and for
 * automatic documentation.
 *
 * The first argument to the macro is a space separated list of
 * categories, starting with the most relevant category. The second
 * argument is a LaTeX-compatible character array with some detailed
 * documentation of the plugin.
 */
MHAPLUGIN_DOCUMENTATION\
(acConcat_wave,
 "data-flow algorithm-communication",
        "This plugin concatenates two or more waveforms in the given order into a new waveform all living in the AC space. The waveforms to be concatenated as well as the concatenated waveforms must have the same number of channels. However the lengths of the waveforms to be concatenated may differ.\n"
        "\n"
        "The waveforms to be concatenated should have been created in advance by some other plugin. This plugin creates an AC variable for the concatenated waveform and puts it into the AC space.\n"
        "\n"
        "The configuration variable {\\bf num\\_AC} defines the number of waveforms, which will be concatenated into one waveform. The waveforms to be concatenated obey the same naming convention followed by a numeric suffix defining the order of concatenation. This order begins with 1 and a whole in the numeric order is not allowed. This naming convention is defined by the user by setting the configuration variable {\\bf prefix\\_names\\_AC}. The lengths of each waveform to be concatenated is defined by the configuration variable {\\bf samples\\_AC}. This vector variable must contain an integer value corresponding to each of the waveforms to be concatenated defining their lengths respectively. The name of the concatenated waveform is defined by the configuration variable {\\bf name\\_con\\_AC}.\n"
        "\n"
        "This plugin is typically used together with the plugins {\\tt doasvm\\_feature\\_extraction} instantiated several times for computing the cross correlation between all combinations of input channels and with {\\tt doasvm\\_classification}, which uses the concatenated cross correlation vectors for each channel combination to estimate the arrival direction of audio signals.\n"
        "\n"
        "As an example, if there are six waveforms, which are supposed to be concatenated into one waveform, this plugin can be configured as shown in the following:\n"
        "\n"
        "\\begin{verbatim}\n"
        "acConcat_wave.num_AC = 6\n"
        "acConcat_wave.samples_AC = [161 17 161 161 17 161]\n"
        "acConcat_wave.prefix_names_AC = \"vGCC\"\n"
        "acConcat_wave.name_con_AC = \"vGCCcon\"\n"
        "\\end{verbatim}\n"
        "\n"
        "In this case, the six waveforms to be concatenated should be called {\\tt vGCC\\_1, vGCC\\_2, vGCC\\_3, vGCC\\_4, vGCC\\_5} and {\\tt vGCC\\_6}. Note that in the localization context, for a setup of four microphones, there are six different combinations of two microphones."
 )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
