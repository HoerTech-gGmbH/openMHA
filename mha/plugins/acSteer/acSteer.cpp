// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2014 2015 2016 2017 2018 2019 2020 HörTech gGmbH
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
 * Loading the steering vectors into a spec object, each vector corresponding to one channel and the selected vector is saved into the AC space again a a spec object.
 */

#include "acSteer.h"
#include <iostream>

#define PATCH_VAR(var) patchbay.connect(&var.valuechanged, this, &acSteer::update_cfg)
#define INSERT_PATCH(var) insert_member(var); PATCH_VAR(var)

acSteer_config::acSteer_config(algo_comm_t &ac, const mhaconfig_t in_cfg, acSteer *acSteer):
    nchan( in_cfg.channels ),
    nfreq( in_cfg.fftlen/2 + 1 ),
    nsteerchan( acSteer->nsteerchan.data ),
    nrefmic( acSteer->nrefmic.data ),
    nangle( nsteerchan / (nrefmic * nchan) ),
    specSteer1( ac, acSteer->acSteerName1.data.c_str(), nfreq, nchan * nangle, false ),
    specSteer2( ac, acSteer->acSteerName2.data.c_str(), nfreq, nchan * nangle, false )
{
    if ( nsteerchan != nrefmic * nchan * nangle)
        throw MHA_Error(__FILE__, __LINE__,
                        "Steering vectors have %u channels, should have a %u (num. of reference microphones)"
                        " multiple of %u (context channels).\n",
                        nsteerchan, nrefmic, nchan);

    //read in the file where the steering vectors are saved
    ifstream fSteer(acSteer->steerFile.data.c_str());

    if (!fSteer)
        throw MHA_Error(__FILE__, __LINE__,
                        "File (%s) does not exist.\n",
                        acSteer->steerFile.data.c_str());

    for (unsigned int ang = 0; ang < nangle; ang++) {
        for (unsigned int c = 0; c < nchan; c++ ) {
            for (unsigned int f = 0; f < nfreq; f++) {
                mha_complex_t comp;
                    if( fSteer.eof() )
                      throw MHA_Error(__FILE__,__LINE__,"end of file reached before data was complete.");
                fSteer >> comp.re >> comp.im;
                specSteer1(f, c + nchan * ang) = comp;
            }
        }
    }

    if (nrefmic == 2){
        for (unsigned int ang = 0; ang < nangle; ang++) {
            for (unsigned int c = 0; c < nchan; c++ ) {
                for (unsigned int f = 0; f < nfreq; f++) {
                    mha_complex_t comp;
                    if( fSteer.eof() )
                      throw MHA_Error(__FILE__,__LINE__,"end of file reached before data was complete.");
                    fSteer >> comp.re >> comp.im;
                    specSteer2(f, c + nchan * ang) = comp;
                }
            }
        }
    }

    fSteer.close();
}


acSteer_config::~acSteer_config() {}


/** Constructs our plugin. */
acSteer::acSteer(algo_comm_t & ac,
                 const std::string & chain_name,
                 const std::string & algo_name)
    : MHAPlugin::plugin_t<acSteer_config>("Steering Vector Loading Plugin",ac)
    , steerFile("Name of the input file where the steering vectors are saved", "steerfile.bin")
    , acSteerName1("Name of the AC variable where the steering vectors of the first (left) reference microphone are saved", "acSteerLeft")
    , acSteerName2("Name of the AC variable where the steering vectors of the second (right) reference microphone are saved", "acSteerRight")
    , nsteerchan("Number of channels in each steering vector", "4")
    , nrefmic("Number of reference microphones", "1", "]0,2]")
{
    //add parser variables and connect them to methods here

    INSERT_PATCH(steerFile);
    INSERT_PATCH(acSteerName1);
    INSERT_PATCH(acSteerName2);
    INSERT_PATCH(nsteerchan);
    INSERT_PATCH(nrefmic);
}

acSteer::~acSteer() {}

void acSteer_config::insert()
{
  specSteer1.insert();
  specSteer2.insert();
}

/** Plugin preparation.
 *  An opportunity to validate configuration parameters before instantiating a configuration.
 * @param signal_info
 *   Structure containing a description of the form of the signal (domain,
 *   number of channels, frames per block, sampling rate.
 */
void acSteer::prepare(mhaconfig_t & signal_info)
{
    if (signal_info.domain != MHA_SPECTRUM)
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin can only process spectrum signals.");


    /* make sure that a valid runtime configuration exists: */
    update_cfg();
    /* insert is allowed at this place: */
    poll_config()->insert();
}

void acSteer::update_cfg()
{
    if ( is_prepared() ) {

        //when necessary, make a new configuration instance
        //possibly based on changes in parser variables
        acSteer_config *config;
        config = new acSteer_config( ac, input_cfg(), this );
        push_config( config );
    }
}

/**
 * Thos method is a NOOP
 */
mha_spec_t * acSteer::process(mha_spec_t * signal)
{
    //just forward the incoming signal to the next plugin in the chain
    poll_config()->insert();
    return signal;
}

/*
 * This macro connects the plugin1_t class with the MHA plugin C interface
 * The first argument is the class name, the other arguments define the
 * input and output domain of the algorithm.
 */
MHAPLUGIN_CALLBACKS(acSteer,acSteer,spec,spec)

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
(acSteer,
 "data-import disk-files beamformer binaural adaptive",
 "The {\\tt acSteer} plugin loads a file contaning pre-computed steering"
 " filters (e.g. MVDR filters) to be used within a beamformer."
 " The steering filters can be monaural ({\\bf nrefmic = 1})"
 " or binaural ({\\bf nrefmic = 2})."
 " The whole file consists of a column vector of concatenated steering vectors,"
 " which are formatted in the order of {\\bf angle} and {\\bf channel}."
 " This means that the first channel vector of the first angle is followed"
 " by the second channel vector of the first angle until the last channel."
 " The channel vectors of the first angle are followed by the channel vectors"
 " of the second angle and so an and so forth.\n"
 "\n"
 "If the steering filters have been computed for two reference microphones,"
 " the steering filters of the second reference microphone just follow the"
 " ones for the first microphone and have the same format.\n"
 "\n"
 "This plugin is typically located"
 " between a localization plugin (e.g. {\\tt doasvm\\_classification})"
 " and a beamforming plugin (e.g. {\\tt steerbf})."
 " The localization plugin estimates the source direction"
 " and saves it in an AC variable."
 " This plugin reads the saved direction from the corresponding AC variable"
 " and saves the corresponding steering vector to the AC space,"
 " which is used by the succeeding beamforming plugin"
 " for steering the beam towards that particular direction.\n"
 "\n"
 "The configuration variable {\\bf nrefmic} indicates the number of different"
 " reference microphone settings, for which the filters were computed."
 " For each reference microphone and each possible DOA angle and each"
 " input channel one filter should be provided so that\n"
 "\\begin{equation}\n"
 "nsteerchan = nrefmix * nchan * nangle\n"
 "\\end{equation}\n"
 )

/*
 * Local Variables:
 * compile-command: "make"
 * indent-tabs-mode: nil
 * c-basic-offset: 4
 * coding: utf-8-unix
 * End:
 */
