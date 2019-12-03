// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2007 2010 2012 2013 2014 2015 2018 2019 HörTech gGmbH
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

#define PASCALE 93.979400086720374929

class levelmeter_t : public MHAPlugin::plugin_t<MHASignal::async_rmslevel_t>
{
public:
  levelmeter_t(const algo_comm_t&,const std::string&,const std::string);
  mha_wave_t* process(mha_wave_t*);
  void prepare(mhaconfig_t&);
private:
  void update_tau();
  void query_rms();
  void query_peak();
  MHAParser::float_t tau;
  MHAParser::kw_t mode;
  MHAParser::vfloat_mon_t rms;
  MHAParser::vfloat_mon_t peak;
  MHAEvents::patchbay_t<levelmeter_t> patchbay;
};

levelmeter_t::levelmeter_t(const algo_comm_t& iac,const std::string&,const std::string)
  : MHAPlugin::plugin_t<MHASignal::async_rmslevel_t>("Broadband level meter.",iac),
  tau("RMS time constant / s","0.1","[0,]"),
  mode("Level scale","Pa","[Pa FS]"),
  rms("RMS level / dB"),
  peak("Peak level / dB")
{
  insert_member(tau);
  insert_member(mode);
  insert_member(rms);
  insert_member(peak);
  patchbay.connect(&tau.writeaccess,this,&levelmeter_t::update_tau);
  patchbay.connect(&rms.prereadaccess,this,&levelmeter_t::query_rms);
  patchbay.connect(&peak.prereadaccess,this,&levelmeter_t::query_peak);
}

mha_wave_t* levelmeter_t::process(mha_wave_t* s)
{
  poll_config()->process(s);
  return s;
}

void levelmeter_t::update_tau()
{
  push_config(new MHASignal::async_rmslevel_t(std::max(1u,(unsigned int)(tftype.srate*tau.data)),tftype.channels));
}

void levelmeter_t::query_rms()
{
  if( cfg ){
    rms.data = cfg->rmslevel();
    if( mode.data.get_index() == 1 ){
      for(unsigned int k=0;k<rms.data.size();k++)
        rms.data[k] -= PASCALE;
    }
  }else{
    rms.data = std::vector<float>(tftype.channels,-200.0f);
  }
}

void levelmeter_t::query_peak()
{
  if( cfg ){
    peak.data = cfg->peaklevel();
    if( mode.data.get_index() == 1 ){
      for(unsigned int k=0;k<peak.data.size();k++)
        peak.data[k] -= PASCALE;
    }
  }else{
    peak.data = std::vector<float>(tftype.channels,-200.0f);
  }
}

void levelmeter_t::prepare(mhaconfig_t& cf)
{
  if( cf.domain != MHA_WAVEFORM )
    throw MHA_Error(__FILE__,__LINE__,"Only waveform processing is supported.");
  tftype = cf;
  update_tau();
}

MHAPLUGIN_CALLBACKS(levelmeter,levelmeter_t,wave,wave)
MHAPLUGIN_DOCUMENTATION(levelmeter,
                        "level compression",
                        "This level meter calculates the RMS level of the input signal in"
                        " the last {\\em tau} seconds."
                        )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
