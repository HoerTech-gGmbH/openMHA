// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2014 2016 2017 2018 HörTech gGmbH
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

#include "pluginbrowser.h"

pluginloader_t::~pluginloader_t() throw ()
{
}

pluginloader_t::pluginloader_t(const std::string& name)
    : PluginLoader::mhapluginloader_t(get_c_handle(),name)
{
}

pluginbrowser_t::pluginbrowser_t()
{
}

void pluginbrowser_t::get_paths()
{
    library_paths = mha_library_paths();
    library_paths.push_back("/lib/");
    library_paths.push_back("/usr/lib/");
}

void pluginbrowser_t::clear_plugins()
{
    for(std::map<std::string,pluginloader_t*>::iterator i=p.begin();i!=p.end();++i)
        delete i->second;
    p.clear();
}

void pluginbrowser_t::add_plugin(const std::string& name)
{
    if( p.find(name) != p.end() )
        return;
    p[name] = new pluginloader_t(name);
}

plugindescription_t pluginbrowser_t::scan_plugin(const std::string& name)
{
    plugindescription_t desc;
    // do whatever is needed:
    pluginloader_t* dl(p[name]);
    desc.name = name;
    desc.fullname = dl->getfullname();
    desc.documentation = dl->get_documentation();
    desc.categories = dl->get_categories();
    desc.wave2wave = dl->has_process(MHA_WAVEFORM,MHA_WAVEFORM);
    desc.wave2spec = dl->has_process(MHA_WAVEFORM,MHA_SPECTRUM);
    desc.spec2wave = dl->has_process(MHA_SPECTRUM,MHA_WAVEFORM);
    desc.spec2spec = dl->has_process(MHA_SPECTRUM,MHA_SPECTRUM);
    if( dl->has_parser() ){
        MHAParser::StrCnv::str2val(dl->parse("?cmds"),desc.query_cmds);
        for(std::vector<std::string>::iterator q=desc.query_cmds.begin();q!=desc.query_cmds.end();++q){
            try{
                desc.queries[*q] = dl->parse(*q);
            }
            catch( const std::exception& e){
            };
        }
    }
    return desc;
}

void pluginbrowser_t::scan_plugins()
{
    for(std::map<std::string,pluginloader_t*>::iterator i=p.begin();i!=p.end();++i)
        plugins.push_back(scan_plugin(i->first));
}

void pluginbrowser_t::add_plugins()
{
    for(std::list<std::string>::iterator it=library_paths.begin();it!=library_paths.end();++it){
        std::list<std::string> pplugins(list_dir(*it,"*.so"));
        for(std::list<std::string>::iterator plug=pplugins.begin();plug!=pplugins.end();++plug){
            size_t extp(plug->rfind(".so"));
            if( extp != std::string::npos ){
                plug->erase(extp);
                try{
                    add_plugin(*plug);
                }
                catch( const std::exception& e){
                }
            }
        }
    }
}

// Local Variables:
// coding: utf-8-unix
// End:
