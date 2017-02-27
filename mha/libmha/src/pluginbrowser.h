// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2014 2016 HörTech gGmbH
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

#ifndef PLUGINBROWSER
#define PLUGINBROWSER

#include <string>
#include <vector>
#include <map>
#include "mha_algo_comm.hh"
#include "mhapluginloader.h"

class plugindescription_t {
public:
    std::string name;
    std::string fullname;
    std::string documentation;
    std::vector<std::string> categories;
    bool wave2wave;
    bool wave2spec;
    bool spec2wave;
    bool spec2spec;
    std::vector<std::string> query_cmds;
    std::map<std::string,std::string> queries;
};

class pluginloader_t : private MHAKernel::algo_comm_class_t, public PluginLoader::mhapluginloader_t
{
public:
    pluginloader_t(const std::string& name);
    ~pluginloader_t() throw ();
};    

class pluginbrowser_t {
public:
    pluginbrowser_t();
    void get_paths();
    plugindescription_t scan_plugin(const std::string& name);
    void add_plugins();
    void clear_plugins();
    void scan_plugins();
    void add_plugin(const std::string& name);
    std::list<plugindescription_t> get_plugins() const { return plugins;};
private:
    std::string plugin_extension;
    std::list<std::string> library_paths;
    std::list<plugindescription_t> plugins;
    std::map<std::string,pluginloader_t*> p;
};


#endif

/*
 * Local Variables:
 * compile-command: "make -C .."
 * c-basic-offset: 4
 * mode: c++
 * coding: utf-8-unix
 * indent-tabs-mode: nil
 * End:
 */
