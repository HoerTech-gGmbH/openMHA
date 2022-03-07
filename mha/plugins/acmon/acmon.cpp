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
#include <memory>
#include "ac_monitor_type.hh"


/** Namespace containing the acmon plugin interface class */
namespace acmon {

    /** acmon plugin interface class */
class acmon_t : public MHAPlugin::plugin_t<int> {
public:
    /** Plugin interface constructor.
     * @param configured_name Assigned name of the plugin within the configuration tree
     */
    acmon_t(MHA_AC::algo_comm_t &, const std::string & configured_name);
    /** Default destructor*/
    ~acmon_t()=default;
    /** Prepare callback. Initializes the AC to monitor bridges and variable list, leaves the signal dimensions untouched*/
    void prepare(mhaconfig_t&);
    /** Do-nothing release */
    void release() {};
    /** Process callback for frequency domain. Calls save_vars() and returns the signal unmodified.*/
    mha_spec_t* process(mha_spec_t*);
    /** Process callback for time domain. Calls save_vars() and returns the signal unmodified.*/
    mha_wave_t* process(mha_wave_t*);
private:
    /** Save the current value of the AC variables into their corresponding monitors. */
    void save_vars();
    /** Set/Re-Set the recording mode from continous to snapshot or vice versa */
    void update_recmode();
    /** Handle to the AC space */
    MHA_AC::algo_comm_t & ac;
    /** Monitor variable containing the AC variable names */
    MHAParser::vstring_mon_t varlist;
    /** Monitor variable containing the dimension strings of the AC variables */
    MHAParser::vstring_mon_t dimensions;
    /** Configuration variable for the display mode */
    MHAParser::kw_t dispmode;
    /** Configuration variable for the record mode */
    MHAParser::kw_t recmode;
    /** Vector containing pointers to the ac to monitor bridge variables. This plugin does not clean up after itself */
    std::vector<std::unique_ptr<ac_monitor_t>> vars;
    /** Patchbay*/
    MHAEvents::patchbay_t<acmon_t> patchbay;
    /** String saving the configured name of the plugin */
    std::string algo;
    /** Recording mode. True for continous mode, false for snapshot mode */
    bool b_cont;
    /** Snapshot flag. Set to true to request a snapshot in the next process() call */
    bool b_snapshot;
};

acmon_t::acmon_t(MHA_AC::algo_comm_t & iac, const std::string& configured_name)
    : MHAPlugin::plugin_t<int>("This plugin converts AC variables into parsable monitor variables.",iac),
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
    vars.clear();
    for(const auto & entry : entrl){
        vars.emplace_back(std::make_unique<ac_monitor_t>(*((MHAParser::parser_t*)this),entry,ac,dispmode.data.get_index()!=0));
        varlist.data.push_back(vars.back()->name);
        dimensions.data.push_back(vars.back()->dimstr);
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

} //namespace acmon

MHAPLUGIN_CALLBACKS(acmon,acmon::acmon_t,spec,spec)
MHAPLUGIN_PROC_CALLBACK(acmon,acmon::acmon_t,wave,wave)
MHAPLUGIN_DOCUMENTATION\
(acmon,
 "data-export network-communication",
 "This plugin converts AC variables into parsable monitor variables. It publishes a monitor variable"
 " containing the name of every AC variable, a monitor variable containing the dimensions of every AC"
 " variable and a monitor variable corresponding to every displayable AC variable. Currently all AC"
 " variables of numerical types and character types are supported. \n\n The monitor variables are either"
 " updated continously (recmode=\"cont\") or on request (recmode=\"snapshot\"). To request a snapshot"
 " set recmode to \"snapshot\". Multidimensional AC variables can either be displayed linearized, i.e. as vector,"
 " or as matrix, using the stride and number of entries to determine the number of rows and columns."
 " This behavior is controlled by the \"dismode\" configuration variable. The size information monitor is"
 " not updated after the prepare command.")

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
