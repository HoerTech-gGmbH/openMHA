// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2016 HörTech gGmbH
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

#ifndef LPC_BL_PREDICTOR_H
#define LPC_BL_PREDICTOR_H

#include "mha_plugin.hh"

#define EPSILON 1e-10

class lpc_bl_predictor;

//runtime config
class lpc_bl_predictor_config {

    algo_comm_t ac;

    MHA_AC::waveform_t f_est;
    MHA_AC::waveform_t b_est;

    MHASignal::waveform_t forward;
    MHASignal::waveform_t backward;

    int lpc_order;
    std::string name_km;
    std::string name_f;
    std::string name_b;

    mha_wave_t km;
    mha_wave_t s_f;
    mha_wave_t s_b;

public:
    lpc_bl_predictor_config(algo_comm_t &iac, const mhaconfig_t in_cfg, lpc_bl_predictor *_lpc);
    ~lpc_bl_predictor_config();

    mha_wave_t* process(mha_wave_t*);

    //declare data necessary for processing state here

};

class lpc_bl_predictor : public MHAPlugin::plugin_t<lpc_bl_predictor_config> {

public:
    lpc_bl_predictor(algo_comm_t & ac,const std::string & chain_name,
                     const std::string & algo_name);
    ~lpc_bl_predictor();
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
    void release(void) {/* Do nothing in release */}

    //declare MHAParser variables here
    MHAParser::int_t lpc_order;
    MHAParser::string_t name_kappa;
    MHAParser::string_t name_lpc_f;
    MHAParser::string_t name_lpc_b;
    MHAParser::string_t name_f;
    MHAParser::string_t name_b;

private:
    void update_cfg();

    /* patch bay for connecting configuration parser
       events with local member functions: */
    MHAEvents::patchbay_t<lpc_bl_predictor> patchbay;

};

#endif // LPC_BL_PREDICTOR_H
