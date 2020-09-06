// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2009 2010 2013 2017 2020 HörTech gGmbH
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
 * A simple example MHA plugin written in C++
 *
 * This is a modified example 5, using a complex number to scale one
 * channel, instead of a real number.
 *
 * This plugin scales one channel of the input signal, working in the
 * spectral domain. The complex scale factor and the scaled channel
 * number is made accessible to the configuration structure.
 *
 * This plugin implementation uses the C++ macros for standard MHA
 * plugins and a template class for algorithms with a thread safe
 * configuration space. The second item is important if the
 * configuration will be changed at runtime.
 */

#include "mha_plugin.hh"

/*
 * The definition of the signal processing run time configuration. The
 * constructor is used to initialize and validate the run time
 * configuration, based on the general (parser) configuration. If the
 * constructor failes, the parser command which caused the failure
 * will be rejected. The signal processing thread will not be affected.
 *
 * The constructor and destructor are called from the
 * control/configuration thread. All other functions and variables
 * should only be accessed from the signal processing thread.
 */
class cfg_t {
public:
    cfg_t(unsigned int ichannel, 
          unsigned int numchannels,
          const mha_complex_t & iscale);
    unsigned int channel;
    mha_complex_t scale;
};

/*
 * The definition of a simple signal processing class, containing a
 * constructor, a processing method and two variables.
 *
 * The base class MHAParser::parser_t provides the configuration
 * interface, the base class MHAPlugin::plugin_t provides standard
 * methods for MHA plugins (i.e. error handling) and functionality for
 * thread safe runtime configuration.
 *
 * Implementation see below.
 */
class complex_scale_channel_t : public MHAPlugin::plugin_t<cfg_t> {
public:
    complex_scale_channel_t(const algo_comm_t&,
                    const std::string&,
                    const std::string&);
    mha_spec_t* process(mha_spec_t*);
    void prepare(mhaconfig_t&);
private:
    void update_cfg();
    MHAEvents::patchbay_t<complex_scale_channel_t> patchbay;
    /* integer variable of MHA-parser: */
    MHAParser::int_t scale_ch;
    /* complex variable of MHA-parser: */
    MHAParser::complex_t factor;
};

/*
 * Constructor of the runtime configuration class.
 *
 * This constructor is called in the configuration thread. This
 * allowes slow initialization processes (memory allocation,
 * configuration processing) even at run time.
 */
cfg_t::cfg_t(unsigned int ichannel,
             unsigned int numchannels,
             const mha_complex_t & iscale)
    : channel(ichannel),scale(iscale)
{
    if( channel >= numchannels )
        throw MHA_Error(__FILE__,__LINE__,
                        "Invalid channel index %u (only %u channels configured)",
                        channel,numchannels);
}

/*
 * Constructor of the simple signal processing class.
 */
complex_scale_channel_t::complex_scale_channel_t(const algo_comm_t& iac,
                                                 const std::string&,
                                                 const std::string&)
    : MHAPlugin::plugin_t<cfg_t>("example plugin configuration structure",iac),
      scale_ch("index of channel to be scaled","0","[0,["),
      factor("complex scale factor","1.0","],[")
{
    /* Register variables to the configuration parser: */
    insert_item("channel",&scale_ch);
    insert_item("factor",&factor);
    /*
     * On write access to the parser variables a notify callback of
     * this class will be called. That funtion will update the runtime
     * configuration.
     */
    patchbay.connect(&scale_ch.writeaccess,this,
                     &complex_scale_channel_t::update_cfg);
    patchbay.connect(&factor.writeaccess,this,
                     &complex_scale_channel_t::update_cfg);
}

/*
 * Signal processing method. One channel of the input signal is scaled
 * by a factor. The factor and channel number can be changed at any
 * time using the configuration parser of the framework.
 */
mha_spec_t* complex_scale_channel_t::process(mha_spec_t* spec)
{
    poll_config();
    unsigned int fr;
    /* Scale channel at index "scale_ch" by "factor": */
    for(fr = 0; fr < spec->num_frames; fr++){
        spec->buf[fr + cfg->channel * spec->num_frames] *= cfg->scale;
    }
    return spec;
}

/*
 * Handle the prepare request of the framework. After this callback
 * was called, a valid run time configuration has to be
 * available. Domain and channel numbers of input and output are
 * passed to the algorithm and can be modified by the algorithm. If
 * not modified by the algorithm, than the output and input parameters
 * are equal. The default domain depends on the output domain of
 * preceeding algorithms.
 */
void complex_scale_channel_t::prepare(mhaconfig_t& cfg)
{
    /* spectral input is needed, spectral data is returned: */
    if( cfg.domain != MHA_SPECTRUM )
        throw MHA_ErrorMsg("complex_scale_channel: Only spectral processing is supported.");
    /* remember the transform configuration (i.e. channel numbers): */
    tftype = cfg;
    /* make sure that a valid runtime configuration exists: */
    update_cfg();
}

/*
 * This function adds a valid runtime configuration. In this case, the
 * channel number has to be configured for a valid configuration (see
 * prepare() callback).
 */
void complex_scale_channel_t::update_cfg()
{
    if( tftype.channels )
        push_config(new cfg_t(scale_ch.data,tftype.channels,factor.data));
}

/*
 * This macro wraps the required ANSI-C interface around the algorithm
 * class. The first argument is the algorithm class name, the other
 * arguments define the input and output domain of the algorithm.
 */
MHAPLUGIN_CALLBACKS(complex_scale_channel,complex_scale_channel_t,spec,spec)
MHAPLUGIN_DOCUMENTATION(complex_scale_channel,"testing","")

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
