// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2007 2008 2009 2010 2011 2013 2014 2015 2018 2019 HörTech gGmbH
// Copyright © 2020 2021 HörTech gGmbH
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

#define MHAPLUGIN_OVERLOAD_OUTDOMAIN
#include "mha_plugin.hh"
#include "mhapluginloader.h"
#include "mha_defs.h"
#include "mha_algo_comm.hh"
#include "mha_windowparser.h"

class mhaplug_cfg_t : private MHAKernel::algo_comm_class_t, public PluginLoader::mhapluginloader_t
{
public:
    mhaplug_cfg_t(algo_comm_t iac,const std::string& libname,bool use_own_ac);
    ~mhaplug_cfg_t() throw () {};
};

mhaplug_cfg_t::mhaplug_cfg_t(algo_comm_t iac,const std::string& libname,bool use_own_ac)
    : PluginLoader::mhapluginloader_t((use_own_ac?get_c_handle():iac),libname)
{
}

class altplugs_t : public MHAPlugin::plugin_t<MHAWindow::fun_t>
{
public:
    altplugs_t(algo_comm_t iac, const std::string & configured_name);
    void prepare(mhaconfig_t&);
    void release();
    void process(mha_wave_t*,mha_wave_t**);
    void process(mha_spec_t*,mha_wave_t**);
    void process(mha_wave_t*,mha_spec_t**);
    void process(mha_spec_t*,mha_spec_t**);
    virtual std::string parse(const std::string& arg);
    virtual void parse(const char* a1,char* a2,unsigned int a3)
    { MHAPlugin::plugin_t<MHAWindow::fun_t>::parse(a1,a2,a3); }
private:
    void event_set_plugs();
    void event_add_plug();
    void event_delete_plug();
    void event_select_plug();
    void update_selector_list();
    void update_ramplen();
    void proc_ramp(mha_wave_t* s);
    MHAParser::bool_t use_own_ac;
    MHAParser::vstring_t parser_plugs;
    MHAParser::string_t add_plug;
    MHAParser::string_t delete_plug;
    MHAParser::float_t ramplen;
    MHAParser::kw_t select_plug;
    // dummy parser, only used to fill entries list:
    MHAParser::parser_t current;
    MHAParser::vstring_mon_t nondefault_labels;
    std::vector<mhaplug_cfg_t*> plugs;
    mhaplug_cfg_t* selected_plug;
    MHAEvents::patchbay_t<altplugs_t> patchbay;
    MHASignal::waveform_t* fallback_wave;
    MHASignal::spectrum_t* fallback_spec;
    mhaconfig_t cfin;
    mhaconfig_t cfout;
    bool prepared;
    bool added_via_plugs;
    unsigned int ramp_counter;
    unsigned int ramp_len;
};

altplugs_t::altplugs_t(algo_comm_t iac, const std::string &)
    : MHAPlugin::plugin_t<MHAWindow::fun_t>("Configure alternative plugins.",iac),
      use_own_ac("Use own AC space for each plug (yes), or share parents space (no). Must be set before plugs.","no"),
      parser_plugs("List of plugins","[]"),
      add_plug("Add a plugin into list",""),
      delete_plug("Delete a plugin from list",""),
      ramplen("Ramp length in seconds","0","[0,]"),
      select_plug("Select a plugin for processing","(none)","[(none)]"),
      nondefault_labels("List of plugin labels."),
      selected_plug(NULL),
      fallback_wave(NULL),
      fallback_spec(NULL),
      prepared(false),
      added_via_plugs(false),
      ramp_counter(0),
      ramp_len(0)
{
    set_node_id( "altplugs" );
    insert_member(use_own_ac);
    insert_item("plugs",&parser_plugs);
    insert_item("add",&add_plug);
    insert_item("delete",&delete_plug);
    insert_member(ramplen);
    insert_item("select",&select_plug);
    insert_item("labels",&nondefault_labels);
    //insert_member(current);
    patchbay.connect(&parser_plugs.writeaccess,this,&altplugs_t::event_set_plugs);
    patchbay.connect(&add_plug.writeaccess,this,&altplugs_t::event_add_plug);
    patchbay.connect(&delete_plug.writeaccess,this,&altplugs_t::event_delete_plug);
    patchbay.connect(&select_plug.writeaccess,this,&altplugs_t::event_select_plug);
}

