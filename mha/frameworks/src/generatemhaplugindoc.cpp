// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2007 2008 2013 2016 HörTech gGmbH
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

#include "mhapluginloader.h"
#include "mha_algo_comm.hh"
#include <exception>
#include <getopt.h>
#include <stdarg.h>
#include <fstream>

std::string conv2latex(std::string s,bool iscolored=false)
{
    MHAParser::strreplace(s,"\\","$$\\backslash$");
    MHAParser::strreplace(s,"{","\\{");
    MHAParser::strreplace(s,"}","\\}");
    MHAParser::strreplace(s,"(c)","\\copyright{}");
    MHAParser::strreplace(s,"_","\\_");
    MHAParser::strreplace(s,"&","\\&");
    MHAParser::strreplace(s,"#","\\#");
    MHAParser::strreplace(s,"$","\\$");
    MHAParser::strreplace(s,"\\$\\$","$");
    MHAParser::strreplace(s,"<","$<$");
    MHAParser::strreplace(s,">","$>$");
    MHAParser::strreplace(s,"%","\\%");
    MHAParser::strreplace(s,"~","\\verb!~!");
    MHAParser::strreplace(s,"^","\\verb!^!");
    if( iscolored )
        s = "\\color{monitorcolor}{"+s+"}";
    return s;
}

class latex_doc_t {
public:
    latex_doc_t(const std::string& plugname,const std::string& plugin_macro);
    std::string get_latex_doc();
    std::string get_main_category();
    std::vector<std::string> get_categories();
private:
    std::string strdom(mha_domain_t d);
    std::string get_ac(MHAKernel::algo_comm_class_t& ac,std::string txt);
    std::string parsername(std::string s);
    std::string get_parser_var(MHAParser::base_t* p,std::string name);
    std::string get_parser_tab(MHAParser::base_t* p,std::string prefix);
    std::string plugname;
    std::string latex_plugname;
    MHAKernel::algo_comm_class_t ac;
    PluginLoader::mhapluginloader_t loader;
    std::string plugin_macro;
};

latex_doc_t::latex_doc_t(const std::string& plugname_,const std::string& plugin_macro_)
    : plugname(plugname_),
      latex_plugname(conv2latex(plugname)),
      loader(ac.get_c_handle(),plugname),
      plugin_macro(plugin_macro_)
{
}

std::string latex_doc_t::get_latex_doc()
{
    std::string plugin_subsection_macro = "\\paragraph";
    if (plugin_macro == "\\subsection")
        plugin_subsection_macro = "\\subsubsection";
    if (plugin_macro == "\\section")
        plugin_subsection_macro = "\\subsection";
    
    std::string retv("");
    retv += "\\" + plugin_macro + "{" + latex_plugname + "}\n\\label{plug:" + plugname + "}\n\n";
    retv += "\\index{" + latex_plugname + " (MHA plugin)}\n";
    retv += "\\index{plugin!" + latex_plugname + "}\n";
    std::vector<std::string> cats = loader.get_categories();
    for( unsigned int k=0;k<cats.size();k++){
        retv += "\\index{" + conv2latex(cats[k]) + " (plugin category)!" + latex_plugname + "}\n";
    }
    if( loader.has_parser() )
        retv += conv2latex(loader.parse("?help")) + "\n\n";
    std::string detailed_doc(loader.get_documentation());
    if( detailed_doc.size() )
        retv += "\\paragraph{Detailed description}\n\n" + loader.get_documentation() + "\n\n";
    retv += "\\paragraph{Supported domains}\n\nThe MHA plugin {\\tt " 
        + latex_plugname + "} supportes these signal domains:\n\\begin{itemize}\n";
    mha_domain_t indom, outdom;
    unsigned int n_domains(0);
    for( indom = 0; indom < MHA_DOMAIN_MAX; indom++ ) 
        for( outdom = 0; outdom < MHA_DOMAIN_MAX; outdom++ ) 
            if( loader.has_process(indom,outdom) ){
                retv += " \\item " + strdom(indom) + " to " + strdom(outdom) + "\n";
                n_domains++;
            }
    if( n_domains == 0 )
        retv += " \\item This plugin has no processing callbacks.\n";
    retv += "\\end{itemize}\n\n";
    if( cats.size() > 0 ){
        retv += "\n\n\\paragraph{Categories}\n\n \\emph{" + cats[0] + "} ";
        for( unsigned int k=1;k<cats.size();k++){
            retv += conv2latex(cats[k]) + " ";
        }
        retv += "\n\n";
    }
    if( loader.has_parser() ){
        retv += get_parser_tab(&loader,"");
    }
    return retv;
}

std::string latex_doc_t::get_main_category()
{
    return loader.get_categories()[0];
}

std::vector<std::string> latex_doc_t::get_categories()
{
    return loader.get_categories();
}

std::string latex_doc_t::strdom( mha_domain_t d )
{
    switch( d ){
    case MHA_WAVEFORM : return "waveform";
    case MHA_SPECTRUM : return "spectrum";
    default: return "unknwon";
    }
}

std::string latex_doc_t::get_ac(MHAKernel::algo_comm_class_t& ac,std::string txt)
{
    std::string retv("");
    std::string stmp;
    std::vector<std::string> vstmp;
    stmp = ac.local_get_entries();
    if( stmp.size() ){
        retv += "AC variables " + txt + ":\n";
        MHAParser::StrCnv::str2val(std::string("[")+stmp+std::string("]"),vstmp);
        for( unsigned int k=0;k<vstmp.size();k++)
            retv += "  " + vstmp[k] + "\n";
        retv += "\n";
    }else{
        retv += "empty AC space " + txt + ".\n";
    }
    return retv;
}

