// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2010 2012 2013 2014 2015 2016 2017 HörTech gGmbH
// Copyright © 2018 2019 2020 HörTech gGmbH
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

#include "mha_generic_chain.h"

mhachain::chain_base_t::chain_base_t(algo_comm_t iac,const std::string& ichain,const std::string & ialgo)
    : MHAPlugin::plugin_t<mhachain::plugs_t>("MHA Chain",iac),
      bprofiling("Profile the loaded plugins. Needs to be set to true before setting algos.",
                 "no"),
      algos("List of plugins to load and arrange in a signal processing chain.  Entries\n"
            "are separated by spaces and given in the order of the signal processing.\n"
            "Please refer to the detailed description of this plugin in the plugin manual\n"
            "for more details.", "[]"),
      b_prepared(false),
      chain(ialgo)
{
    set_node_id( "mhachain" );
    patchbay.connect(&algos.writeaccess,this,&chain_base_t::update);
    update();
}

void mhachain::chain_base_t::update()
{
    for(unsigned int k=0;k<old_algos.size();k++){
        MHAParser::expression_t cfgname(old_algos[k], ":");
        if(!cfgname.rval.size())
            cfgname.rval = cfgname.lval;
        force_remove_item(cfgname.rval);
    }
    old_algos = algos.data;
    push_config(new plugs_t(algos.data,
                            cfin,
                            cfout,
                            b_prepared,
                            *this,
                            ac,
                            chain,bprofiling.data));
    if( !b_prepared )
        poll_config();
}

void mhachain::chain_base_t::prepare(mhaconfig_t& cf)
{
    poll_config();
    if( cfg->prepared() )
        throw MHA_ErrorMsg("mhachain: plugins are allready prepared.");
    cfin = cf;
    cfg->prepare(cf);
    cfout = cf;
    b_prepared = true;
}

void mhachain::chain_base_t::release()
{
    b_prepared = false;
    poll_config();
    if( !(cfg->prepared()) )
        throw MHA_ErrorMsg("mhachain: plugins are not prepared.");
    cfg->release();
    cleanup_unused_cfg();
}

void mhaconfig_compare(mhaconfig_t req, mhaconfig_t avail,const char* cpref)
{
    std::string pref;
    if(cpref)
        pref = cpref;
    if(req.channels != avail.channels)
        throw MHA_Error(__FILE__, __LINE__,"%s: %u channels required, %u available.",
                        pref.c_str(), req.channels, avail.channels);
    if(req.domain != avail.domain)
        throw MHA_Error(__FILE__, __LINE__,"%s: domain %s required, %s available.",
                        pref.c_str(), PluginLoader::mhastrdomain(req.domain), PluginLoader::mhastrdomain(avail.domain));
    if(req.fragsize != avail.fragsize)
        throw MHA_Error(__FILE__, __LINE__,"%s: a fragsize of %u samples required, %u available.",
                        pref.c_str(), req.fragsize, avail.fragsize);
    if(req.fftlen != avail.fftlen)
        throw MHA_Error(__FILE__, __LINE__,"%s: a FFT length of %u samples required, %u available.",
                        pref.c_str(), req.fftlen, avail.fftlen);
    if(req.wndlen != avail.wndlen)
        throw MHA_Error(__FILE__, __LINE__,"%s: a window length of %u samples required, %u available.",
                        pref.c_str(), req.wndlen, avail.wndlen);
    if(req.srate != avail.srate)
        throw MHA_Error(__FILE__, __LINE__,"%s: a sample rate of %g Hz required, %g Hz available.",
                        pref.c_str(), req.srate, avail.srate);
}

mhachain::plugs_t::plugs_t(std::vector<std::string> algos,
                           mhaconfig_t cfin,
                           mhaconfig_t cfout,
                           bool do_prepare,
                           MHAParser::parser_t& p,
                           algo_comm_t iac,
                           std::string ichain,
                           bool use_profiling)
    : b_prepared(false),
      parser(p),
      ac(iac),
      chain(ichain),
      profiling("sub-plugin profiling information"),
      prof_algos("names of algorithms"),
      prof_init("time of init callback / seconds"),
      prof_prepare("time of prepare callback / seconds"),
      prof_release("time of release callback / seconds"),
      prof_process("cumulative time of process callback / seconds"),
      prof_process_tt("total processed signal time / seconds"),
      prof_process_load("load of process callback / percent"),
      prof_load_con(&prof_process_load.prereadaccess,this,&mhachain::plugs_t::update_proc_load),
      prof_tt_con(&prof_process_tt.prereadaccess,this,&mhachain::plugs_t::update_proc_load),
      b_use_profiling(use_profiling)
{
    profiling.insert_item("algos",&prof_algos);
    profiling.insert_item("init",&prof_init);
    profiling.insert_item("prepare",&prof_prepare);
    profiling.insert_item("release",&prof_release);
    profiling.insert_item("process",&prof_process);
    profiling.insert_item("process_tt",&prof_process_tt);
    profiling.insert_item("process_load",&prof_process_load);
    profiling.set_node_id("chain_profiler");
    if( b_use_profiling ){
        prof_algos.data = algos;
        prof_init.data.resize(algos.size());
        prof_prepare.data.resize(algos.size());
        prof_release.data.resize(algos.size());
        prof_process.data.resize(algos.size());
        parser.force_remove_item("profiling");
        parser.insert_member(profiling);
    }
    try{
        alloc_plugs(algos);
        if( do_prepare ){
            prepare(cfin);
            mhaconfig_compare(cfout,cfin,"mhachain");
        }
    }
    catch(MHA_Error&e ){
        cleanup_plugs();
        throw e;
    }
}