void altplugs_t::proc_ramp(mha_wave_t* s)
{
    if( ramp_counter ){
        poll_config();
        unsigned int k=0;
        while( ramp_counter && (k < s->num_frames) ){
            if( cfg->num_frames > ramp_counter ){
                for(unsigned int ch=0;ch < s->num_channels; ch++)
                    value(s,k,ch) *= cfg->buf[ramp_counter];
            }else{
                for(unsigned int ch=0;ch < s->num_channels; ch++)
                    value(s,k,ch) = 0.0f;
            }
            ramp_counter--;
            k++;
        }
    }
}

void altplugs_t::process(mha_wave_t* sIn,mha_wave_t** sOut)
{
    if( selected_plug )
        selected_plug->process(sIn,sOut);
    else
        *sOut = fallback_wave;
    proc_ramp(*sOut);
}

void altplugs_t::process(mha_spec_t* sIn,mha_wave_t** sOut)
{
    if( selected_plug )
        selected_plug->process(sIn,sOut);
    else
        *sOut = fallback_wave;
    proc_ramp(*sOut);
}

void altplugs_t::process(mha_wave_t* sIn,mha_spec_t** sOut)
{
    if( selected_plug )
        selected_plug->process(sIn,sOut);
    else
        *sOut = fallback_spec;
}

void altplugs_t::process(mha_spec_t* sIn,mha_spec_t** sOut)
{
    if( selected_plug )
        selected_plug->process(sIn,sOut);
    else
        *sOut = fallback_spec;
}

void altplugs_t::prepare(mhaconfig_t& cf)
{
    cfin = cf;
    cfout = cf; // initialization for the no-plugins case
    for(unsigned int k=0;k<plugs.size();k++){
        cf = cfin;
        try{
            plugs[k]->prepare(cf);
        }
        catch(...){
            for(unsigned int kin=0;kin<k;kin++)
                plugs[kin]->release();
            throw;
        }
        if( k==0 ){
            cfout = cf;
        }else{
            PluginLoader::mhaconfig_compare(cfout,cf,plugs[k]->get_configname());
        }
    }
    tftype = cfout;
    fallback_wave = new MHASignal::waveform_t(cfout.fragsize,cfout.channels);
    fallback_spec = new MHASignal::spectrum_t(cfout.fftlen/2+1,cfout.channels);
    prepared = true;
    update_ramplen();
}

void altplugs_t::release()
{
    for(unsigned int k=0;k<plugs.size();k++){
        plugs[k]->release();
    }
    delete fallback_wave;
    delete fallback_spec;
    prepared = false;
}

void altplugs_t::update_ramplen()
{
    ramp_len = static_cast<unsigned>(ramplen.data*input_cfg().srate);
    push_config(new MHAWindow::fun_t(ramp_len,MHAWindow::hanning,0.0f,1.0f));
}

void altplugs_t::event_add_plug()
{
    if( add_plug.data.size() ){
        mhaplug_cfg_t* plug;
        plug = new mhaplug_cfg_t(ac,add_plug.data,use_own_ac.data);
        try{
            if( prepared ){
                mhaconfig_t cf(cfin);
                plug->prepare(cf);
                PluginLoader::mhaconfig_compare(cfout,cf,plug->get_configname());
            }
            if( plug->has_parser() )
                insert_item(plug->get_configname(),plug);
            if( !added_via_plugs )
                parser_plugs.data.push_back(add_plug.data);
            plugs.push_back(plug);
        }
        catch(...){
            delete plug;
            throw;
        }
    }
    add_plug.data = "";
    update_selector_list();
}

