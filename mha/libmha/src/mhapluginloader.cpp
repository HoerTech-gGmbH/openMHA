// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2007 2008 2009 2011 2012 2013 2014 2016 2017 2018 HörTech gGmbH
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

#include <cstring>
#include "mhapluginloader.h"
#ifdef MHA_DEBUG
#include <cstdio>
#endif


PluginLoader::config_file_splitter_t::config_file_splitter_t(const std::string& name)
    : origname(name)
{
    MHAParser::expression_t cfgfile_exp(name,"<");
    configfile = cfgfile_exp.rval;
    MHAParser::expression_t cfgname(cfgfile_exp.lval, ":");
    if(!cfgname.rval.size())
        cfgname.rval = cfgname.lval;
    if (cfgname.lval == "")
        throw MHA_ErrorMsg("Empty plugin configuration name.");
    libname = cfgname.lval;
    configname = cfgname.rval;
}

PluginLoader::mhapluginloader_t::mhapluginloader_t(algo_comm_t iac,const std::string& libname,bool check_version)
    : config_file_splitter_t(libname),
      MHAParser::c_ifc_parser_t(get_libname()),
      ac(iac),
      lib_handle(get_libname()),
      lib_data(NULL),
      MHAGetVersion_cb(NULL),
      MHAInit_cb(NULL),
      MHADestroy_cb(NULL),
      MHAPrepare_cb(NULL),
      MHARelease_cb(NULL),
      MHAProc_wave2wave_cb(NULL),
      MHAProc_spec2spec_cb(NULL),
      MHAProc_wave2spec_cb(NULL),
      MHAProc_spec2wave_cb(NULL),
      MHASet_cb(NULL),
      MHASetcpp_cb(NULL),
      MHAStrError_cb(NULL),
      plugin_documentation(""),
      b_check_version(check_version),
      b_is_prepared(false)
{
    resolve_and_init();
}

void PluginLoader::mhapluginloader_t::resolve_and_init()
{
    MHAPluginDocumentation_t MHAPluginDocumentation_cb;
    MHAPluginCategory_t MHAPluginCategory_cb;
    MHA_RESOLVE_CHECKED((&lib_handle),MHAGetVersion);
    MHA_RESOLVE((&lib_handle),MHAInit);
    MHA_RESOLVE((&lib_handle),MHADestroy);
    MHA_RESOLVE((&lib_handle),MHAProc_wave2wave);
    MHA_RESOLVE((&lib_handle),MHAProc_spec2spec);
    MHA_RESOLVE((&lib_handle),MHAProc_wave2spec);
    MHA_RESOLVE((&lib_handle),MHAProc_spec2wave);
    MHA_RESOLVE((&lib_handle),MHASet);
    MHA_RESOLVE((&lib_handle),MHASetcpp);
    MHA_RESOLVE((&lib_handle),MHAPrepare);
    MHA_RESOLVE((&lib_handle),MHARelease);
    MHA_RESOLVE((&lib_handle),MHAStrError);
    MHA_RESOLVE((&lib_handle),MHAPluginDocumentation);
    MHA_RESOLVE((&lib_handle),MHAPluginCategory);
    if( b_check_version )
        test_version();

    if( MHAPluginDocumentation_cb )
        plugin_documentation = MHAPluginDocumentation_cb();
    if( MHAPluginCategory_cb ){
        std::string tmp_categories = MHAPluginCategory_cb();
        tmp_categories = "["+tmp_categories+"]";
        MHAParser::StrCnv::str2val(tmp_categories,plugin_categories);
    }

    if( !plugin_categories.size() )
        plugin_categories.push_back("other");
    if( MHAInit_cb ){
        lib_err = MHAInit_cb(ac,"(no chain)",get_configname().c_str(),&lib_data);
        test_error();
    }
    set_parse_cb(MHASet_cb,MHAStrError_cb,lib_data);

    std::string cfgfile = get_configfile();
    if( cfgfile.size() )
        parse("?read:"+cfgfile);
}

void PluginLoader::mhapluginloader_t::mha_test_struct_size(unsigned int s) {
    s = s & 0xff;
    if(!(s & 0x01))
        throw MHA_Error(__FILE__, __LINE__, "Invalid size of 'mha_real_t'. Please check your compiler settings.");
    if(!(s & 0x02))
        throw MHA_Error(__FILE__, __LINE__, "Invalid size of 'mha_complex_t'. Please check your compiler settings.");
    if(!(s & 0x04))
        throw MHA_Error(__FILE__, __LINE__, "Invalid size of 'mha_wave_t'. Please check your compiler settings.");
    if(!(s & 0x08))
        throw MHA_Error(__FILE__, __LINE__, "Invalid size of 'mha_spec_t'. Please check your compiler settings.");
    if(!(s & 0x10))
        throw MHA_Error(__FILE__, __LINE__, "Invalid size of 'mhaconfig_t'. Please check your compiler settings.");
}

