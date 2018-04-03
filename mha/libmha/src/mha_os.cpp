// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2008 2012 2013 2014 2015 2016 2017 HörTech gGmbH
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

#include "mha_os.h"
#include "mha_error.hh"
#include <iostream>

#ifndef _WIN32
#include <dirent.h>
#include <fnmatch.h>
#endif


std::string mha_getenv(std::string envvar)
{
    std::string lp("");
#ifdef _WIN32
    char lpsz[16*MAX_PATH];
    ZeroMemory(lpsz, 16*MAX_PATH);
    if (GetEnvironmentVariable(envvar.c_str(), lpsz, 16*MAX_PATH))
        lp = lpsz;
#else
    char * s = getenv(envvar.c_str());
    if(s)
        lp = s;
#endif
    return lp;
}


std::list<std::string> mha_library_paths()
{
    std::list<std::string> paths;
#ifndef MHA_STATIC_PLUGINS
    std::string lp;
    lp = mha_getenv("MHA_LIBRARY_PATH");
    if( !lp.size() )
        lp += ";";
    else if( lp[lp.size()-1] != ';' )
        lp += ";";
    std::string ptok;
    while( lp.find(";") < lp.size() ){
        ptok = lp.substr(0, lp.find(";") );
#ifdef _WIN32
        if( ptok.size() && (ptok[ptok.size()-1] != '\\') )
            ptok += "\\";
#else
        if( ptok.size() && (ptok[ptok.size()-1] != '/') )
            ptok += "/";
#endif
        paths.push_back( ptok );
        lp.erase( 0, lp.find(";")+1 );
    }
#endif
    return paths;
}


dynamiclib_t::dynamiclib_t(const std::string& n)
    : modulename(n)
#ifndef _WIN32
    ,h(NULL)
#endif
{
#ifdef MHA_STATIC_PLUGINS // plugin code is compiled into executable
  fullname = modulename;
#ifdef _WIN32
  h = GetModuleHandle(NULL);
#else
  h = dlopen(NULL, RTLD_LAZY);
#endif
  // check if code for this plugin is present by checking marker symbol
  if (!resolve("dummy_interface_test"))
    throw MHA_Error(__FILE__,__LINE__,
                    "Plugin \"%s\" is not present in this MHA executable",
                    n.c_str());
#else
    std::list<std::string> paths(mha_library_paths());
    std::string lp("");
    std::string loadliberr("");
    std::string libpath("");
    for( std::list<std::string>::iterator it = paths.begin(); it != paths.end(); ++it ){
        libpath += *it + ";";
        fullname = *it + n + mha_lib_extension;
#ifdef _WIN32
        h = LoadLibrary( fullname.c_str() );
#else
        h = dlopen( fullname.c_str(), RTLD_NOW );
        if( !h ){
            loadliberr += "\n";
            loadliberr += dlerror();
        }
#endif
        if( h )
            break;
    }
    if( !h )
        throw MHA_Error(__FILE__,__LINE__,
                        "Unable to load library \"%s\" (MHA_LIBRARY_PATH=%s).%s",
                        n.c_str(), libpath.c_str(),loadliberr.c_str());
#endif
}

void* dynamiclib_t::resolve(const std::string& n)
{
    std::string resolve_name =
#ifdef MHA_STATIC_PLUGINS
        "MHA_STATIC_" + getmodulename() + "_" +
#else
        "MHA_DYNAMIC_" + 
#endif
    n;
#ifdef _WIN32
    void* ret;
    ret = (void*)GetProcAddress(h,resolve_name.c_str());
    if( ret )
        return ret;
    resolve_name = "_" + resolve_name;
    return (void*)GetProcAddress(h,resolve_name.c_str());
#else
    return dlsym(h,resolve_name.c_str());
#endif
}

void* dynamiclib_t::resolve_checked(const std::string& n)
{
    void* ret = resolve( n );
    if( !ret )
        throw MHA_Error(__FILE__,__LINE__,
                        "Unable to resolve function \"%s\" in library %s.",
                        n.c_str(), fullname.c_str() );
    return ret;
}

dynamiclib_t::~dynamiclib_t()
{
#ifndef MHA_STATIC_PLUGINS
#ifdef _WIN32
    FreeLibrary( h );
#else
    dlclose( h );
#endif
#endif
}


std::list<std::string> list_dir(const std::string& path,const std::string& pattern)
{
    std::list<std::string> search_result;
#ifdef _WIN32
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;
    hFind = FindFirstFile((path + pattern).c_str(), &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
        return search_result;
    do {
        search_result.push_back(FindFileData.cFileName);
    } while (FindNextFile(hFind, &FindFileData) != 0);
    FindClose(hFind);
#else
    struct dirent **namelist;
    int n;
    std::string dir(path);
    if( !dir.size() )
        dir = ".";
    n = scandir(dir.c_str(), &namelist, NULL, alphasort );
    if( n >= 0 ){
        for(int k=0;k<n;k++){
            if( fnmatch(pattern.c_str(),namelist[k]->d_name,0)==0 )
                search_result.push_back(namelist[k]->d_name);
            free(namelist[k]);
        }
        free(namelist);
    }
#endif
    return search_result;
}

// Local Variables:
// compile-command: "make -C .."
// coding: utf-8-unix
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
