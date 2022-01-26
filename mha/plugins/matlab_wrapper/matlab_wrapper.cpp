// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2020 2021 HörTech gGmbH
// Copyright © 2022 Hörzentrum Oldenburg gGmbH
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

#include "matlab_wrapper.hh"
#include "emxAPI.h"
#include "emxutil.h"
#include "emxutil2.hh"
#include "mha.hh"
#include "mha_signal.hh"

matlab_wrapper::callback::callback(matlab_wrapper_t* parent_,
                                   user_config_t* user_config_,
                                   MHAParser::mfloat_t* var_):
    user_config(user_config_),
    var(var_),
    parent(parent_){}

void matlab_wrapper::callback::on_writeaccess(){
    // Check if size changed
    // We do not need to check for consistent row length, the parser already does that
    if(static_cast<size_t>(user_config->value->size[0])!=var->data.size() ||
       std::any_of(var->data.begin(),var->data.end(),[&](auto k){return k.size()!=static_cast<size_t>(user_config->value->size[1]);})){
        user_config->value->size[0]=var->data.size();
        user_config->value->size[1]=var->data.begin()->size();
        emxEnsureCapacity_real_T(user_config->value,user_config->value->size[0]*user_config->value->size[1]);
     }
    int c=0;
    for(int k=0;k<user_config->value->size[0];++k){
        for(int j=0;j<user_config->value->size[1];++j){
            user_config->value->data[c++]=var->data[k][j];
        }
    }
    parent->update();
}

matlab_wrapper::matlab_wrapper_rt_cfg_t::matlab_wrapper_rt_cfg_t(emxArray_user_config_t* user_config_) {
    user_config=c_argInit_Unboundedx1_user_conf(user_config_->size[0]);
    for(int i=0;i<user_config_->size[0];++i){
        emxCopyStruct_user_config_t(&user_config->data[i],&user_config_->data[i]);
    }
}

matlab_wrapper::matlab_wrapper_rt_cfg_t::~matlab_wrapper_rt_cfg_t(){
    emxDestroyArray_user_config_t(user_config);
}

matlab_wrapper::matlab_wrapper_t::wrapped_plugin_t::wrapped_plugin_t(const char* name_):
    library_handle(name_)
{
    using namespace std::string_literals;
    // Resolve functions. process_** and terminate are mandatory, throw if not found
    fcn_process_ww=(decltype(fcn_process_ww))library_handle.resolve("process_ww");
    fcn_process_ss=(decltype(fcn_process_ss))library_handle.resolve("process_ss");
    fcn_process_sw=(decltype(fcn_process_sw))library_handle.resolve("process_sw");
    fcn_process_ws=(decltype(fcn_process_ws))library_handle.resolve("process_ws");
    fcn_init=(decltype(fcn_init))library_handle.resolve("init");
    fcn_prepare=(decltype(fcn_prepare))library_handle.resolve("prepare");
    fcn_release=(decltype(fcn_release))library_handle.resolve("release");
    fcn_terminate=(decltype(fcn_terminate))library_handle.resolve_checked(std::string(name_)+"_terminate");
    // Initialize empty user config array to be filled by library's init
    user_config=c_argInit_Unboundedx1_user_conf(0);
    state=c_argInit_Unboundedx1_user_conf(0);
    if(fcn_init){
        fcn_init(user_config,state);
    }
}
matlab_wrapper::matlab_wrapper_t::wrapped_plugin_t::~wrapped_plugin_t(){
    emxDestroyArray_user_config_t(user_config);
    emxDestroyArray_user_config_t(state);
    if(fcn_terminate)
        (*fcn_terminate)();
};

