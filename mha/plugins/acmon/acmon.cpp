// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2004 2005 2006 2008 2009 2010 2013 2014 2015 HörTech gGmbH
// Copyright © 2017 2018 2019 2021 HörTech gGmbH
// Copyright © 2022 Hörzentrum Oldenburg gGmbH
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
#include "mha_signal.hh"
#include "mha_defs.h"
#include <math.h>
#include "ac_monitor_type.hh"

namespace acmon {
class acmon_t : public MHAPlugin::plugin_t<int> {
public:
    acmon_t(MHA_AC::algo_comm_t &, const std::string & configured_name);
    ~acmon_t();
    void prepare(mhaconfig_t&);
    void release() {};
    mha_spec_t* process(mha_spec_t*);
    mha_wave_t* process(mha_wave_t*);
private:
    void save_vars();
    void update_recmode();
    MHA_AC::algo_comm_t & ac;
    MHAParser::vstring_mon_t varlist;
    MHAParser::vstring_mon_t dimensions;
    MHAParser::kw_t dispmode;
    MHAParser::kw_t recmode;
    std::vector<ac_monitor_t*> vars;
    MHAEvents::patchbay_t<acmon_t> patchbay;
    std::string algo;
    bool b_cont;
    bool b_snapshot;
};

acmon_t::acmon_t(MHA_AC::algo_comm_t & iac, const std::string& configured_name)
    : MHAPlugin::plugin_t<int>("This algorithm converts AC variables into parsable monitor variables.",iac),
      ac(iac),
      varlist("complete list of variables"),
      dimensions("variable dimensions in AC space"),
      dispmode("display mode of variables","vector","[vector matrix]"),
      recmode("record mode","cont","[cont snapshot]"),
      algo(configured_name),
      b_cont(true),
      b_snapshot(false)
{
    vars.clear();
    insert_item("varlist",&varlist);
    insert_item("dimensions",&dimensions);
    insert_item("dispmode",&dispmode);
    insert_item("recmode",&recmode);
    patchbay.connect(&recmode.writeaccess,this,&acmon_t::update_recmode);
}

acmon_t::~acmon_t(void)
{
}

void acmon_t::update_recmode()
{
    b_cont = (recmode.data.get_index()==0);
    b_snapshot = true;
}

void acmon_t::prepare(mhaconfig_t&)
{
    const std::vector<std::string> & entrl = ac.get_entries();
    varlist.data.clear();
    dimensions.data.clear();
    unsigned int k;
    ac_monitor_t* tmp;
    for(k=0;k<vars.size();k++){
        delete vars[k];
    }
    vars.clear();
    for(k=0;k<entrl.size();k++){
        tmp = new ac_monitor_t(*((MHAParser::parser_t*)this),entrl[k],ac,dispmode.data.get_index()!=0);
        vars.push_back(tmp);
        varlist.data.push_back(tmp->name);
        dimensions.data.push_back(tmp->dimstr);
    }
    save_vars();
}

mha_spec_t* acmon_t::process(mha_spec_t* s)
{
    save_vars();
    return s;
}

mha_wave_t* acmon_t::process(mha_wave_t* s)
{
    save_vars();
    return s;
}

void acmon_t::save_vars()
{
    if( b_cont || b_snapshot )
        for(unsigned int k=0;k<vars.size();k++)
            vars[k]->getvar(ac);
    b_snapshot = false;
}

}

MHAPLUGIN_CALLBACKS(acmon,acmon::acmon_t,spec,spec)
MHAPLUGIN_PROC_CALLBACK(acmon,acmon::acmon_t,wave,wave)
MHAPLUGIN_DOCUMENTATION\
(acmon,
 "data-export network-communication",
 "")

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
