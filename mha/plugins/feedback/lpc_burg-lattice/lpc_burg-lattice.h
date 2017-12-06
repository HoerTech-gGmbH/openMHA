// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2016 2017 HörTech gGmbH
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

#ifndef LPC_BURGLATTICE_H
#define LPC_BURGLATTICE_H

#include "mha_plugin.hh"

#define EPSILON 1e-10

class lpc_burglattice;

//runtime config
class lpc_burglattice_config {

    algo_comm_t ac;

    MHASignal::waveform_t forward;
    MHASignal::waveform_t backward;

    MHASignal::waveform_t kappa;
    MHA_AC::waveform_t kappa_block;
    MHASignal::waveform_t dm;
    MHASignal::waveform_t nm;
    mha_real_t lambda;
    int lpc_order;
    std::string name_f;
    std::string name_b;

    mha_wave_t s_f;
    mha_wave_t s_b;

public:
    lpc_burglattice_config(algo_comm_t &iac, const mhaconfig_t in_cfg, lpc_burglattice * _lpc);
    ~lpc_burglattice_config();

    mha_wave_t* process(mha_wave_t*);

    //declare data necessary for processing state here

};

class lpc_burglattice : public MHAPlugin::plugin_t<lpc_burglattice_config> {

public:
    lpc_burglattice(algo_comm_t & ac,const std::string & chain_name,
                     const std::string & algo_name);
    ~lpc_burglattice();
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
    void release(void) {/* Do nothing in release */}

    //declare MHAParser variables here
    MHAParser::int_t lpc_order;
    MHAParser::string_t name_kappa;
    MHAParser::string_t name_f;
    MHAParser::string_t name_b;
    MHAParser::float_t lambda;

private:
    void update_cfg();


    /* patch bay for connecting configuration parser
       events with local member functions: */
    MHAEvents::patchbay_t<lpc_burglattice> patchbay;

};

#endif // LPC_BURGLATTICE_H