mha_wave_t *matlab_wrapper::matlab_wrapper_t::wrapped_plugin_t::process_ww(mha_wave_t *s,
                                                                           emxArray_user_config_t *user_config_)
{
    // Reorder input from row-major to column major
    auto idx=[&s](int fr,int ch){return fr+s->num_frames*ch;};
    for(unsigned ch=0;ch<s->num_channels;++ch){
        for(unsigned fr=0;fr<s->num_frames;++fr){
            wave_in->data[idx(fr,ch)]=value(s,fr,ch);
        }
    }
    (*fcn_process_ww)(wave_in,&signal_dimensions,user_config_,state,wave_out);
    //Reorder back to mha convention
    for(unsigned fr=0;fr<mha_wave_out->num_frames;++fr){
        for(unsigned ch=0;ch<mha_wave_out->num_channels;++ch){
            value(*mha_wave_out,fr,ch)=wave_out->data[idx(fr,ch)];
        }
    }
    return mha_wave_out.get();
}

mha_spec_t *matlab_wrapper::matlab_wrapper_t::wrapped_plugin_t::process_ss(mha_spec_t *s,
                                                                           emxArray_user_config_t *user_config_)
{
    // Reorder input from row-major to column major
    auto idx=[&s](int fr,int ch){return fr+s->num_frames*ch;};
    for(unsigned ch=0;ch<s->num_channels;++ch){
        for(unsigned fr=0;fr<s->num_frames;++fr){
            spec_in->data[idx(fr,ch)].re=value(s,fr,ch).re;
            spec_in->data[idx(fr,ch)].im=value(s,fr,ch).im;
        }
    }
    (*fcn_process_ss)(spec_in,&signal_dimensions,user_config_,state,spec_out);
    //Reorder back to mha convention
    for(unsigned fr=0;fr<mha_spec_out->num_frames;++fr){
        for(unsigned ch=0;ch<mha_spec_out->num_channels;++ch){
            value(*mha_spec_out,fr,ch).re=spec_out->data[idx(fr,ch)].re;
            value(*mha_spec_out,fr,ch).im=spec_out->data[idx(fr,ch)].im;
        }
    }
    return mha_spec_out.get();
}


mha_spec_t *matlab_wrapper::matlab_wrapper_t::wrapped_plugin_t::process_ws(mha_wave_t *s,
                                                                           emxArray_user_config_t *user_config_)
{
    // Reorder input from row-major to column major
    auto idx=[&s](int fr,int ch){return fr+s->num_frames*ch;};
    for(unsigned ch=0;ch<s->num_channels;++ch){
        for(unsigned fr=0;fr<s->num_frames;++fr){
            wave_in->data[idx(fr,ch)]=value(s,fr,ch);
        }
    }
    (*fcn_process_ws)(wave_in,&signal_dimensions,user_config_,state,spec_out);
    //Reorder back to mha convention
    for(unsigned fr=0;fr<mha_spec_out->num_frames;++fr){
        for(unsigned ch=0;ch<mha_spec_out->num_channels;++ch){
            value(*mha_spec_out,fr,ch).re=spec_out->data[idx(fr,ch)].re;
            value(*mha_spec_out,fr,ch).im=spec_out->data[idx(fr,ch)].im;
        }
    }
    return mha_spec_out.get();
}

mha_wave_t *matlab_wrapper::matlab_wrapper_t::wrapped_plugin_t::process_sw(mha_spec_t *s,
                                                                           emxArray_user_config_t *user_config_)
{
    // Reorder input from row-major to column major
    auto idx=[&s](int fr,int ch){return fr+s->num_frames*ch;};
    for(unsigned ch=0;ch<s->num_channels;++ch){
        for(unsigned fr=0;fr<s->num_frames;++fr){
            spec_in->data[idx(fr,ch)].re=value(s,fr,ch).re;
            spec_in->data[idx(fr,ch)].im=value(s,fr,ch).im;
        }
    }
    (*fcn_process_sw)(spec_in,&signal_dimensions,user_config_,state,wave_out);
    //Reorder back to mha convention
    for(unsigned fr=0;fr<mha_wave_out->num_frames;++fr){
        for(unsigned ch=0;ch<mha_wave_out->num_channels;++ch){
            value(*mha_wave_out,fr,ch)=wave_out->data[idx(fr,ch)];
        }
    }
    return mha_wave_out.get();
}

