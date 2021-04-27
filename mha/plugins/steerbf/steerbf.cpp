// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2014 2015 2016 2017 2018 2019 2021 HörTech gGmbH
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

#include "steerbf.h"

#define PATCH_VAR(var) patchbay.connect(&var.valuechanged, this, &steerbf::update_cfg)
#define INSERT_PATCH(var) insert_member(var); PATCH_VAR(var)

steerbf_config::steerbf_config(algo_comm_t &ac, const mhaconfig_t in_cfg,
                               steerbf *steerbf) :
  nchan( in_cfg.channels ),
    nfreq( in_cfg.fftlen/2 + 1 ),
    outSpec( nfreq, 1 ), //mono output
    bf_vec(  ),
    //set nangle by counting/inferring number of blocks
    nangle( bf_vec.num_channels / nchan ),
    _steerbf( steerbf ), ac(ac),
  bf_src_copy( steerbf->bf_src.data )
{
    //set the correct upper limit given data
    steerbf->angle_ind.set_max_angle_ind( nangle-1 );
}

steerbf_config::~steerbf_config() {}

/* live processing class */
mha_spec_t *steerbf_config::process(mha_spec_t *inSpec)
{
    bf_vec = MHA_AC::get_var_spectrum(ac, bf_src_copy );
    //if angle_src is set, then retrieve steering from AC variable
    //otherwise use the configuration variable
    int angle_ind;
    if ( _steerbf->angle_src.data.compare("") != 0 ) {
        // angle_ind = MHA_AC::get_var_int(ac, _steerbf->angle_src.data );
        const mha_wave_t angle_ind_wave = MHA_AC::get_var_waveform(ac, _steerbf->angle_src.data);
        angle_ind = (int)value(angle_ind_wave, 0, 0);
    }
    else {
        angle_ind = _steerbf->angle_ind.data;
    }
    int block_ind = angle_ind*nchan;

    // std::cout << _steerbf->bf_src.data << std::endl;

    //do the filtering and summing
    for (unsigned int f=0; f<nfreq; f++) {

        // std::cout << "freq: " << f << std::endl;
        //init output to zero
        outSpec(f,0).re = 0;
        outSpec(f,0).im = 0;

        for (unsigned int m=0; m<nchan; ++m) {
            //outSpec(f,0) += _conjugate((*beamW)(f,m)) * value(inSpec,f,m);
            outSpec(f,0) += _conjugate(value(bf_vec,f,m+block_ind)) * value(inSpec,f,m);
            // std::cout << "c: " << m << ": " << value(bf_vec,f,m+block_ind).re << " " << value(bf_vec,f,m+block_ind).im << std::endl;
        }
    }

    return &outSpec;
}

/** Constructs our plugin. */
steerbf::steerbf(algo_comm_t iac, const std::string &)
    : MHAPlugin::plugin_t<steerbf_config>("Steerable Beamformer",iac),
      bf_src("Provides the beamforming filters encoded as a block matrix: [chanXnangle,nfreq].", ""),
      angle_ind("Sets the steering angle in filtering.", "0", "[0,1000]"),
      angle_src("If initialized, provides an int-AC variable of steering index.","")
{
    //only make a new configuration when bf_src changes
    INSERT_PATCH(bf_src);

    //otherwise, the processing plugins query for the current angles
    insert_member(angle_ind);
    insert_member(angle_src);
}

steerbf::~steerbf() {}

/** Plugin preparation.
 *  An opportunity to validate configuration parameters before instantiating a configuration.
 * @param signal_info
 *   Structure containing a description of the form of the signal (domain,
 *   number of channels, frames per block, sampling rate.
 */
void steerbf::prepare(mhaconfig_t & signal_info)
{
    //good idea: restrict input type and dimension
    /*
    if (signal_info.channels != 2)
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin must have 2 input channels: (%d found)\n"
                        "[Left, Right].", signal_info.channels);
                        */
    if (signal_info.domain != MHA_SPECTRUM)
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin can only process spectrum signals.");

    //set output dimension
    signal_info.channels = 1;

    /* make sure that a valid runtime configuration exists: */
    update_cfg();
}

void steerbf::update_cfg()
{
    if ( is_prepared() ) {

        //when necessary, make a new configuration instance
        //possibly based on changes in parser variables
        steerbf_config *config;
        config = new steerbf_config( ac, input_cfg(), this );
        push_config( config );
    }
}

/**
 * Defers to configuration class.
 */
mha_spec_t * steerbf::process(mha_spec_t * signal)
{
    //this stub method defers processing to the configuration class
    return poll_config()->process( signal );
}

/*
 * This macro connects the plugin1_t class with the MHA plugin C interface
 * The first argument is the class name, the other arguments define the
 * input and output domain of the algorithm.
 */
MHAPLUGIN_CALLBACKS(steerbf,steerbf,spec,spec)

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
(steerbf,
 "filter spatial audio-channels beamformer binaural",
 "Implements frequency-domain beamformer processing (filter and sum) using"
 " externally provided filters. "
 "A plugin called {\\tt acSteer} can be used to provide the filter coefficients. "
 "The filter coefficients to be read are saved as a waveform object in the AC space. "
 "Each channel of this object corresponds to a different steering angle."
 " The steering angle is typically determined in real-time by a"
 " localization plugin (e.g. {\\tt doasvm\\_classification}). "
 "In this case, the index to the corresponding steering direction is read"
 " from the AC space."
 " Note that the number of available filters should be consistent with"
 " the number of possible steering directions to be estimated."
 " The configuration variable \\textbf{angle\\_src} keeps the name of the"
 " AC variable for the estimated steering direction. "
 "The steering angle can also be fixed in the configuration time using the"
 " configuration variable \\textbf{angle\\_ind}."
 )


/*
 * Local Variables:
 * compile-command: "make"
 * indent-tabs-mode: nil
 * c-basic-offset: 4
 * coding: utf-8-unix
 * End:
 */
