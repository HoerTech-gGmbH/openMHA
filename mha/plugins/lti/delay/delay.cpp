// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2013 2014 2015 2016 2017  HörTech gGmbH
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
#include "mha_events.h"

namespace delay {

class interface_t : public MHAPlugin::plugin_t<MHASignal::delay_t> {
public:
    interface_t(const algo_comm_t&,const std::string&,const std::string&);
    void prepare(mhaconfig_t&);
    mha_wave_t* process(mha_wave_t*);
private:
    void update();
    MHAParser::vint_t delays;
    MHAEvents::patchbay_t<interface_t> patchbay;
};

interface_t::interface_t(const algo_comm_t& iac,const std::string&,const std::string&)
    : MHAPlugin::plugin_t<MHASignal::delay_t>("Delay line",iac),
      delays("delay in samples, one entry for each channel","[0 0]","[0,[")
{
    insert_item("delay",&delays);
    patchbay.connect(&delays.writeaccess,this,&interface_t::update);
}

void interface_t::update()
{
    if( tftype.channels )
        push_config(new MHASignal::delay_t(delays.data,tftype.channels));
}

void interface_t::prepare(mhaconfig_t& tf)
{
    if( tf.domain != MHA_WAVEFORM )
        throw MHA_ErrorMsg("delay: Only waveform processing is supported.");
    tftype = tf;
    update();
}

mha_wave_t* interface_t::process(mha_wave_t* s)
{
    poll_config();
    return cfg->process(s);
}

}

MHAPLUGIN_CALLBACKS(delay,delay::interface_t,wave,wave)
MHAPLUGIN_DOCUMENTATION(delay,"signalflow","")
