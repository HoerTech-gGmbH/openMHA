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

#include "mha_os.h"
#include "pluginbrowser.h"
#include "mha_algo_comm.hh"
#include <exception>

#ifndef _WIN32
#include <dirent.h>
#include <fnmatch.h>
#endif

#define DEBUG(x) std::cerr << __FILE__ << ":" << __LINE__ << " " << #x<<"="<<x << std::endl

int main(int argc,char** argv)
{
    std::string pattern("");
    if( argc > 1 )
        pattern = argv[1];
    pluginbrowser_t pb;
    pb.get_paths();
    pb.add_plugins();
    pb.scan_plugins();
    std::list<plugindescription_t> plugins(pb.get_plugins());
    for(std::list<plugindescription_t>::iterator it=plugins.begin();it!=plugins.end();++it){
        if( (pattern.size() == 0) || (it->name.find(pattern) != std::string::npos) ){
            std::cout << it->name << ": ";
            if( it->wave2wave )
                std::cout << "wave2wave ";
            if( it->wave2spec )
                std::cout << "wave2spec ";
            if( it->spec2wave )
                std::cout << "spec2wave ";
            if( it->spec2spec )
                std::cout << "spec2spec ";
            std::cout << "[";
            for( std::vector<std::string>::iterator cat=it->categories.begin();cat!=it->categories.end();++cat)
                std::cout << " " << *cat;
            std::cout << " ]" << std::endl;
        }
    }
    pb.clear_plugins();
}
 
/*
 * Local Variables:
 * compile-command: "make -C .."
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * coding: utf-8-unix
 * End:
 */
