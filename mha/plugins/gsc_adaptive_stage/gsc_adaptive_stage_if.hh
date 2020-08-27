// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2013 2014 2018 2020 HörTech gGmbH
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

#ifndef SHYNKCONFIG_H
#define SHYNKCONFIG_H

#include "mha_plugin.hh"
#include "gsc_adaptive_stage.hh"
namespace gsc_adaptive_stage {
/** Plugin interface class */
class gsc_adaptive_stage_if : public MHAPlugin::plugin_t<gsc_adaptive_stage> {

public:
  gsc_adaptive_stage_if(algo_comm_t & ac,const std::string &,
                       const std::string &);
  ~gsc_adaptive_stage_if()=default;
  mha_wave_t* process(mha_wave_t*);
  void prepare(mhaconfig_t&);
  void release(void) {/* Do nothing in release */}

private:
  void update_cfg();

  /* patch bay for connecting configuration parser
     events with local member functions: */
  MHAEvents::patchbay_t<gsc_adaptive_stage_if> patchbay;

  /** How many old samples should be buffered per filter block */
  MHAParser::int_t lenOldSamps;
  /** Whether to compensate for circular convolution */
  MHAParser::bool_t doCircularComp;
  /** Linear coefficient for gradient used in filter adaption */
  MHAParser::float_t mu;
  /** Autoregressive coefficient for PSD estimation */
  MHAParser::float_t alp;
  /** Wether to use VAD for conditional filter adaption */
  MHAParser::bool_t useVAD;
  /** Name of VAD AC variable. Ignored if useVAD=no */
  MHAParser::string_t vadName;

  void on_model_param_valuechanged();

};
} // gsc_adaptive_stage
#endif // SHYNKCONFIG_H
