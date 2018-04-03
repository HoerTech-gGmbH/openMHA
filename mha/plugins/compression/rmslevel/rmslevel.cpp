// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2009 2010 2013 2014 2015 2017 HörTech gGmbH
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
#include "mha_parser.hh"
#include "mha_defs.h"
#include <math.h>
#include <limits>

using namespace MHAPlugin;

class mon_t : public MHA_AC::waveform_t, private MHAParser::vfloat_mon_t {
public:
    mon_t(unsigned int nch,std::string name,algo_comm_t ac,std::string base,MHAParser::parser_t& p,std::string help);
    void store();
};

mon_t::mon_t(unsigned int nch,std::string name,algo_comm_t ac,std::string base,MHAParser::parser_t& p,std::string help)
    : MHA_AC::waveform_t(ac,base+"_"+name,1,nch,false),
      MHAParser::vfloat_mon_t(help)
{
    p.force_remove_item(name);
    p.insert_item(name,this);
    data.resize(nch);
}

void mon_t::store()
{
    for(unsigned int k=0;k<get_size();k++)
        data[k] = buf[k];
}

class rmslevel_t {
public:
    rmslevel_t(unsigned int nch,algo_comm_t ac,std::string name,MHAParser::parser_t& p,unsigned int fftlen_);
    mha_spec_t* process(mha_spec_t*);
    mha_wave_t* process(mha_wave_t*);
    void insert();
private:
    mon_t level_db;
    mon_t peak_db;
    mon_t level;
    mon_t peak;
    unsigned int fftlen;
};

class rmslevel_if_t : public plugin_t<rmslevel_t> {
public:
    rmslevel_if_t(const algo_comm_t&,const std::string&,const std::string&);
    mha_spec_t* process(mha_spec_t*);
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
private:
    std::string name;
};

void rmslevel_t::insert()
{
    level_db.insert();
    peak_db.insert();
    level.insert();
    peak.insert();
}

rmslevel_t::rmslevel_t(unsigned int nch,algo_comm_t ac,std::string name,MHAParser::parser_t& p,unsigned int fftlen_)
    : level_db(nch,"level_db",ac,name,p,"RMS level in dB"),
      peak_db(nch,"peak_db",ac,name,p,"peak amplitude in dB"),
      level(nch,"level",ac,name,p,"RMS level in W/m^2"),
      peak(nch,"peak",ac,name,p,"peak amplitude in Pa"),
      fftlen(fftlen_)
{
}

rmslevel_if_t::rmslevel_if_t(const algo_comm_t& iac,const std::string& ith,const std::string& ial)
    : plugin_t<rmslevel_t>(
        "This algorithm displays block based RMS level informations.\n"
        "Results are stored in these AC variables (replace 'rmslevel'\n"
        "by the configured plugin name):\n\n"
        "  rmslevel_level_db\n"
        "  rmslevel_peak_db\n"
        "  rmslevel_level\n"
        "  rmslevel_peak\n"
        ,iac),
      name(ial)
{
}

mha_spec_t* rmslevel_if_t::process(mha_spec_t* s)
{
    poll_config();
    return cfg->process(s);
}

mha_wave_t* rmslevel_if_t::process(mha_wave_t* s)
{
    poll_config();
    return cfg->process(s);
}

mha_spec_t* rmslevel_t::process(mha_spec_t* s)
{
    //insert();
    unsigned int ch;
    for(ch=0; ch<s->num_channels;ch++){
        level[ch] = std::max(MHASignal::rmslevel(*s,ch,fftlen),2e-10f);
        peak[ch] = std::max(MHASignal::maxabs(*s,ch),2e-10f);
        level_db[ch] = MHASignal::pa2dbspl(level[ch]);
        peak_db[ch] = MHASignal::pa2dbspl(peak[ch]);
    }
    level.store();
    peak.store();
    level_db.store();
    peak_db.store();
    return s;
}

mha_wave_t* rmslevel_t::process(mha_wave_t* s)
{
    //insert();
    unsigned int ch;
    for(ch=0; ch<s->num_channels;ch++){
        level[ch] = std::max(MHASignal::rmslevel(*s,ch),2e-10f);
        peak[ch] = std::max(MHASignal::maxabs(*s,ch),2e-10f);
        level_db[ch] = MHASignal::pa2dbspl(level[ch]);
        peak_db[ch] = MHASignal::pa2dbspl(peak[ch]);
    }
    level.store();
    peak.store();
    level_db.store();
    peak_db.store();
    return s;
}

void rmslevel_if_t::prepare(mhaconfig_t& tf)
{
    tftype = tf;
    push_config(new rmslevel_t(tf.channels,ac,name,static_cast<MHAParser::parser_t&>(*this),tf.fftlen));
    poll_config();
    cfg->insert();
}

MHAPLUGIN_CALLBACKS(rmslevel,rmslevel_if_t,spec,spec)
    MHAPLUGIN_PROC_CALLBACK(rmslevel,rmslevel_if_t,wave,wave)
    MHAPLUGIN_DOCUMENTATION(rmslevel,"level","")
    
// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:

