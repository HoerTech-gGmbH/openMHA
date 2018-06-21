// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2013 2014 2015 2016 2018 HörTech gGmbH
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
        return s;
    };
    void prepare(mhaconfig_t& tf)
    {
        if( tf.domain != MHA_SPECTRUM )
            throw MHA_ErrorMsg("save_spec: Only spectral processing is supported.");
        push_config(new MHA_AC::spectrum_t(ac,basename,tf.fftlen/2+1,tf.channels,true));
    }
private:
    std::string basename;
};

MHAPLUGIN_CALLBACKS(save_spec,save_spec_t,spec,spec)
    MHAPLUGIN_DOCUMENTATION(save_spec,"AC-variables","")

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
