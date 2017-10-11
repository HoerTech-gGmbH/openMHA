// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2009 2010 2013 2014 2015 HörTech gGmbH
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

class save_wave_t : public MHAPlugin::plugin_t<MHA_AC::waveform_t> {
public:
    save_wave_t(const algo_comm_t& iac,
                const std::string& ith,
                const std::string& ial)
        : MHAPlugin::plugin_t<MHA_AC::waveform_t>("Save signal waveform to AC variable",iac),
          basename(ial){};
    mha_wave_t* process(mha_wave_t* s)
    {
        //poll_config();
	//cfg->insert();
        cfg->copy(*s);
        return s;
    };
    void prepare(mhaconfig_t& tf)
    {
        if( tf.domain != MHA_WAVEFORM )
            throw MHA_ErrorMsg("save_wave: Only waveform processing is supported.");
        push_config(new MHA_AC::waveform_t(ac,basename,tf.fragsize,tf.channels,false));
	poll_config();
	cfg->insert();
    }
private:
    std::string basename;
};

MHAPLUGIN_CALLBACKS(save_wave,save_wave_t,wave,wave)
    MHAPLUGIN_DOCUMENTATION(save_wave,"AC-variables","")

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
