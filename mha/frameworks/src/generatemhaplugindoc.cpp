// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2007 2008 2013 2014 2016 2017 2018 2019 HörTech gGmbH
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
#include <set>

/** Escapes various character sequences in texts not intended to be processed
 * by LaTeX for processing by LaTeX.  Focus is on correct display of symbols
 * contained in these texts.  E.g. the help texts of MHA configuration variables
 * can be processed by this function.
 * The contents of the MHAPLUGIN_DOCUMENTATION is already in LaTeX format
 * and should not be processed by this function. 
 * @return A copy of s with various symbols escaped for LaTeX processing
 * @param s Text not ready for LaTeX
 * @param iscolored if true, the complete returned text is surrounded with
 *                  "\\color{monitorcolor}{" and "}"
 */
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

/** Class to access the information stored in the plugin source code's
 * MHAPLUGIN_DOCUMENTATION macro.
 */
class latex_doc_t {
public:
    /** Constructor loads the plugin into this process.
     * @param plugname Name of the MHA plugin to process
     * @param plugin_macro name of the LaTeX section macro that
     *        documents a single plugin (e.g. "section", "subsection",
     *        "subsubsection", ...)
     */ 
    latex_doc_t(const std::string& plugname,const std::string& plugin_macro);
    /** This method accesses the compiled-in contents of the
     * MHAPLUGIN_DOCUMENTATION macro and the exported interface
     * functions of the loaded plugin to produce latex docuementation
     * for the plugin.  It tentatively prepares the plugin for
     * processing and checks the AC variables registered by the
     * plugin.
     * @return the complete latex documentation for this plugin */
    std::string get_latex_doc();
    /** @return the first word of the categories string in the
     * MHAPLUGIN_DOCUMENTATION macro */
    std::string get_main_category() const;
    /** @return a vector of all words in the categories string in the
     * MHAPLUGIN_DOCUMENTATION macro */
    std::vector<std::string> get_categories() const;
private:
    std::string strdom(mha_domain_t d) const;
    std::string get_ac(MHAKernel::algo_comm_class_t& ac,std::string txt) const;
    std::string parsername(std::string s) const;
    std::string get_parser_var(MHAParser::base_t* p,std::string name) const;
    std::string get_parser_tab(MHAParser::base_t* p,
                               const std::string & prefix,
                               const std::string & latex_macro) const;
    const std::string plugname;
    const std::string latex_plugname;
    MHAKernel::algo_comm_class_t ac;
    PluginLoader::mhapluginloader_t loader;
    const std::string plugin_macro;
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
    if (plugin_macro == "subsection")
        plugin_subsection_macro = "\\subsubsection";
    if (plugin_macro == "section")
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
        retv += plugin_subsection_macro + "{Detailed description}\n\n" +
            loader.get_documentation() + "\n\n";
    retv += plugin_subsection_macro +
        "{Supported domains}\n\nThe MHA plugin {\\tt " + latex_plugname +
        "} supports these signal domains:\n\\begin{itemize}\n";
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
        retv += "\n\n"+ plugin_subsection_macro +
            "{Plugin Tags}\n\n"
            "\\hyperref[mha-tag:" + cats[0] + "]"
            "{\\emph{" + conv2latex(cats[0]) + "}}{\\/} ";
        for( unsigned int k=1;k<cats.size();k++){
            retv += "\\hyperref[mha-tag:" + cats[k] + "]"
                "{" + conv2latex(cats[k]) + "} ";
        }
        retv += "\n\n";
    }
    if( loader.has_parser() ){
        retv += get_parser_tab(&loader,"",plugin_subsection_macro);
    }
    return retv;
}

std::string latex_doc_t::get_main_category() const
{
    return loader.get_categories()[0];
}

std::vector<std::string> latex_doc_t::get_categories() const
{
    return loader.get_categories();
}

