// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2010 2012 2013 2014 2015 2017 2018 2019 2020 HörTech gGmbH
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

#include "mha_multisrc.h"

namespace route {

    class process_t {
    public:
        process_t(algo_comm_t iac,
                  const std::string acname,
                  const std::vector<std::string>& r_out,
                  const std::vector<std::string>& r_ac,
                  const mhaconfig_t& cf_in,
                  const mhaconfig_t& cf_out,
                  const mhaconfig_t& cf_ac,
                  bool sync);
        mha_wave_t* process(mha_wave_t*);
        mha_spec_t* process(mha_spec_t*);
    private:
        MHAMultiSrc::waveform_t wout;
        MHAMultiSrc::spectrum_t sout;
        MHAMultiSrc::waveform_t wout_ac;
        MHAMultiSrc::spectrum_t sout_ac;
    };

    class interface_t : public MHAPlugin::plugin_t<route::process_t>
    {
    public:
        interface_t(algo_comm_t iac,const std::string&,const std::string&);
        void prepare(mhaconfig_t&);
        void release();
        mha_wave_t* process(mha_wave_t*);
        mha_spec_t* process(mha_spec_t*);
    private:
        void update();
        MHAEvents::patchbay_t<route::interface_t> patchbay;
        MHAParser::vstring_t route_out;
        MHAParser::vstring_t route_ac;
        mhaconfig_t cfin;
        mhaconfig_t cfout;
        mhaconfig_t cfac;
        bool prepared;
        bool stopped;
        std::string algo;
    };

}

route::interface_t::interface_t(algo_comm_t iac,const std::string&,const std::string& ialg)
    : MHAPlugin::plugin_t<route::process_t>(
        "Signal router plugin.\n\n"
        "Arguments are the input signal source names (AC variables)\n"
        "followed by a colon, followed by the channel number, starting\n"
        "at zero. Empty names correspond to the direct input.\n\n"
        "An AC variable will be created if the AC output dimension is not zero.\n"
        "Example:\n  out = [:0 :1 x:0 x:1]\n  ac = [:2 :3]\n"
        "returns a four channel output signal containing first two\n"
        "direct input channels, and the first two channels of the AC\n"
        "variable \"x\". An AC variable is created with the third and\n"
        "fourth channel of the direct input.\n\n",iac), 
      route_out("direct output","[]"),
      route_ac("AC output","[]"),
      prepared(false), 
      stopped(true),
      algo(ialg)
{
    insert_item("out",&route_out);
    insert_item("ac",&route_ac);
    patchbay.connect(&writeaccess,this,&route::interface_t::update);
}

void route::interface_t::prepare(mhaconfig_t& cf)
{
    cfin = cf;
    cf.channels = route_out.data.size();
    cfout = cf;
    cfac = cfout;
    cfac.channels = route_ac.data.size();
    prepared = true;
    update();
    stopped = false;
}

void route::interface_t::release()
{
    prepared = false;
    stopped = true;
}

void route::interface_t::update()
{
    if( prepared )
        push_config(new route::process_t(ac,
                                         algo,
                                         route_out.data,
                                         route_ac.data,
                                         cfin,
                                         cfout,
                                         cfac,stopped));
}

mha_wave_t* route::interface_t::process(mha_wave_t* s)
{
    poll_config();
    return cfg->process(s);
}

mha_spec_t* route::interface_t::process(mha_spec_t* s)
{
    poll_config();
    return cfg->process(s);
}

route::process_t::process_t(algo_comm_t ac,
                            const std::string acname,
                            const std::vector<std::string>& r_out,
                            const std::vector<std::string>& r_ac,
                            const mhaconfig_t& cf_in,
                            const mhaconfig_t& cf_out,
                            const mhaconfig_t& cf_ac,
                            bool sync)
    : wout(ac,"",cf_out.fragsize, cf_out.channels),
      sout(ac,"",cf_out.fftlen/2+1, cf_out.channels),
      wout_ac(ac,acname,cf_ac.fragsize, cf_ac.channels),
      sout_ac(ac,acname,cf_ac.fftlen/2+1, cf_ac.channels)
{
    if( r_out.size() != cf_out.channels )
        throw MHA_Error(__FILE__,__LINE__,
                        "Expected %u entries in output route variable, got %zu.",
                        cf_out.channels,r_out.size());
    if( r_ac.size() != cf_ac.channels )
        throw MHA_Error(__FILE__,__LINE__,
                        "Expected %u entries in AC route variable, got %zu.",
                        cf_ac.channels,r_ac.size());
    wout.select_source(r_out,cf_in.channels);
    sout.select_source(r_out,cf_in.channels);
    wout_ac.select_source(r_ac,cf_in.channels);
    sout_ac.select_source(r_ac,cf_in.channels);
    if( sync && (cf_in.domain == MHA_WAVEFORM) && (cf_ac.channels > 0) )
        wout_ac.insert();
    if( sync && (cf_in.domain == MHA_SPECTRUM) && (cf_ac.channels > 0) )
        sout_ac.insert();
}

mha_wave_t* route::process_t::process(mha_wave_t* s)
{
    wout_ac.update(s);
    return wout.update(s);
}

mha_spec_t* route::process_t::process(mha_spec_t* s)
{
    sout_ac.update(s);
    return sout.update(s);
}

MHAPLUGIN_CALLBACKS(route,route::interface_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(route,route::interface_t,spec,spec)
MHAPLUGIN_DOCUMENTATION\
(route,
 "data-flow audio-channels algorithm-communication",
 "")

// Local Variables:
// compile-command: "make"
// indent-tabs-mode: nil
// c-basic-offset: 4
// coding: utf-8-unix
// End:
