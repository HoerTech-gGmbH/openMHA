// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2018 2019 2020 2021 HörTech gGmbH
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

#include "mha_plugin.hh"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "lsl_cpp.h"
#pragma GCC diagnostic pop

#include <pthread.h>
#include <sched.h>

/** All types for the wave2lsl plugins live in this namespace. */
namespace wave2lsl{

    /** Runtime configuration class of the wave2lsl plugin */
    class cfg_t {
        /** Counter of frames to skip */
        unsigned skipcnt;
        /** Number of frames to skip after each send */
        const unsigned skip;

        /** LSL stream info. */
        lsl::stream_info info;
        /** LSL stream outlet. Interface to lsl */
        lsl::stream_outlet stream;
    public:

        /** C'tor of wave2lsl run time configuration
         * @param skip_          Number of frames to skip after each send
         * @param num_channels_  Number of channels in the LSL stream
         * @param num_samples_   Number of samples within one frame
         * @param source_id_     LSL identifier for this data stream
         * @param varname_       Names of AC variables to send over LSL
         * @param rate           Rate with wich chunks of data are sent to the LSL
         *                       stream.  Usually the rate with which process calls
         *                       happen, but may be lower due to the subsampling
         *                       caused by skip_ */
        cfg_t(unsigned skip_, unsigned num_channels_, unsigned num_samples_,
              const std::string& source_id, const std::string varname_,
              double rate);
        void process(mha_wave_t *s);

    };

    /** Plugin class of wave2lsl */
    class wave2lsl_t : public MHAPlugin::plugin_t<cfg_t>
    {
    public:
        wave2lsl_t(algo_comm_t iac, const std::string & configured_name);
        /** Prepare locks the configuration, then calls update(). */
        void prepare(mhaconfig_t&);
        /** Processing fct for waveforms. Calls process of the cfg class. */
        mha_wave_t* process(mha_wave_t* s);
        /** Release fct. Unlocks the configuration */
        void release();
    private:
        /** Construct new runtime configuration */
        void update();
        MHAParser::string_t name;
        MHAParser::string_t source_id;
        MHAParser::bool_t rt_strict;
        MHAParser::bool_t activate;
        MHAParser::int_t skip;
        MHAEvents::patchbay_t<wave2lsl_t> patchbay;
        bool is_first_run;
    };
}

wave2lsl::wave2lsl_t::wave2lsl_t(algo_comm_t iac, const std::string &)
    : MHAPlugin::plugin_t<wave2lsl::cfg_t>("Generate an LSL stream from the incoming waveform.",iac),
    name("Name of the LSL stream to be generated.","[]"),
    source_id("Unique source id for the stream outlet.",""),
    rt_strict("Abort if used in real-time thread?","yes"),
    activate("Send frames to network?","yes"),
    skip("Number of frames to skip after sending","0","[0,]"),
    is_first_run(true)
{
    insert_member(name);
    insert_member(source_id);
    insert_member(rt_strict);
    insert_member(activate);
    insert_member(skip);
    //Nota bene: Activate should not be connected to the patchbay because we skip processing
    //in the plugin class when necessary. If activate used update() as callback, streams
    //would get recreated everytime activate is toggled.
    patchbay.connect(&source_id.writeaccess,this,&wave2lsl_t::update);
    patchbay.connect(&rt_strict.writeaccess,this,&wave2lsl_t::update);
    patchbay.connect(&skip.writeaccess,this,&wave2lsl_t::update);
    patchbay.connect(&name.writeaccess,this,&wave2lsl_t::update);
}

void wave2lsl::wave2lsl_t::prepare(mhaconfig_t&)
{
    try {
        name.setlock(true);
        rt_strict.setlock(true);

        update();
    }
    catch(MHA_Error& e){
        name.setlock(false);
        rt_strict.setlock(false);
        throw;
    }
}

void wave2lsl::wave2lsl_t::release()
{
    is_first_run=true;
    rt_strict.setlock(false);
    name.setlock(false);
}

mha_wave_t* wave2lsl::wave2lsl_t::process(mha_wave_t* s)
{
    if(is_first_run){
        if(rt_strict.data)
            {
                is_first_run=false;
                pthread_t this_thread=pthread_self();
                int policy=0;
                struct sched_param params;
                auto ret=pthread_getschedparam(this_thread,&policy,&params);
                if(ret != 0)
                    throw MHA_Error(__FILE__,__LINE__,"could not retrieve"
                                    " thread scheduling parameters!");
                if(policy == SCHED_FIFO or policy==SCHED_RR)
                    throw MHA_Error(__FILE__,__LINE__,"wave2lsl used in"
                                    " real-time thread with"
                                    " rt-strict=true!");
            }
    }
    poll_config();
    if(activate.data)
        cfg->process(s);

    return s;
}

void wave2lsl::wave2lsl_t::update(){

    if(is_prepared()){

        auto c=new cfg_t(skip.data,
                         input_cfg().channels,
                         input_cfg().fragsize,
                         source_id.data,
                         name.data,
                         input_cfg().srate/input_cfg().fragsize/(skip.data+1.0f));

        push_config(c);
    }
}


wave2lsl::cfg_t::cfg_t(unsigned skip_, unsigned num_channels_,
                       unsigned num_samples_, const std::string& source_id_,
                       const std::string varname_, double rate_):
    skipcnt(skip_),
    skip(skip_),
    info(varname_, "MHA_AC_MHAREAL", num_channels_, rate_, lsl::cf_float32, source_id_),
    stream(info, num_samples_)
{
}

void wave2lsl::cfg_t::process(mha_wave_t *s){

    if(!skipcnt){
        stream.push_chunk_multiplexed(s->buf, s->num_frames * s->num_channels, 0.0, true);

        skipcnt=skip;
    }
    else{
        skipcnt--;
    }
}


MHAPLUGIN_CALLBACKS(wave2lsl,wave2lsl::wave2lsl_t,wave,wave)
MHAPLUGIN_DOCUMENTATION\
(wave2lsl,
 "data-export network-communication lab-streaming-layer",
 "This plugin provides a mechanism"
 " to send the incoming audio signal over the network using the lab"
 " streaming layer (lsl). The number of channels and the fragment size"
 " in the incoming audio fragment is preserved. \n"
 " If no source id is set,\n"
 " recovery of the stream after changing channel count,\n"
 " data type, or any configuration variable is not possible.\n"
 " Sending data over the network is not real-time safe and\n"
 " processing will be aborted if this plugin is used in a\n"
 " real-time thread without user override.")

/*
 * Local variables:
 * c-basic-offset: 4
 * compile-command: "make"
 * End:
 */