void matlab_wrapper::matlab_wrapper_t::wrapped_plugin_t::prepare(mhaconfig_t& config){
    // Initialize input wave
    if(config.domain==MHA_WAVEFORM){
        wave_in=emxCreate_real_T(config.fragsize,config.channels);
        for(int i=0; i<wave_in->allocatedSize;++i){
            wave_in->data[i]=0.0;
        }
    }
    else if(config.domain==MHA_SPECTRUM){
        spec_in=emxCreate_creal_T(config.fftlen/2+1,config.channels);
        for(int i=0; i<spec_in->allocatedSize;++i){
            spec_in->data[i].re=spec_in->data[i].im=0.0;
        }
    }
    else
        throw MHA_Error(__FILE__,__LINE__,"Unknown domain %u",config.domain);
    // Read mha's signal dimensions into matlab struct
    signal_dimensions.channels=config.channels;
    signal_dimensions.domain=[config](){
                                 switch(config.domain){
                                 case MHA_WAVEFORM:
                                     return 'W';
                                 case MHA_SPECTRUM:
                                     return 'S';
                                 case MHA_DOMAIN_UNKNOWN:
                                     return 'U';
                                 default:
                                     throw MHA_Error(__FILE__,__LINE__,
                                                     "Bug: Unknown domain in input config: %u", config.domain);
                                 }
                             }();
    signal_dimensions.fragsize=config.fragsize;
    signal_dimensions.wndlen=config.wndlen;
    signal_dimensions.fftlen=config.fftlen;
    signal_dimensions.srate=config.srate;
    auto tmp=signal_dimensions;
    if(fcn_prepare)
        (*fcn_prepare)(&tmp,user_config,state);
    if(tmp.domain=='W' and config.domain==MHA_WAVEFORM and !fcn_process_ww)
        throw MHA_Error(__FILE__,__LINE__,"Processing callback not found!");
    if(tmp.domain=='W' and config.domain==MHA_SPECTRUM and !fcn_process_sw)
        throw MHA_Error(__FILE__,__LINE__,"Processing callback not found!");
    if(tmp.domain=='S' and config.domain==MHA_WAVEFORM and !fcn_process_ws)
        throw MHA_Error(__FILE__,__LINE__,"Processing callback not found!");
    if(tmp.domain=='S' and config.domain==MHA_SPECTRUM and !fcn_process_ss)
        throw MHA_Error(__FILE__,__LINE__,"Processing callback not found!");
    // And back again
    config.channels=tmp.channels;
    config.domain=[this,tmp](){
                         switch(tmp.domain){
                         case 'W':
                             return MHA_WAVEFORM;
                         case 'S':
                             return MHA_SPECTRUM;
                         case 'U':
                             return MHA_DOMAIN_UNKNOWN;
                         default:
                             throw MHA_Error(__FILE__,__LINE__,
                                             "Bug: Unknown domain in input config: %c", signal_dimensions.domain);
                         }
                     }();
    config.fragsize=tmp.fragsize;
    config.wndlen=tmp.wndlen;
    config.fftlen=tmp.fftlen;
    config.srate=tmp.srate;
    if(config.domain==MHA_WAVEFORM){
        wave_out=emxCreate_real_T(config.fragsize,config.channels);
        mha_wave_out=std::make_unique<MHASignal::waveform_t>(config.fragsize, config.channels);
        for(int i=0; i<wave_out->allocatedSize;++i){
            wave_out->data[i]=0.0;
        }
    }
    else if(config.domain==MHA_SPECTRUM){
        spec_out=emxCreate_creal_T(config.fftlen/2+1,config.channels);
        mha_spec_out=std::make_unique<MHASignal::spectrum_t>(config.fftlen/2+1, config.channels);
        for(int i=0; i<spec_out->allocatedSize;++i){
            spec_out->data[i].re=spec_out->data[i].im=0.0;
        }
    }
    else // Currently already caught earlier, but harden against changes
        throw MHA_Error(__FILE__,__LINE__,"Unknown domain %u",config.domain);
}