std::string latex_doc_t::parsername(std::string s)
{
    if( s.rfind(".") < s.size() )
        s.erase(0,s.rfind(".")+1);
    return s;
}

std::string latex_doc_t::get_parser_var(MHAParser::base_t* p,std::string name)
{
    std::string retv("");
    bool is_parser(false);
    bool is_monitor(false);
    if( p->parse(name+"?type") == "parser" )
        is_parser = true;
    if( !is_parser )
        is_monitor = p->parse(name+"?perm")=="monitor";
    retv += conv2latex(parsername(name),is_monitor) + " & ";
    retv += conv2latex(p->parse(name+"?type"),is_monitor) + " & ";
    retv += conv2latex(p->parse(name+"?help"));
    std::string cmds = p->parse(name+"?cmds");
    if( cmds.find("range") < cmds.size() ){
        std::string range = p->parse(name+"?range");
        if( range.size() )
            retv += "\n\n{\\bf Range:} " + conv2latex(range);
    }
    retv += " & ";
    if( is_parser )
        retv += "(see below)";
    else{
        if( is_monitor )
            retv += conv2latex("(monitor)",is_monitor);
        else
            retv += conv2latex(p->parse(name+"?val"));
    }
    retv += "\\\\\n";
    return retv;
}

std::string latex_doc_t::get_parser_tab(MHAParser::base_t* p,std::string prefix)
{
    std::string retv;
    if( p->parse(prefix+"?type") == "parser" ){
        std::vector<std::string> entries;
        MHAParser::StrCnv::str2val(p->parse(prefix+"?entries"),entries);
        if( entries.size() ){
            unsigned int k;
            if( prefix.size() )
                retv += "\n\nVariables of sub-parser {\\tt " + conv2latex(parsername(prefix)) + "}:\n\n";
            else
                retv += "\n\\paragraph{Configuration variables}\n\n";
            retv += "{\\small\\begin{tabular}{|l|l|p{6cm}|l|}\n\\hline\n";
            retv += "{\\bf\\normalsize Name}&{\\bf\\normalsize Type}&{\\bf\\normalsize Description}&{\\bf\\normalsize Default}\\\\\\hline\n";
            for(k=0;k<entries.size();k++){
                retv += get_parser_var(p,prefix+"."+entries[k]);
                retv += "\\hline\n";
            }
            retv += "\\end{tabular}}\n";
            for(k=0;k<entries.size();k++){
                if( p->parse(prefix+"."+entries[k]+"?type")=="parser")
                    retv += get_parser_tab(p,prefix+"."+entries[k]);
            }
        }
    }else{
        if( prefix.size() == 0 ){
            retv += "\\paragraph{Configuration}\n\nThe plugin represents a variable node in the MHA configuration hierarchy.\n\n";
            retv += "{\\small\\begin{tabular}{|ll|p{5cm}|l|}\n\\hline\n";
            retv += "&{\\bf\\normalsize Type}&{\\bf\\normalsize Description}&{\\bf\\normalsize Default}\\\\\\hline\n";
            retv += get_parser_var(p,prefix);
            retv += "\\hline\n\\end{tabular}}\n";
        }
    }
    return retv;
}

void create_latex_doc(std::map<std::string,std::string>& doc,const std::string& plugname,const std::string& plugin_macro)
{
    latex_doc_t ldoc(plugname,plugin_macro);
    doc[ldoc.get_main_category()] += ldoc.get_latex_doc();
}

int main(int argc,char** argv)
{
    std::string ofname("generatemhaplugindoc.tex");
    std::string category_macro("section");
    std::string plugin_macro("subsection");
    int option;
    static struct option long_options[] = {
        {"ofname",         1, NULL, 'o'},
        {"category-macro", 1, NULL, 'c'},
        {"plugin-macro",   1, NULL, 'p'},
        {"help",           0, NULL, 'h'},
        {NULL,             0, NULL, 0  }
    };
    static char short_options[] = "o:c:p:h";
    while( (option = getopt_long(argc,argv,short_options,long_options,NULL)) > -1 ){
        switch( option ){
        case 'o' :
            ofname = optarg;
            break;
        case 'c' :
            category_macro = optarg;
            break;
        case 'p' :
            plugin_macro = optarg;
            break;
        case 'h' :
            std::cout << 
                "Options:\n"
                "  --ofname=name | -o name : set output file name\n"
                "  --category-macro=macro | -c macro : set LaTeX macro for categories\n"
                "  --plugin-macro=macro | -p macro : set LaTeX macro for plugins\n"
                "  --help | -h : print this help screen\n";
            exit(1);
            // break not needed, would be unreachable
        }
    }
    std::map<std::string,std::string> ldoc;
    for( int karg=optind;karg<argc;karg++ ){
        try{
            create_latex_doc(ldoc,argv[karg],plugin_macro);
        }
        catch(std::exception& e){
            std::cerr << argv[karg] << ": " << e.what() << std::endl;
        }
    }
    std::ofstream ofile(ofname.c_str());
    std::map<std::string,std::string>::const_iterator ldoc_i;
    for( ldoc_i=ldoc.begin();ldoc_i != ldoc.end(); ++ldoc_i ){
        if( ldoc_i->first != "other" ){
            ofile << "\\" << category_macro << "{Plugin category '" << conv2latex(ldoc_i->first) << "'}\n" << std::endl;
            ofile << ldoc_i->second << std::endl;
        }
    }
    for( ldoc_i=ldoc.begin();ldoc_i != ldoc.end(); ++ldoc_i ){
        if( ldoc_i->first == "other" ){
            ofile << "\\" << category_macro << "{Uncategorized plugins}\n" << std::endl;
            ofile << ldoc_i->second << std::endl;
        }
    }
    return 0;
}


// Local Variables:
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
