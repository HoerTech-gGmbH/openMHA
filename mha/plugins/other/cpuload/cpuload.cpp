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

#include <math.h>
#include "mha_plugin.hh"

class cpuload_t : public MHAPlugin::plugin_t<float> {
public:
  cpuload_t(algo_comm_t , const char*, const char*);
  mha_spec_t* process(mha_spec_t*);
  mha_wave_t* process(mha_wave_t*);
  void prepare(mhaconfig_t&);
private:
  MHAParser::float_t factor;
  MHAParser::bool_t use_sine;
  float phase;
  volatile float result;
  std::vector<float> table;
  inline void compute_something() {
    result = sinf(phase++);
    if (phase > M_PI) phase -= 2*M_PI;
  }
  inline void compute_something_else() {
    double index = (phase + M_PI) * 10429;
    unsigned index_i = static_cast<unsigned>(index);
    double frag_d = index - index_i;
    result = table[index_i+1] * frag_d + table[index_i] * (1-frag_d);
    if (++phase > M_PI) phase -= 2*M_PI;
  }
};

cpuload_t::cpuload_t(algo_comm_t iac,const char*,const char*)
  : MHAPlugin::plugin_t<float>("cpu load generator. CPU load is proportional to number of channels, number of frames, and factor",iac),
    factor("cpu load factor. Values > 1 increase cpu load, values < 1 decrease it","1","[0,]"),
    use_sine("Whether to use the sine function. If not, table interpolation will be used","yes"),
    phase(0),
    result(0),
    table(65536, 0.0f)
{
  insert_member(factor);
  insert_member(use_sine);
  for (unsigned i = 0; i < table.size(); ++i) {
    table[i] = rand();
  }
}

mha_spec_t* cpuload_t::process(mha_spec_t* s)
{
  for (unsigned n = 0;
       n < (s->num_channels * s->num_frames * 20 * factor.data);
       ++n) {
    use_sine.data ? compute_something() : compute_something_else();
  }
  return s;
}

mha_wave_t* cpuload_t::process(mha_wave_t* s)
{
  for (unsigned n = 0;
       n < (s->num_channels * s->num_frames * 10 * factor.data);
       ++n) {
    use_sine.data ? compute_something() : compute_something_else();
  }
  return s;
}

void cpuload_t::prepare(mhaconfig_t& cf)
{
}

MHAPLUGIN_CALLBACKS(cpuload,cpuload_t,spec,spec)
MHAPLUGIN_PROC_CALLBACK(cpuload,cpuload_t,wave,wave)

// Local Variables:
// compile-command: "make"
// c-basic-offset: 2
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
