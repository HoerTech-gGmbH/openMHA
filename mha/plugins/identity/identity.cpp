// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2018 HörTech gGmbH
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


#include "mha_plugin.hh"

class identity_t : public MHAPlugin::plugin_t<int> 
{
public:
    identity_t(const algo_comm_t&,const std::string&,const std::string&);
    mha_wave_t* process(mha_wave_t*);
    mha_spec_t* process(mha_spec_t*);
    void prepare(mhaconfig_t&);
    void release();
};

identity_t::identity_t(const algo_comm_t& iac,
                       const std::string&,
                       const std::string&)
    : MHAPlugin::plugin_t<int>("",iac)
{
}

void identity_t::prepare(mhaconfig_t& tf)
{
}

void identity_t::release()
{
}

mha_wave_t* identity_t::process(mha_wave_t* s)
{
    return s;
}

mha_spec_t* identity_t::process(mha_spec_t* s)
{
    return s;
}

MHAPLUGIN_CALLBACKS(identity,identity_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(identity,identity_t,spec,spec)

MHAPLUGIN_DOCUMENTATION(identity,
    "core",
    "The simplest \\mha{} plugin.\n\n"
    "This plugin does not modify the signal."
)

// Local variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