void PluginLoader::mhapluginloader_t::test_version()
{
    unsigned int v = MHAGetVersion_cb();
    std::string name = lib_handle.getname();
    unsigned int v_major = (v & 0xff000000) >> 24;
    unsigned int v_minor = (v & 0x00ff0000) >> 16;
    unsigned int v_release = (v & 0x0000ff00) >> 8;
    unsigned int v_structs = (v & 0x000000ff);
    if((v_major != MHA_VERSION_MAJOR) || (v_minor != MHA_VERSION_MINOR))
        throw MHA_Error(__FILE__, __LINE__,
                        "version conflict: MHA is %d.%drc%d, DLL is %u.%urc%u (%s)",
                        MHA_VERSION_MAJOR, MHA_VERSION_MINOR, MHA_VERSION_RELEASE,
                        v_major, v_minor, v_release, name.c_str());
    mha_test_struct_size(v_structs);
}

PluginLoader::mhapluginloader_t::~mhapluginloader_t() throw()
{
    if( MHADestroy_cb )
        MHADestroy_cb(lib_data);
}

bool PluginLoader::mhapluginloader_t::has_process(mha_domain_t dom_in,mha_domain_t dom_out) const
{
    if( (dom_in == MHA_WAVEFORM) && (dom_out == MHA_WAVEFORM) )
        return MHAProc_wave2wave_cb;
    if( (dom_in == MHA_SPECTRUM) && (dom_out == MHA_SPECTRUM) )
        return MHAProc_spec2spec_cb;
    if( (dom_in == MHA_WAVEFORM) && (dom_out == MHA_SPECTRUM) )
        return MHAProc_wave2spec_cb;
    if( (dom_in == MHA_SPECTRUM) && (dom_out == MHA_WAVEFORM) )
        return MHAProc_spec2wave_cb;
    return false;
}

bool PluginLoader::mhapluginloader_t::has_parser() const
{
    return MHASet_cb;
}

void PluginLoader::mhapluginloader_t::prepare(mhaconfig_t& tf)
{
    bool prepare_called(false);
    cf_input = tf;
    if( MHAPrepare_cb ){
        lib_err = MHAPrepare_cb(lib_data,&tf);
        test_error();
        prepare_called = true;
    }
    cf_output = tf;
    if( !has_process( cf_input.domain, cf_output.domain ) ){
        std::string name = lib_handle.getname();
        std::string release_err("");
        if( prepare_called && MHARelease_cb ){
            try{
                lib_err = MHARelease_cb(lib_data);
                test_error();
            }
            catch( std::exception& e ){
                release_err = "\n(Release failed with message: \n\"";
                release_err += e.what();
                release_err += "\"";
            }
        }
        throw MHA_Error(__FILE__,__LINE__,
                        "The plugin %s has no processing callback for %s to %s processing.%s",
                        name.c_str(), mhastrdomain(cf_input.domain), mhastrdomain(cf_output.domain), release_err.c_str() );
    }
    b_is_prepared = true;
}

void PluginLoader::mhapluginloader_t::release()
{
    if( MHARelease_cb ){
        lib_err = MHARelease_cb(lib_data);
        test_error();
    }
    b_is_prepared = false;
}

void PluginLoader::mhapluginloader_t::process(mha_wave_t* s_in,mha_wave_t** s_out)
{
    if( !MHAProc_wave2wave_cb )
        throw MHA_ErrorMsg("Processing callback undefined.");
    lib_err = MHAProc_wave2wave_cb(lib_data, s_in, s_out );
    test_error();
}

void PluginLoader::mhapluginloader_t::process(mha_spec_t* s_in,mha_spec_t** s_out)
{
    if( !MHAProc_spec2spec_cb )
        throw MHA_ErrorMsg("Processing callback undefined.");
    lib_err = MHAProc_spec2spec_cb(lib_data, s_in, s_out );
    test_error();
}

void PluginLoader::mhapluginloader_t::process(mha_wave_t* s_in,mha_spec_t** s_out)
{
    if( !MHAProc_wave2spec_cb )
        throw MHA_ErrorMsg("Processing callback undefined.");
    lib_err = MHAProc_wave2spec_cb(lib_data, s_in, s_out );
    test_error();
}

void PluginLoader::mhapluginloader_t::process(mha_spec_t* s_in,mha_wave_t** s_out)
{
    if( !MHAProc_spec2wave_cb )        
        throw MHA_ErrorMsg("Processing callback undefined.");
    lib_err = MHAProc_spec2wave_cb(lib_data, s_in, s_out );
    test_error();
}

void PluginLoader::mhapluginloader_t::test_error()
{
    std::string modulename = lib_handle.getmodulename();
    if( lib_err != 0 ){
        if( MHAStrError_cb){

            throw MHA_Error(__FILE__,__LINE__,
                            "Error in module \"%s:%s\":\n%s", 
                            modulename.c_str(),
                            get_configname().c_str(),
                            MHAStrError_cb(lib_data,lib_err));
        }else{
            throw MHA_Error(__FILE__,__LINE__,
                            "Error in module \"%s:%s\" (error no %d).", 
                            modulename.c_str(),
                            get_configname().c_str(),lib_err);
        }
    }
}

