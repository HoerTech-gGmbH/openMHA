// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2014 2017 HörTech gGmbH
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

#ifndef LPC_H
#define LPC_H

#include "mha_plugin.hh"
#include <vector>

//runtime config
class lpc_config {

public:
    lpc_config(algo_comm_t &ac, const mhaconfig_t in_cfg, std::string &algo_name, unsigned int _order, unsigned int _lpc_buffer_size, bool _shift, unsigned int _comp_each_iter, bool _norm);
    ~lpc_config();

    mha_wave_t* process(mha_wave_t*);
    void insert();

private:
    //declare data necessary for processing state here
    bool norm;
    bool shift;
    unsigned int comp_each_iter;
    unsigned int order;
    unsigned int lpc_buffer_size;
    unsigned int N;
    unsigned int comp_iter;
    mha_wave_t sample;
    std::vector<mha_real_t> R;
    std::vector<mha_real_t> A;
    MHASignal::ringbuffer_t inwave;
    MHA_AC::waveform_t lpc_out;
    MHA_AC::waveform_t corr_out;

};

class lpc : public MHAPlugin::plugin_t<lpc_config> {

public:
    lpc(algo_comm_t & ac,const std::string & chain_name,
        const std::string & algo_name);
    ~lpc();
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
    void release(void) {/* Do nothing in release */}

private:
    void update_cfg();

    std::string algo_name; //store name to give to config

    //declare MHAParser variables here
    MHAParser::int_t lpc_order;
    MHAParser::int_t lpc_buffer_size;
    MHAParser::bool_t shift;
    MHAParser::int_t comp_each_iter;
    MHAParser::bool_t norm;


    /* patch bay for connecting configuration parser
       events with local member functions: */
    MHAEvents::patchbay_t<lpc> patchbay;

};

#endif // LPC_H

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