std::string latex_doc_t::strdom( mha_domain_t d ) const
{
    switch( d ){
    case MHA_WAVEFORM : return "waveform";
    case MHA_SPECTRUM : return "spectrum";
    default: return "unknwon";
    }
}

std::string latex_doc_t::get_ac(MHAKernel::algo_comm_class_t& ac,
                                std::string txt) const
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

std::string latex_doc_t::parsername(std::string s) const
{
    if( s.rfind(".") < s.size() )
        s.erase(0,s.rfind(".")+1);
    return s;
}

std::string latex_doc_t::get_parser_var(MHAParser::base_t* p,std::string name) const
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

std::string latex_doc_t::get_parser_tab(MHAParser::base_t* p,
                                        const std::string & prefix,
                                        const std::string & latex_macro) const
{   // latex_macro is "\\paragraph" or "\\subsubsection" or similar
    std::string retv;
    if( p->parse(prefix+"?type") == "parser" ){
        std::vector<std::string> entries;
        MHAParser::StrCnv::str2val(p->parse(prefix+"?entries"),entries);
        if( entries.size() ){
            unsigned int k;
            if( prefix.size() )
                retv += "\n\nVariables of sub-parser {\\tt " + conv2latex(parsername(prefix)) + "}:\n\n";
            else
                retv += "\n" + latex_macro + "{Configuration variables}\n\n";
            retv += "{\\small\\begin{tabular}{|l|l|p{6cm}|l|}\n\\hline\n";
            retv += "{\\bf\\normalsize Name}&{\\bf\\normalsize Type}&{\\bf\\normalsize Description}&{\\bf\\normalsize Default}\\\\\\hline\n";
            for(k=0;k<entries.size();k++){
                retv += get_parser_var(p,prefix+"."+entries[k]);
                retv += "\\hline\n";
            }
            retv += "\\end{tabular}}\n";
            for(k=0;k<entries.size();k++){
                if( p->parse(prefix+"."+entries[k]+"?type")=="parser")
                    retv += get_parser_tab(p,prefix+"."+entries[k],latex_macro);
            }
        }
    }else{
        if( prefix.size() == 0 ){
            retv += latex_macro + "{Configuration}\n\n"
                "The plugin represents a variable node"
                " in the MHA configuration hierarchy.\n\n";
            retv += "{\\small\\begin{tabular}{|ll|p{5cm}|l|}\n\\hline\n";
            retv += "&{\\bf\\normalsize Type}&{\\bf\\normalsize Description}&{\\bf\\normalsize Default}\\\\\\hline\n";
            retv += get_parser_var(p,prefix);
            retv += "\\hline\n\\end{tabular}}\n";
        }
    }
    return retv;
}

/** Function prints an overview of all categories and their associated
 * plugins into the document.
 * @param all_categories A sorted container with all category names
 * @param main_category_plugins  map of main categories to plugin names
 * @param additional_category_plugins  map of tags to plugin names
 * @param ofile Latex document is produced by writing output to this stream
 */
static void print_plugin_references
(const std::set<std::string> & all_categories,
 std::map<std::string,std::vector<std::string> > main_category_plugins,
 std::map<std::string,std::vector<std::string> > additional_category_plugins,
 std::ofstream & ofile,
 const std::string & category_macro);
                             
/** Loads the plugin, creates the latex documentation for the plugin,
 * and adds the latex documentation for this plugin to the plugin's
 * main category entry in doc. 
 * @return the vector of all categories.
 * @param doc map of main categories to a string containint the documentation
 *            of all plugins in that categories.  The documentation of the
 *            current plugin will be appended to the existing documentation of
 *            its main category.  Will be created if non-existant.
 * @param plugname Name of the MHA plugin to process
 * @param plugin_macro name of the LaTeX section macro that
 *        documents a single plugin (e.g. "section", "subsection",
 *        "subsubsection", ...) */
std::vector<std::string>
create_latex_doc(std::map<std::string, std::string> & doc,
                 const std::string& plugname,
                 const std::string& plugin_macro)
{
    latex_doc_t ldoc(plugname,plugin_macro);
    doc[ldoc.get_main_category()] += ldoc.get_latex_doc();
    return ldoc.get_categories();
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
    // map of main categories to the documentation of declaring plugins
    std::map<std::string,std::string> ldoc;
    // map of main categories to plugin names
    std::map<std::string,std::vector<std::string> > main_category_plugins;
    // map of tags to plugin names
    std::map<std::string,std::vector<std::string> > additional_category_plugins;
    // set of all categories and tags
    std::set<std::string> all_categories;
    
    for( int karg=optind;karg<argc;karg++ ){
        try{
            std::vector<std::string> categories =
                create_latex_doc(ldoc,argv[karg],plugin_macro);
            
            if (std::find(categories.begin(), categories.end(), "other")
                != categories.end()) {
                std::cerr << "generatemhaplugindoc: error: Plugin " <<
                    argv[karg] << " has deprecated category \"other\"" <<
                    std::endl;
                exit(EXIT_FAILURE); // Having uncategorized plugins is an error
            }
            if (categories.empty()) {
                std::cerr << "generatemhaplugindoc: error: Plugin " <<
                    argv[karg] << " has no categories" << std::endl;
                exit(EXIT_FAILURE); // Having uncategorized plugins is an error
            }
            main_category_plugins[categories[0]].push_back(argv[karg]);
            for (size_t index = 1; index < categories.size(); ++index)
                additional_category_plugins[categories[index]].
                    push_back(argv[karg]);
            all_categories.insert(categories.begin(),categories.end());
        }
        catch(std::exception& e){
            std::cerr << "generatemhaplugindoc: error: Plugin " << argv[karg] <<
                ": " << e.what() << std::endl;
            exit(EXIT_FAILURE); // Having undocumented plugins is an error
        }
    }
    std::ofstream ofile(ofname.c_str());
    std::map<std::string,std::string>::const_iterator ldoc_i;
    for( ldoc_i=ldoc.begin();ldoc_i != ldoc.end(); ++ldoc_i ){
        ofile << "\\" << category_macro << "{Plugin category '" << conv2latex(ldoc_i->first) << "'}\n" << std::endl;
        ofile << ldoc_i->second << std::endl;
    }

    print_plugin_references(all_categories,
                            main_category_plugins,
                            additional_category_plugins,
                            ofile,
                            category_macro);

    return EXIT_SUCCESS;
}

static void print_plugin_references
(const std::set<std::string> & all_categories,
 std::map<std::string,std::vector<std::string> > main_category_plugins,
 std::map<std::string,std::vector<std::string> > additional_category_plugins,
 std::ofstream & ofile,
 const std::string & category_macro)
{
    const char * const emphasis[2] = {"\\textit","\\textbf"};

    for (const std::string& c : all_categories) {
        ofile << "\\" << category_macro << "{All plugins tagged '"
              << conv2latex(c) << "'}\n"
              << "\\label{mha-tag:" << c << "}\n"
              << "\\begin{itemize}\n";
        std::map<std::string, bool> plugins_main_or_additional;
        for (const std::string& p : main_category_plugins[c])
            plugins_main_or_additional[p] = true;
        for (const std::string& p : additional_category_plugins[c])
            plugins_main_or_additional[p] = false;
        
        for (const std::pair<std::string,bool> p : plugins_main_or_additional)
            ofile << "\\item "
                  << emphasis[p.second] << "{" << conv2latex(p.first) << "}: "
                  << "Section \\ref{plug:" + p.first + "} "
                  << "on page \\pageref{plug:" + p.first + "}\n";
        ofile << "\\end{itemize}\n\n";
    }
}

// Local Variables:
// c-basic-offset: 4
// indent-tabs-mode: nil
// compile-command: "make -C .."
// coding: utf-8-unix
// End:
