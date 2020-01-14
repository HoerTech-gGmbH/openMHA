// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2010 2013 2014 2015 2017 2018 HörTech gGmbH
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
 * This plugin calculates the RMS level of a given channel of the
 * input signal, working in the time domain. The channel number is
 * made accessible to the configuration structure, the result is
 * stored into a algorithm communication variable (AC variable).
 *
 * This plugin implementation uses the C++ macros for standard \mha
 * plugins and a template class for algorithms with a thread safe
 * configuration space. The second item is important if the
 * configuration will be changed at runtime.
 */

#include <stdio.h>
#include <math.h>
 
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
class cfg_t {
public:
    cfg_t(unsigned int,unsigned int);
    unsigned int channel;
};

/*
 * The definition of a simple signal processing class, containing a
 * constructor, a processing method and a parser variable.
 *
 * The base class MHAParser::parser_t provides the configuration
 * interface, the base class MHAPlugin::plugin_t provides standard
 * methods for \mha plugins (i.e. error handling) and functionality for
 * thread safe runtime configuration and algorithm communication.
 *
 * Implementation see below.
 */
class example6_t : public MHAPlugin::plugin_t<cfg_t> {
public:
    example6_t(const algo_comm_t&,const std::string&,const std::string&);
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
private:
    void update_cfg();
    /* integer variable of MHA-parser: */
    MHAParser::int_t channel_no;
    /* RMS level result variable (propagated to AC handle): */
    float rmsdb;
    MHAEvents::patchbay_t<example6_t> patchbay;
};

/*
 * This macro wraps the required ANSI-C interface around the algorithm
 * class. The first argument is the algorithm class name, the other
 * arguments define the input and output domain of the algorithm.
 */
MHAPLUGIN_CALLBACKS(example6,example6_t,wave,wave)

/*
 * Constructor of the runtime configuration class.
 *
 * This constructor is called in the configuration thread. This
 * allowes slow initialization processes (memory allocation,
 * configuration processing) even at run time.
 */
cfg_t::cfg_t(unsigned int ichannel,
             unsigned int numchannels)
    : channel(ichannel)
{
    if( channel >= numchannels )
        throw MHA_Error(__FILE__,__LINE__,
                        "Invalid channel number %u (only %u channels configured).",
                        channel,numchannels);
}

/*
 * Constructor of the simple signal processing class.
 */
example6_t::example6_t(const algo_comm_t& iac,
                       const std::string&,const std::string&)
    : MHAPlugin::plugin_t<cfg_t>("Example rms level meter plugin",iac),
      /* initialzing variable 'channel_no' with MHAParser::int_t(char* name, .... ) */
      channel_no("channel in which the RMS level is measured","0","[0,[")
{
    /* Register variables to the configuration parser: */
    insert_item("channel",&channel_no);
    /*
     * On write access to the parser variables a notify callback of
     * this class will be called. That funtion will update the runtime
     * configuration.
     */
    patchbay.connect(&channel_no.writeaccess,this,&example6_t::update_cfg);
    /*
     * Propagate the level variable to all algorithms in the
     * processing chain. If multiple instances of this algorithm are
     * required, than it is necessary to use different names for this
     * variable (i.e. prefixing the name with the algorithm name
     * passed to MHAInit).
     */
    ac.insert_var_float( ac.handle, "example6_rmslev", &rmsdb );
}

/*
 * Signal processing method. One channel of the input signal is scaled
 * by a factor. The factor and channel number can be changed at any
 * time using the configuration parser of the framework.
 */
mha_wave_t* example6_t::process(mha_wave_t* wave)
{
    poll_config();
    unsigned int fr;

    rmsdb = 0;
    /* calculate RMS level in dB of channel "channel": */
    for(fr = 0; fr < wave->num_frames; fr++){
        rmsdb += (wave->buf[fr * wave->num_channels + cfg->channel] *
                  wave->buf[fr * wave->num_channels + cfg->channel]);
    }
    /* average across one signal chunk: */
    rmsdb /= wave->num_frames;
    /* lower limit is -100 dB: */
    if( rmsdb < 1e-10 )
        rmsdb = 1e-10;
    rmsdb = 10*log10( rmsdb );
    return wave;
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
void example6_t::prepare(mhaconfig_t& tfcfg)
{
    if( tfcfg.domain != MHA_WAVEFORM )
        throw MHA_Error(__FILE__,__LINE__,"Example6: Only waveform processing is supported.");
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
void example6_t::update_cfg()
{
    if( tftype.channels )
        push_config(new cfg_t(channel_no.data,tftype.channels));
}

MHAPLUGIN_DOCUMENTATION\
(example6,
 "example feature-extraction algorithm-communication",
 "This plugin calculates the RMS level of a given channel of the input signal,"
 " working in the time domain.\n"
 "The channel number is made accessible to the configuration structure and\n"
 "the result is stored into a algorithm communication variable (AC variable).")

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
