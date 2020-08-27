// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2012 2013 2014 2015 2018 2019 2020 HörTech gGmbH
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

#include <math.h>
#include <algorithm>

namespace cpuload{
  class cpuload_cfg_t {

  public:

    /// Ctor of the runtime configuration class
    /// @param factor_ cpu load factor. Values > 1 increase cpu load, values < 1 decrease it
    /// @param table_size_
    /// @param use_sine_
    cpuload_cfg_t(mha_real_t factor_, size_t table_size_, bool use_sine_);

    /// Process callback. Does not actually change signal.
    void process(unsigned fac_);
  private:
    void calc_sine();
    void write_to_table();

    /// Phase of the sine
    float phase=0;
    /// Result of sin(phase). Volatile to prevent compiler from optimizing away the calculation
    volatile float result=0;
    /// Use sine or do table operation
    bool use_sine=false;
    /// cpu load factor. Values > 1 increase cpu load, values < 1 decrease it
    float factor=0;
    /// Table with arbitrary values to operate on. Unused if use_sine=true.
    std::vector<float> table;
  };

  void cpuload_cfg_t::calc_sine() {
    result = sinf(phase++);
    if (phase > M_PI) phase -= 2*M_PI;
  }

  void cpuload_cfg_t::write_to_table() {
    auto value=rand();
    for(auto & elm : table){
      elm=value;
    }
  }

  cpuload_cfg_t::cpuload_cfg_t(mha_real_t factor_, size_t table_size_, bool use_sine_):
    use_sine(use_sine_),
    factor(factor_)
  {
    if(!use_sine_){
      table.resize(table_size_);
      for (unsigned i = 0; i < table.size(); ++i) {
        table[i] = rand();
      }
    }
    else{
      phase=0;
    }
  }

  void cpuload_cfg_t::process(unsigned fac_) {
    if(use_sine){
      for (unsigned n = 0;n < (fac_ * 10 * factor);++n) {
        calc_sine();
      }
    }
    else{
      for (unsigned n = 0;n < (fac_ * 10 * factor);++n) {
        write_to_table();
      }
    }
  }

  class cpuload_if_t :  public MHAPlugin::plugin_t<cpuload_cfg_t> {
  public:
    cpuload_if_t(algo_comm_t , const char*, const char*);
    mha_spec_t* process(mha_spec_t*);
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
  private:
    void update();
    MHAParser::float_t factor;
    MHAParser::int_t table_size;
    MHAParser::bool_t use_sine;
    MHAEvents::patchbay_t<cpuload_if_t> patchbay;
  };

  cpuload_if_t::cpuload_if_t(algo_comm_t iac,const char*,const char*)
    : MHAPlugin::plugin_t<cpuload_cfg_t>("cpu load generator. CPU load is proportional to number of channels, number of frames, and factor",iac),
      factor("cpu load factor. Values > 1 increase cpu load, values < 1 decrease it","1","[0,]"),
      table_size("Size of the lookup table","65536","[1,]"),
      use_sine("Whether to use the sine function.","yes")
  {
    insert_member(factor);
    patchbay.connect(&factor.writeaccess,this,&cpuload_if_t::update);

    insert_member(table_size);
    patchbay.connect(&table_size.writeaccess,this,&cpuload_if_t::update);

    insert_member(use_sine);
    patchbay.connect(&use_sine.writeaccess,this,&cpuload_if_t::update);

  }

  void cpuload_if_t::update()
  {
    auto * cfg = new cpuload_cfg_t(factor.data, table_size.data, use_sine.data);
    push_config(cfg);
  }

  mha_spec_t* cpuload_if_t::process(mha_spec_t* s)
  {
    poll_config()->process(s->num_channels * s->num_frames);
    return s;
  }

  mha_wave_t* cpuload_if_t::process(mha_wave_t* s)
  {
    poll_config()->process(s->num_channels * s->num_frames);
    return s;
  }

  void cpuload_if_t::prepare(mhaconfig_t& cf)
  {
  }
}

MHAPLUGIN_CALLBACKS(cpuload,cpuload::cpuload_if_t,spec,spec)
MHAPLUGIN_PROC_CALLBACK(cpuload,cpuload::cpuload_if_t,wave,wave)
MHAPLUGIN_DOCUMENTATION\
(cpuload,
 "test-tool",
 "This plugin artificially generates cpu load. The achieved CPU load is proportional to number of channels, number of frames, and factor.\n"
 "If use\\_sine is set, a sine of is calculated, making the load mainly cpu-bound. Alternatively an operation on a\n"
 "variable size table is done, simulatung a memory bound problem.")

// Local Variables:
// compile-command: "make"
// c-basic-offset: 2
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
