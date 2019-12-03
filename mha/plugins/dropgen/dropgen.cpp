// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2019 HörTech gGmbH
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
#include <random>
#include <unistd.h>
class dropgen_t : public MHAPlugin::plugin_t<int>
{
public:
    dropgen_t(const algo_comm_t&,const std::string&,const std::string&);
    mha_wave_t* process(mha_wave_t*);
    mha_spec_t* process(mha_spec_t*);
    void prepare(mhaconfig_t&);
    void release();
    MHAParser::float_t min_sleep_time;
    MHAParser::float_t max_sleep_time;
    MHAParser::float_t chance;
    MHAEvents::patchbay_t<dropgen_t> patchbay;
    std::random_device r;
    std::mt19937 random_engine;
    std::uniform_real_distribution<> dis;
};

dropgen_t::dropgen_t(const algo_comm_t& iac,
                     const std::string&,
                     const std::string&)
    : MHAPlugin::plugin_t<int>("",iac),
      min_sleep_time("minimum sleep time, in s","0","[0,["),
      max_sleep_time("minimum sleep time, in s","0","[0,["),
      chance("chance of an artificial dropout","0","[0,["),
      random_engine(r()),
      dis(0,1)
{
    insert_item("min_sleep_time",&min_sleep_time);
    insert_item("max_sleep_time",&max_sleep_time);
    insert_item("chance",&chance);
}

void dropgen_t::prepare(mhaconfig_t& tf)
{
}

void dropgen_t::release()
{
}

mha_wave_t* dropgen_t::process(mha_wave_t* s)
{
    float x = dis(random_engine);
    if (x > (1 - chance.data)) {
        float t =
            (min_sleep_time.data +
             dis(random_engine) * (max_sleep_time.data - min_sleep_time.data)) *
            1000.0f;
        std::cerr<<"sleeping for "<<t<<"ms...";
        mha_msleep(t);
        std::cerr<<" done!\n";
    }
    return s;
}

mha_spec_t* dropgen_t::process(mha_spec_t* s)
{
    return s;
}

MHAPLUGIN_CALLBACKS(dropgen,dropgen_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(dropgen,dropgen_t,spec,spec)

MHAPLUGIN_DOCUMENTATION\
(dropgen,
 "test-tool",
 "This plugin randomly generates dropouts by waiting between 1 and 10 frames in .5% of frames. \n\n"
 "This plugin does not otherwise modify the signal. Do not include this plugin in production setups."
 )

// Local variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