void mhachain::plugs_t::update_proc_load()
{
    prof_process_load.data = prof_process.data;
    prof_process_tt.data = (float)proc_cnt*(float)prof_cfg.fragsize/prof_cfg.srate;
    mha_real_t t_sum = 0;
    for(unsigned int k=0;k<prof_process_load.data.size();k++)
        t_sum += prof_process_load.data[k];
    for(unsigned int k=0;k<prof_process_load.data.size();k++)
        prof_process_load.data[k] *= 100.0f/prof_process_tt.data;
}

void mhachain::plugs_t::alloc_plugs(std::vector<std::string> algonames)
{
    if( algos.size() )
        throw MHA_ErrorMsg("mhachain: The algos are not empty. This is a fatal bug.");
    for( unsigned int k=0;k<algonames.size();k++){
        if( b_use_profiling )
            mha_platform_tic(&tictoc);
        algos.push_back(new PluginLoader::mhapluginloader_t(ac,algonames[k]));
        parser.force_remove_item(algos.back()->get_configname());
        if( algos.back()->has_parser() )
            parser.insert_item(algos.back()->get_configname(),algos.back());
        if( b_use_profiling )
            prof_init.data[k] = mha_platform_toc(&tictoc);
    }
}

void mhachain::plugs_t::cleanup_plugs()
{
    for( unsigned int k=0;k<algos.size();k++){
        parser.remove_item(algos[k]);
        delete algos[k];
    }
    algos.clear();
}

void mhachain::plugs_t::prepare(mhaconfig_t& tf)
{
    proc_cnt = 0;
    prof_cfg = tf;
    unsigned int k, kmax = 0;
    try{
        for(k=0;k<algos.size();k++){
            kmax = k;
            if( b_use_profiling )
                mha_platform_tic(&tictoc);
            algos[k]->prepare(tf);
            if( b_use_profiling ){
                prof_prepare.data[k] = mha_platform_toc(&tictoc);
                prof_process.data[k] = 0;
            }
        }
        b_prepared = true;
    }
    catch(MHA_Error& e){
        for(k=0;k<kmax;k++){
            algos[k]->release();
        }
        throw e;
    }
}

void mhachain::plugs_t::release()
{
    b_prepared = false;
    for(unsigned int k=0;k<algos.size();k++){
        if( b_use_profiling )
            mha_platform_tic(&tictoc);
        algos[k]->release();
        if( b_use_profiling )
            prof_release.data[k] = mha_platform_toc(&tictoc);
    }
}

void mhachain::plugs_t::process(mha_wave_t* win,mha_spec_t* sin,mha_wave_t** wout,mha_spec_t** sout)
{
    proc_cnt++;
    mha_wave_t* wv = win;
    mha_spec_t* sp = sin;
    for(unsigned int k=0;k<algos.size();k++){
        if( b_use_profiling )
            mha_platform_tic(&tictoc);
        switch( algos[k]->input_domain() ){
        case MHA_WAVEFORM :
            switch( algos[k]->output_domain() ){
            case MHA_WAVEFORM :
                algos[k]->process(wv,&wv);
                break;
            case MHA_SPECTRUM :
                algos[k]->process(wv,&sp);
                break;
            }
            break;
        case MHA_SPECTRUM :
            switch( algos[k]->output_domain() ){
            case MHA_WAVEFORM :
                algos[k]->process(sp,&wv);
                break;
            case MHA_SPECTRUM :
                algos[k]->process(sp,&sp);
                break;
            }
            break;
        }
        if( b_use_profiling )
            prof_process.data[k] += mha_platform_toc(&tictoc);
    }
    if( wout )
        *wout = wv;
    if( sout )
        *sout = sp;
}

mhachain::plugs_t::~plugs_t()
{
    if( prepared() )
        release();
    cleanup_plugs();
    parser.force_remove_item("profiling");
}

void mhachain::chain_base_t::process(mha_wave_t* sin,mha_wave_t** sout)
{
    poll_config();
    cfg->process(sin,NULL,sout,NULL);
}

void mhachain::chain_base_t::process(mha_spec_t* sin,mha_wave_t** sout)
{
    poll_config();
    cfg->process(NULL,sin,sout,NULL);
}

void mhachain::chain_base_t::process(mha_wave_t* sin,mha_spec_t** sout)
{
    poll_config();
    cfg->process(sin,NULL,NULL,sout);
}

void mhachain::chain_base_t::process(mha_spec_t* sin,mha_spec_t** sout)
{
    poll_config();
    cfg->process(NULL,sin,NULL,sout);
}

// Local Variables:
// indent-tabs-mode: nil
// c-basic-offset: 4
// coding: utf-8-unix
// compile-command: "make"
// End:
