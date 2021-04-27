// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2010 2013 2014 2015 2016 2017 2018 HörTech gGmbH
// Copyright © 2019 2021 HörTech gGmbH
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

namespace mhachain {

    class mhachain_t : public mhachain::chain_base_t
    {
    public:
        mhachain_t(algo_comm_t iac, const std::string & configured_name);
    };

}

mhachain::mhachain_t::mhachain_t(algo_comm_t iac, const std::string & configured_name)
    : mhachain::chain_base_t(iac, configured_name)
{
    insert_item("use_profiling",&bprofiling);
    insert_item("algos",&algos);
}


MHAPLUGIN_CALLBACKS(mhachain,mhachain::mhachain_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(mhachain,mhachain::mhachain_t,spec,spec)
MHAPLUGIN_PROC_CALLBACK(mhachain,mhachain::mhachain_t,spec,wave)
MHAPLUGIN_PROC_CALLBACK(mhachain,mhachain::mhachain_t,wave,spec)
MHAPLUGIN_DOCUMENTATION\
(mhachain,
 "plugin-arrangement data-flow",
 "Load a sequence of plugins."
 " During processing, the signal is passed from plugin to plugin,"
 " and may change its domain or dimension.\n\n"
 "If profiling is switched on, the cumulative time spent in the processing"
 " callback of each plugin is stored in a monitor variable.\n\n"
 "Plugins are loaded by assigning a vector of strings to the configuration"
 " variable {\\em algos}.  Each entry in this vector has the form"
 " \\textit{plugin}\\textcolor{orange}{\\textit{:configured\\_name}}"
 "\\textcolor{blue}{\\textit{<config\\_file}}, where"
 "\\begin{itemize} \\item \\textit{plugin} is the filename of the plugin"
 " without path or file extension,"
 "\\item \\textcolor{orange}{\\textit{:configured\\_name}} optionally assigns"
 " a different name to this instance of the plugin. This is useful if multiple"
 " instances of the same plugin are loaded into different positions of the"
 " processing chain. the colon and the"
 " \\textcolor{orange}{\\textit{configured\\_name}} are not"
 " specified, then the \\textcolor{orange}{\\textit{configured\\_name}}"
 " defaults to \\textit{plugin}."
 "\\item \\textcolor{blue}{\\textit{<config\\_file}} optionally specifies a"
 " configuration file with which the plugin is initially configured."
 " This is only needed when replacing a complete chain while the mha is"
 " performing signal processing by reassigning {\\em algos}."
 "\\end{itemize}"
 "The plugins loaded by assigning to configuration variable {\\em algos}"
 " cause creation of sub-parsers named like the"
 " \\textcolor{orange}{\\textit{configured\\_name}} in the mhachain plugin"
 " configuration and can be configured through these sub-parsers."
 )

/*
 * Local Variables:
 * compile-command: "make"
 * indent-tabs-mode: nil
 * c-basic-offset: 4
 * coding: utf-8-unix
 * End:
 */