void matlab_wrapper::matlab_wrapper_t::wrapped_plugin_t::release(){
    if(fcn_release)
        (*fcn_release)();
    // Clean up input/output signal dimensions might change, we need to re-create anyway
    if(wave_in) emxDestroyArray_real_T(wave_in);
    if(wave_out) emxDestroyArray_real_T(wave_out);
    if(spec_in) emxDestroyArray_creal_T(spec_in);
    if(spec_out) emxDestroyArray_creal_T(spec_out);
}

matlab_wrapper::matlab_wrapper_t::matlab_wrapper_t(MHA_AC::algo_comm_t & iac,
                                                   const std::string &)
    : MHAPlugin::plugin_t<matlab_wrapper::matlab_wrapper_rt_cfg_t>("",iac)
{
    insert_item("library_name",&library_name);
    patchbay.connect(&library_name.writeaccess,this,&matlab_wrapper_t::load_lib);
}

void matlab_wrapper::matlab_wrapper_t::prepare(mhaconfig_t& signal_dimensions)
{
    if(plug)
        plug->prepare(signal_dimensions);
    else
        throw MHA_Error(__FILE__,__LINE__,"Library %s not loaded",library_name.data.c_str());
    for(auto & m : monitors){
        remove_item(&m);
    }
    insert_monitors();
    update();
    // Do not want to handle library reload during processing right now, so disable
    // With the curent code structure, a reload would probably probably not be done w/o
    // a re-initialiation of the user defined configuration variables
    library_name.setlock(true);
}

void matlab_wrapper::matlab_wrapper_t::release()
{
    plug->release();
    library_name.setlock(false);
}

void matlab_wrapper::matlab_wrapper_t::insert_monitors(){
    monitors.clear();
    auto state=plug->state;
    for(int i=0;i<state->size[0];++i){
        // We can not properly handle empty names
        if(strncmp(state->data[i].name->data,"",state->data[i].name->allocatedSize)==0)
            // We have no useful name, an index is all we can give the user
            throw MHA_Error(__FILE__,__LINE__,"state(%i) has no name!",i+1);
        else{
            // Construct string from char array to ensure termination
            auto name=std::string(state->data[i].name->data,state->data[i].name->size[1]);
            // Handle all variables as matrices for generality
            monitors.emplace_back(name);
            // c=Linearized index
            int c=0;
            monitors.back().data.resize(state->data[i].value->size[0]);
            for(int k=0;k<state->data[i].value->size[0];++k){
                monitors.back().data[k].resize(state->data[i].value->size[1]);
                for(int j=0;j<state->data[i].value->size[1];++j){
                    monitors.back().data[k][j]=state->data[i].value->data[c++];
                }
            }
            insert_item(name,&monitors.back());
        }
    }
}

void matlab_wrapper::matlab_wrapper_t::insert_config_vars() {
    auto user_config=plug->user_config;
    for(int i=0;i<user_config->size[0];++i){
        // We can not properly handle empty names
        if(strncmp(user_config->data[i].name->data,"",user_config->data[i].name->allocatedSize)==0)
            // We have no useful name, an index is all we can give the user
            throw MHA_Error(__FILE__,__LINE__,"user_config(%i) has no name!",i+1);
        else{
            // Handle all variables as matrices for generality
            vars.emplace_back(user_config->data[i].name->data,"[[0]]","");
            // c=Linearized index
            int c=0;
            for(int k=0;k<user_config->data[i].value->size[0];++k){
                for(int j=0;j<user_config->data[i].value->size[1];++j){
                    vars.back().data[k][j]=user_config->data[i].value->data[c++];
                }
            }
            insert_item(user_config->data[i].name->data,&vars.back());
            callbacks.emplace_back(this,&user_config->data[i],&vars.back());
            cb_patchbay.connect(&vars.back().writeaccess,&callbacks.back(),&callback::on_writeaccess);
        }
    }
}
void matlab_wrapper::matlab_wrapper_t::update_monitors(){
    auto state=plug->state;
    for(int i=0;i<state->size[0];++i){
        // c=Linearized index
        int c=0;
        for(int k=0;k<state->data[i].value->size[0];++k){
            for(int j=0;j<state->data[i].value->size[1];++j){
                if(static_cast<unsigned>(state->data[i].value->size[0])!=monitors[i].data.size() or
                   static_cast<unsigned>(state->data[i].value->size[1])!=monitors[i].data[k].size())
                    throw MHA_Error(__FILE__,__LINE__,"Monitor variable %s has changed size from %zu x %zu to %i x %i",
                                    monitors[i].fullname().c_str(),
                                    monitors[i].data.size(),
                                    monitors[i].data[k].size(),
                                    state->data[i].value->size[0],
                                    state->data[i].value->size[1]);
                monitors[i].data[k][j]=state->data[i].value->data[c++];
            }
        }
    }
}

void matlab_wrapper::matlab_wrapper_t::process(mha_wave_t* sin, mha_wave_t** sout)
{
    poll_config();
    // Call wrappers process, provide real time configs user_config as we are guaranteed
    // That it will not change under us
    *sout=plug->process_ww(sin,cfg->user_config);
    update_monitors();
}

void matlab_wrapper::matlab_wrapper_t::process(mha_wave_t* sin, mha_spec_t** sout)
{
    poll_config();
    // Call wrappers process, provide real time configs user_config as we are guaranteed
    // That it will not change under us
    *sout=plug->process_ws(sin,cfg->user_config);
    update_monitors();
}

void matlab_wrapper::matlab_wrapper_t::process(mha_spec_t* sin, mha_wave_t** sout)
{
    poll_config();
    // Call wrappers process, provide real time configs user_config as we are guaranteed
    // That it will not change under us
    *sout=plug->process_sw(sin,cfg->user_config);
    update_monitors();
}

void matlab_wrapper::matlab_wrapper_t::process(mha_spec_t* sin, mha_spec_t** sout)
{
    poll_config();
    // Call wrappers process, provide real time configs user_config as we are guaranteed
    // That it will not change under us
    *sout=plug->process_ss(sin,cfg->user_config);
    update_monitors();
}

void matlab_wrapper::matlab_wrapper_t::load_lib(){
    try{
        plug=std::make_unique<wrapped_plugin_t>(library_name.data.c_str());
    }
    catch(std::exception& e){
        throw MHA_Error(__FILE__,__LINE__,"Could not library %s: %s",library_name.data.c_str(),e.what());
    }
    catch(...){
        throw MHA_Error(__FILE__,__LINE__,"Could not library %s: Unknown error",library_name.data.c_str());
    }
    insert_config_vars();
    insert_monitors();
}

void matlab_wrapper::matlab_wrapper_t::update(){
    // rt config creates copy of user config that is protected against non rt safe writes
    // Wrappers's process callback only gets this protected copy which can only change atomically
    push_config(new matlab_wrapper::matlab_wrapper_rt_cfg_t(plug->user_config));
}

MHAPLUGIN_CALLBACKS(matlab_wrapper, matlab_wrapper::matlab_wrapper_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(matlab_wrapper, matlab_wrapper::matlab_wrapper_t,spec,spec)
MHAPLUGIN_PROC_CALLBACK(matlab_wrapper, matlab_wrapper::matlab_wrapper_t,wave,spec)
MHAPLUGIN_PROC_CALLBACK(matlab_wrapper,matlab_wrapper::matlab_wrapper_t,spec,wave)

MHAPLUGIN_DOCUMENTATION\
(matlab_wrapper,
 "test-tool",
 "")

// Local variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