void altplugs_t::event_delete_plug()
{
    mhaplug_cfg_t* plug(NULL);
    std::string oname;
    for(unsigned int k=0;k<plugs.size();k++){
        if( plugs[k]->get_configname() == delete_plug.data ){
            plug = plugs[k];
            oname = plug->get_origname();
            plugs.erase(plugs.begin()+k);
            if( plug == selected_plug ){
                select_plug.data.set_index(0);
                selected_plug = NULL;
            }
            force_remove_item(plug->get_configname());
            delete_plug.data = "";
            delete plug;
            for(unsigned int klist=0;klist<parser_plugs.data.size();klist++){
                if( parser_plugs.data[klist] == oname ){
                    parser_plugs.data.erase(parser_plugs.data.begin()+klist);
                    break;
                }
            }
            break;
        }
    }
    update_selector_list();
}

void altplugs_t::event_select_plug()
{
    mhaplug_cfg_t* plug(NULL);
    for(unsigned int k=0;k<plugs.size();k++){
        if( plugs[k]->get_configname() == select_plug.data.get_value() )
            plug = plugs[k];
    }
    selected_plug = plug;
    ramp_counter = ramp_len;
}

void altplugs_t::event_set_plugs()
{
    added_via_plugs = true;
    try{
        for(unsigned int k=0;k<parser_plugs.data.size();k++)
            add_plug.parse("="+parser_plugs.data[k]);
        added_via_plugs = false;
    }
    catch(...){
        added_via_plugs = false;
        throw;
    }
}

void altplugs_t::update_selector_list()
{
    nondefault_labels.data.clear();
    std::vector<std::string> plist;
    plist.push_back("(none)");
    for(unsigned int k=0;k<plugs.size();k++){
        plist.push_back(plugs[k]->get_configname());
        nondefault_labels.data.push_back(plugs[k]->get_configname());
    }
    std::string splist(MHAParser::StrCnv::val2str(plist));
    select_plug.data.set_entries(splist);
}

std::string altplugs_t::parse(const std::string& arg)
{
    MHAParser::expression_t x(arg,".=?");
    if( x.lval == "current" ){
        if( selected_plug ){
            return selected_plug->parse(x.op+x.rval);
        }else
            throw MHA_ErrorMsg("No plugin is selected (current is invalid)!");
    }
    return MHAPlugin::plugin_t<MHAWindow::fun_t>::parse(arg);
}


MHAPLUGIN_CALLBACKS(altplugs,altplugs_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(altplugs,altplugs_t,spec,spec)
MHAPLUGIN_PROC_CALLBACK(altplugs,altplugs_t,spec,wave)
MHAPLUGIN_PROC_CALLBACK(altplugs,altplugs_t,wave,spec)
MHAPLUGIN_DOCUMENTATION\
(altplugs,
 "plugin-arrangement data-flow",
 "The plugin {\\tt altplugs} allows configuration of alternative plugins.\n"
 "Plugins can either be registered en-bloc via the {\\tt plugs} variable or one by one"
 " by repeated assignment to the {\\tt add} variable. Plugins can be removed via {\\tt delete}.\n"
 " Registered plugins are configured as sub parsers of {\\tt altplugs}. "
 "The plugin to be used for processing can be selected via the {\\tt select}"
 " variable at any time. If the plugin output is in the time domain the newly selected plugin"
 " can optionally be faded in, {\\tt ramplen} controlling the ramp length, the old plugin "
 " is always switched off instantaneously. \n"
 "Any plugins can be used as alternative plugins, with the only limitations"
 " that input and output domain and signal dimension is equal for all"
 " alternative plugins.\n"
 "Plugins can renamed using the \":\" operator.\n\n"
 "A module for the {\\tt mhacontrol} graphical user interface is provided."
 )


/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * compile-command: "make"
 * End:
 */