mha_domain_t PluginLoader::mhapluginloader_t::input_domain() const
{
    return cf_input.domain;
}

mha_domain_t PluginLoader::mhapluginloader_t::output_domain() const
{
    return cf_output.domain;
}

std::string PluginLoader::mhapluginloader_t::parse(const std::string & str)
{
    if (MHASetcpp_cb)
        return MHASetcpp_cb(lib_data, str);
    return MHAParser::c_ifc_parser_t::parse(str);
}

const char* PluginLoader::mhastrdomain(mha_domain_t d)
{
    switch(d){
    case MHA_WAVEFORM :
        return "waveform";
    case MHA_SPECTRUM :
        return "spectrum";
    }
    return "unknown domain";
}

/**

   \brief Compare two mhaconfig_t structures, and report differences as an error

   \param req Expected mhaconfig_t structure
   \param avail Available mhaconfig_t structure
   \param pref Prefix for error messages

*/
void PluginLoader::mhaconfig_compare(const mhaconfig_t& req, const mhaconfig_t& avail,const std::string& pref)
{
    if(req.channels != avail.channels)
        throw MHA_Error(__FILE__, __LINE__,"%s: %u channels required, %u available.",pref.c_str(),
                        req.channels, avail.channels);
    if(req.domain != avail.domain)
        throw MHA_Error(__FILE__, __LINE__,"%s: domain %s required, %s available.",pref.c_str(),
                        PluginLoader::mhastrdomain(req.domain), PluginLoader::mhastrdomain(avail.domain));
    if(req.fragsize != avail.fragsize)
        throw MHA_Error(__FILE__, __LINE__,"%s: a fragsize of %u samples required, %u available.",
                        pref.c_str(), req.fragsize, avail.fragsize);
    if(req.srate != avail.srate)
        throw MHA_Error(__FILE__, __LINE__,"%s: a sample rate of %g Hz required, %g Hz available.",
                        pref.c_str(), req.srate, avail.srate);
    if( req.domain == MHA_SPECTRUM ){
        if(req.fftlen != avail.fftlen)
            throw MHA_Error(__FILE__, __LINE__,"%s: a FFT length of %u samples required, %u available.",
                            pref.c_str(), req.fftlen, avail.fftlen);
        if(req.wndlen != avail.wndlen)
            throw MHA_Error(__FILE__, __LINE__,"%s: a window length of %u samples required, %u available.",
                            pref.c_str(), req.wndlen, avail.wndlen);
    }
}

MHAParser::mhapluginloader_t::mhapluginloader_t(MHAParser::parser_t& parent,const algo_comm_t& ac,
                                                const std::string& plugname_name, const std::string& prefix)
    : plug(NULL),
      parent_(parent),
      plugname("Plugin name",""),
      prefix_(prefix),
      connector(&plugname.writeaccess,this,&MHAParser::mhapluginloader_t::load_plug),
      ac_(ac),
      plugname_name_(plugname_name)
{
    parent_.insert_item(plugname_name,&plugname);
    memset(&cf_in_,0,sizeof(mhaconfig_t));
    memset(&cf_out_,0,sizeof(mhaconfig_t));
}

MHAParser::mhapluginloader_t::~mhapluginloader_t()
{
    if( plug )
        delete plug;
}

void MHAParser::mhapluginloader_t::load_plug()
{
    if( plug )
        delete plug;
    plug = NULL;
    parent_.force_remove_item(last_name);
    if( plugname.data.size() ){
        plug = new PluginLoader::mhapluginloader_t(ac_,plugname.data);
        if( plug->has_parser() ){
            if( prefix_.size() )
                last_name = prefix_;
            else
                last_name = plug->get_configname();
            parent_.insert_item(last_name,plug);
        }
    }
}

void MHAParser::mhapluginloader_t::prepare(mhaconfig_t& cf)
{
    if( plug ){
        cf_in_ = cf;
        plug->prepare(cf);
        cf_out_ = cf;
    }else 
        throw MHA_Error(__FILE__,__LINE__,"No plugin loaded (variable \"%s\" is empty).",plugname_name_.c_str());
    plugname.setlock(true);
    // \todo allow loading of plugins after prepare.
}

void MHAParser::mhapluginloader_t::release()
{ 
    memset(&cf_in_,0,sizeof(mhaconfig_t));
    memset(&cf_out_,0,sizeof(mhaconfig_t));
    plugname.setlock(false);
    if( !plug )
        throw MHA_Error(__FILE__,__LINE__,"Programming error: plug is undefined (mhapluginloader_t).");
    plug->release();
}

// Local Variables:
// compile-command: "make -C .."
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
