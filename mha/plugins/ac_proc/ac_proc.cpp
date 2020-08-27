// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2015 2018 2019 HörTech gGmbH
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

#include "mha_toolbox.h"
#include "mha_plugin.hh"
#include "mha_fftfb.hh"
#include "mha_events.h"
#include "mhapluginloader.h"
#include "mha_algo_comm.hh"
#include "dc_afterburn.h"
#include "mha_defs.h"

#include <math.h>
#include <fstream>
#include <iostream>
namespace ac_proc {
    class interface_t : public MHAPlugin::plugin_t<int> {
    public:
        interface_t(const algo_comm_t&,
                    const std::string&,
                    const std::string&);
        void prepare(mhaconfig_t&);
        void release();
        void process();
        mha_spec_t* process(mha_spec_t*);
        mha_wave_t* process(mha_wave_t*);
    private:
        std::string algo;//< Storage of plugin name as given by configuration, used for output AC variable
        MHAParser::mhapluginloader_t plug; //< Plugin loader
        MHAParser::string_t input; //< name of AC variable to be used as signal input
        MHAParser::bool_t permute; //< use transposed (true) or original (false) dimension
        MHA_AC::waveform_t* s_out; //< container for return signal, to be stored in AC space
        MHASignal::waveform_t* s_in_perm; //< container for transposition
        bool b_permute; //< copy of transpose flag, static between prepare and release
        mha_wave_t s_in; //< input signal, pointer updated in prepare() and each process() call
    };

    /** \internal

        Default values are set and MHA configuration variables registered into the parser.

        \param ac_     algorithm communication handle
        \param th     chain name
        \param al     algorithm name
    */
    interface_t::interface_t(const algo_comm_t& ac_,
                             const std::string& th,
                             const std::string& al)
        : MHAPlugin::plugin_t<int>("AC variable processor.",ac_),
        algo(al),
        plug(*this,ac),
        input("Name of AC variable to use as input (must exist during prepare)",""),
        permute("Permute AC variable?","no"),
        s_out(nullptr),
        s_in_perm(nullptr),
        b_permute(false)
    {
        insert_member(input);
        insert_member(permute);
    }

    void interface_t::prepare(mhaconfig_t& tf)
    {
        // test if input AC variable is available as waveform:
        s_in = MHA_AC::get_var_waveform(ac,input.data);
        mhaconfig_t cf;
        memset(&cf,0,sizeof(cf));
        // create copy of transpose flag to keep it constant until next release
        b_permute = permute.data;
        if( b_permute ){
            cf.fragsize = s_in.num_channels;
            cf.channels = s_in.num_frames;
        }else{
            cf.fragsize = s_in.num_frames;
            cf.channels = s_in.num_channels;
        }
        // (de-)allocate transposed buffer
        if( s_in_perm )
            delete s_in_perm;
        s_in_perm = new MHASignal::waveform_t(cf.fragsize,cf.channels);
        // calculate new sampling rate:
        cf.srate = (double)tf.srate*(double)cf.fragsize/(double)tf.fragsize;
        // prepare sub-plugin with dimensions based on AC variable:
        plug.prepare(cf);
        if( cf.domain != MHA_WAVEFORM )
            throw MHA_Error(__FILE__,__LINE__,"The sub plugin returned spectrum"
                            " output, currently only waveform is supported.");
        // allocate output variable and insert into AC space:
        if( s_out )
            delete s_out;
        s_out = new MHA_AC::waveform_t(ac,algo,cf.fragsize,cf.channels,true);
    }

    void interface_t::release()
    {
        plug.release();
        if( s_out )
            delete s_out;
        s_out = NULL;
        if( s_in_perm )
            delete s_in_perm;
        s_in_perm = NULL;
    }

    void interface_t::process()
    {
        s_in = MHA_AC::get_var_waveform(ac,input.data);
        if( b_permute ){
            MHA_assert_equal(s_in.num_frames,s_in_perm->num_channels);
            MHA_assert_equal(s_in.num_channels,s_in_perm->num_frames);
            MHASignal::copy_permuted( s_in_perm, &s_in );
            s_in = *s_in_perm;
        }else{
            MHA_assert_equal(s_in.num_frames,s_in_perm->num_frames);
            MHA_assert_equal(s_in.num_channels,s_in_perm->num_channels);
        }
        mha_wave_t* s_proc;
        plug.process(&s_in,&s_proc);
        s_out->copy(s_proc);
        s_out->insert();
    }

    mha_spec_t* interface_t::process(mha_spec_t* s)
    {
        process();
        return s;
    }

    mha_wave_t* interface_t::process(mha_wave_t* s)
    {
        process();
        return s;
    }
}

MHAPLUGIN_CALLBACKS(ac_proc,ac_proc::interface_t,spec,spec)
MHAPLUGIN_PROC_CALLBACK(ac_proc,ac_proc::interface_t,wave,wave)
MHAPLUGIN_DOCUMENTATION(ac_proc,"data-flow algorithm-communication feature-extraction",
                        "This plugin can use the content of a real-valued AC variable (scalar,\n"
                        "vector or matrix) as a time-domain input of a plugin. A sub-graph for\n"
                        "this plugin is created. The return value of the plugin is stored as an\n"
                        "AC variable.\n\n"
                        "A typical usage of this plugin is feature analysis and processing,\n"
                        "e.g., for level compression."
                        )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
