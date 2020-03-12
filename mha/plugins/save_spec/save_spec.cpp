// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2009 2010 2013 2014 2015 2018 2019 2020 HörTech gGmbH
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

class save_spec_t : public MHAPlugin::plugin_t<MHA_AC::spectrum_t> {
public:
    save_spec_t(const algo_comm_t& iac,
                const std::string& ith,
                const std::string& ial)
        : MHAPlugin::plugin_t<MHA_AC::spectrum_t>("Save signal spectrum to AC variable",iac),
          basename(ial){};
    mha_spec_t* process(mha_spec_t* s)
    {
        poll_config();
        cfg->copy(*s);
        cfg->insert();
        return s;
    };
    void prepare(mhaconfig_t& tf)
    {
        if( tf.domain != MHA_SPECTRUM )
            throw MHA_ErrorMsg("save_spec: Only spectral processing is supported.");

        // Last argument (true) triggers insertion of spectrum into AC space.
        // This is only permitted because we are in prepare().  Do not copy this
        // scheme into the configuration update() callback in other plugins.
        push_config(new MHA_AC::spectrum_t(ac,basename,tf.fftlen/2+1,tf.channels,true));
    }
private:
    std::string basename;
};

MHAPLUGIN_CALLBACKS(save_spec,save_spec_t,spec,spec)
MHAPLUGIN_DOCUMENTATION\
(save_spec,
 "data-flow algorithm-communication",
 "This plugin saves the spectral signal to an AC variable."
 " The name of the variable is the same as the name of the plugin\n"
 "and can be changed by assigning an alias to the plugin with the"
 " usual plugin\\_name:alias\\_name syntax.")


// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
