// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2010 2012 2013 2014 2015 2017 2018 HörTech gGmbH
// Copyright © 2019 2020 HörTech gGmbH
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
 * A simple example \mha plugin written in C++
 *
 * This plugin scales one channel of the input signal, working in the
 * spectral domain. The scale factor and the scaled channel number is made
 * accessible to the configuration structure.
 *
 * This plugin implementation uses the C++ macros for standard \mha
 * plugins and a template class for algorithms with a thread safe
 * configuration space. The second item is important if the
 * configuration will be changed at runtime.
 */

#include <stdio.h>



#ifdef _WIN32
#include <windows.h>
/*
 * On some MS Windows compilers, the function "snprintf" is defined as
 * "_snprintf": 
 */
#define snprintf _snprintf
#else
/*
 * On MS Windows functions have to be exported with
 * __declspec(dllexport) so we define a dummy for other OS (like
 * Linux)
 */
#define __declspec(p)
#endif


// MicroSoft Visual C++ has problems with it's own min and max macros...
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include "mha_plugin.hh"
#include "mha_events.h"

/*
 * The definition of the signal processing run time configuration. The
 * constructor is used to initialize and validate the run time
 * configuration, based on the general (parser) configuration. If the
 * constructor failes, the parser command which caused the failure
 * will be rejected. The signal processing thread will not be affected.
 *
 * The constructor and destructor are called from the
 * control/configuration thread. All other functions and variables
 * should only be accessed from the signal processing thread. Read
 * only access from the configuration thread is possible if the read
 * operation is atomic.
 */
class example5_t {
public:
    example5_t(unsigned int,unsigned int,mha_real_t);
    mha_spec_t* process(mha_spec_t*);
private:
    unsigned int channel;
    mha_real_t scale;
};

/*
 * The definition of a simple signal processing class, containing a
 * constructor, a processing method and two variables.
 *
 * The base class MHAParser::parser_t provides the configuration
 * interface, the base class MHAPlugin::plugin_t provides standard
 * methods for \mha plugins (i.e. error handling) and functionality for
 * thread safe runtime configuration.
 *
 * Implementation see below.
 */
class plugin_interface_t : public MHAPlugin::plugin_t<example5_t> {
public:
    plugin_interface_t(const algo_comm_t&,const std::string&,const std::string&);
    mha_spec_t* process(mha_spec_t*);
    void prepare(mhaconfig_t&);
private:
    void update_cfg();
    /* integer variable of MHA-parser: */
    MHAParser::int_t scale_ch;
    /* float variable of MHA-parser: */
    MHAParser::float_t factor;
    /* patch bay for connecting configuration parser 
       events with local member functions: */
    MHAEvents::patchbay_t<plugin_interface_t> patchbay;
};

/*
 * Constructor of the runtime configuration class.
 *
 * This constructor is called in the configuration thread. This
 * allowes slow initialization processes (memory allocation,
 * configuration processing) even at run time.
 */
example5_t::example5_t(unsigned int ichannel,
                       unsigned int numchannels,
                       mha_real_t iscale)
    : channel(ichannel),scale(iscale)
{
    if( channel >= numchannels )
        throw MHA_Error(__FILE__,__LINE__,
                        "Invalid channel number %u (only %u channels configured).",
                        channel,numchannels);
}

/*
 * Signal processing method. One channel of the input signal is scaled
 * by a factor. The factor and channel number can be changed at any
 * time using the configuration parser of the framework.
 */
mha_spec_t* example5_t::process(mha_spec_t* spec)
{
    /* Scale channel number "scale_ch" by "factor": */
    for(unsigned int fr = 0; fr < spec->num_frames; fr++){
        spec->buf[fr + channel * spec->num_frames].re *= scale;
        spec->buf[fr + channel * spec->num_frames].im *= scale;
    }
    return spec;
}

/*
 * Constructor of the simple signal processing class.
 */
plugin_interface_t::plugin_interface_t(
    const algo_comm_t& iac,
    const std::string&,const std::string&)
    : MHAPlugin::plugin_t<example5_t>("example plugin scaling a spectral signal",iac),
      /* initialzing variable 'scale_ch' with MHAParser::int_t(char* name, .... ) */
      scale_ch("channel number to be scaled","0","[0,["),
      /* initialzing variable 'factor' with MHAParser::float_t(char* name, .... ) */
      factor("scale factor","1.0","[0,2]")
{
    /* Register variables to the configuration parser: */
    insert_item("channel",&scale_ch);
    insert_item("factor",&factor);
    /*
     * On write access to the parser variables a notify callback of
     * this class will be called. That funtion will update the runtime
     * configuration.
     */
    patchbay.connect(&scale_ch.writeaccess,this,&plugin_interface_t::update_cfg);
    patchbay.connect(&factor.writeaccess,this,&plugin_interface_t::update_cfg);
}

/*
 * Signal processing method of the interface class. The new
 * configuration is polled and the processing passed to the new
 * processing.
 */
mha_spec_t* plugin_interface_t::process(mha_spec_t* spec)
{
    poll_config();
    return cfg->process(spec);
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
void plugin_interface_t::prepare(mhaconfig_t& tfcfg)
{
    if( tfcfg.domain != MHA_SPECTRUM )
        throw MHA_Error(__FILE__,__LINE__,
                        "Example5: Only spectral processing is supported.");
    /* remember the transform configuration (i.e. channel numbers): */
    tftype = tfcfg;
    /* make sure that a valid runtime configuration exists: */
    update_cfg();
}

/*
 * This function adds a valid runtime configuration. In this case, the
 * channel number has to be configured for a valid configuration (see
 * prepare() callback).
 */
void plugin_interface_t::update_cfg()
{
    if( tftype.channels )
        push_config(new example5_t(scale_ch.data,tftype.channels,factor.data));
}

/*
 * This macro wraps the required ANSI-C interface around the algorithm
 * class. The first argument is the algorithm class name, the other
 * arguments define the input and output domain of the algorithm.
 */
MHAPLUGIN_CALLBACKS(example5,plugin_interface_t,spec,spec)
MHAPLUGIN_DOCUMENTATION\
(example5,
 "example level-modification audio-channels",
 "This plugin scales one channel of the input signal,"
 " working in the spectral domain.\n"
 "The scale factor and the scaled channel number"
 " is made accessible to the configuration structure."
)

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
