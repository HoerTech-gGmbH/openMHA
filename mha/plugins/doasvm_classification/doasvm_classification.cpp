// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2014 2015 2016 2018 2019 2020 HörTech gGmbH
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
 * This plugin computes the probabilities given the cross-correlation
 * matrix as an AC variable. It needs the parameters of the SVM, which
 * are from a configuration file.
 */

#include "doasvm_classification.h"

#define PATCH_VAR(var) patchbay.connect(&var.valuechanged, this, &doasvm_classification::update_cfg)
#define INSERT_PATCH(var) insert_member(var); PATCH_VAR(var)

doasvm_classification_config::doasvm_classification_config(algo_comm_t &ac, const mhaconfig_t in_cfg, doasvm_classification *_doasvm):
    ac(ac),
    doasvm(_doasvm),
    p(ac, _doasvm->p_name.data.c_str(), _doasvm->angles.data.size(), 1, true),
    p_max(ac, _doasvm->max_p_ind_name.data.c_str(), (_doasvm->angles.data.size() - 1) / 2)
{
    //initialize plugin state for a new configuration
    c.num_frames = _doasvm->w.data.size(); // number of rows of w
    if( (c.buf = new mha_real_t[c.num_frames]) == 0 )
        throw MHA_Error(__FILE__, __LINE__,
                        "Error initialising c.buf.");
 }

doasvm_classification_config::~doasvm_classification_config() {delete c.buf;}

//the actual processing implementation
mha_wave_t *doasvm_classification_config::process(mha_wave_t *wave)
{
    //do actual processing here using configuration state
    const mha_wave_t vGCC = MHA_AC::get_var_waveform(ac, doasvm->vGCC_name.data.c_str());

    // Apply linear SVM model (one for each direction) to feature vector
    for( size_t i = 0; i < doasvm->w.data.size(); ++i ) {
        c.buf[i] = doasvm->b.data[i];
        for( size_t j = 0; j < doasvm->w.data[i].size(); ++j )
            c.buf[i] += doasvm->w.data[i][j]*vGCC.buf[j];
    }

    for( unsigned int i = 0; i < p.num_frames; ++i )
        p(i, 0) =  1/(1 + std::exp(-(doasvm->x.data[i] + c.buf[i]*doasvm->y.data[i])));


    // map to probability using a sigmoid transformation
    // find the max probability
    mha_real_t max = 0;
    int  max_ind = -1;
    for( unsigned int i = 0; i < p.num_frames; ++i ) {
        p(i, 0) =  1/(1 + std::exp(-(doasvm->x.data[i] + c.buf[i]*doasvm->y.data[i])));


        // Find the direction with the maximum probability
        if (max < p.value(i, 0)) {
            max = p.value(i, 0);
            max_ind = i;

        }
    }

    p_max.data = max_ind;


    //return current fragment
    return wave;
}

/** Constructs our plugin. */
doasvm_classification::doasvm_classification(algo_comm_t & ac,
                                             const std::string & chain_name,
                                             const std::string & algo_name)
    : MHAPlugin::plugin_t<doasvm_classification_config>("Support vector machine (SVM) plugin for computing the direction of arrival (DOA) probabilities",ac)
    , angles("The angles for which the SVM model has been trained", "[]")
    , w("The separation planes of the model.", "[[]]")
    , b("The model bias.", "[]")
    , x("The sigmoid probability mapping parameter x.", "[]")
    , y("The sigmoid probability mapping parameter y.", "[]")
    , p_name("The name of the AC variable for the vector of probabilities of the DOA estimation.", "p")
    , max_p_ind_name("The name of the AC variable for the index of the maximum probability of the DOA estimation", "p_max")
    , vGCC_name("The name of the AC variable for the GCC matrix, which is computed by another plugin", "vGCC_ac")
{
    // make the plug-in findable via "?listid"
    set_node_id(algo_name);

    //add parser variables and connect them to methods here
    //INSERT_PATCH(foo_parser);

    INSERT_PATCH(angles);
    INSERT_PATCH(w);
    INSERT_PATCH(b);
    INSERT_PATCH(x);
    INSERT_PATCH(y);
    INSERT_PATCH(p_name);
    INSERT_PATCH(max_p_ind_name);
    INSERT_PATCH(vGCC_name);
}

doasvm_classification::~doasvm_classification() {}

/** Plugin preparation.
 *  An opportunity to validate configuration parameters before instantiating a configuration.
 * @param signal_info
 *   Structure containing a description of the form of the signal (domain,
 *   number of channels, frames per block, sampling rate.
 */
void doasvm_classification::prepare(mhaconfig_t & signal_info)
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

    if( signal_info.domain != MHA_WAVEFORM )
        throw MHA_Error(__FILE__, __LINE__,
                        "This plug-in requires time-domain signals.");

    if( signal_info.channels != 2 )
        throw MHA_Error(__FILE__, __LINE__,
                        "This input signal must have exactly 2 channels, not %u.",
                        signal_info.channels);

    /* make sure that a valid runtime configuration exists: */
    update_cfg();
}

void doasvm_classification::update_cfg()
{
    if ( is_prepared() ) {

        //when necessary, make a new configuration instance
        //possibly based on changes in parser variables
        doasvm_classification_config *config;
        config = new doasvm_classification_config( ac, input_cfg(), this );
        push_config( config );
    }
}

/**
 * Checks for the most recent configuration and defers processing to it.
 */
mha_wave_t * doasvm_classification::process(mha_wave_t * signal)
{
    //this stub method defers processing to the configuration class
    return poll_config()->process( signal );
}

/*
 * This macro connects the plugin1_t class with the MHA plugin C interface
 * The first argument is the class name, the other arguments define the
 * input and output domain of the algorithm.
 */
MHAPLUGIN_CALLBACKS(doasvm_classification,doasvm_classification,wave,wave)

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
(doasvm_classification,
 "spatial classifier binaural",
 "This plugin loads the parameters of a pre-trained SVM and computes"
 " the probabilities for given range of directions of arrival (DOA).\n"
 "These probabilities take a value within the interval of $[0,1]$."
 " Higher probability for a certain DOA indicates higher possibility"
 " of a source coming from that particular DOA."
 )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
