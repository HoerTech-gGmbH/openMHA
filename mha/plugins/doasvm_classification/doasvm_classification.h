// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2014 2015 2018 HörTech gGmbH
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

#ifndef DOASVM_CLASSIFICATION_H
#define DOASVM_CLASSIFICATION_H

#include "mha_plugin.hh"

class doasvm_classification;

//runtime config
class doasvm_classification_config {

public:
    doasvm_classification_config(algo_comm_t &ac, const mhaconfig_t in_cfg, doasvm_classification *_doasvm);
    ~doasvm_classification_config();

    mha_wave_t* process(mha_wave_t*);

    //declare data necessary for processing state here
    algo_comm_t &ac;
    doasvm_classification *doasvm;
    MHA_AC::waveform_t p;
    MHA_AC::int_t p_max;
    mha_wave_t c;

};

class doasvm_classification : public MHAPlugin::plugin_t<doasvm_classification_config> {

public:
    doasvm_classification(algo_comm_t & ac,const std::string & chain_name,
                          const std::string & algo_name);
    ~doasvm_classification();
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
    void release(void) {/* Do nothing in release */}

    //declare MHAParser variables here
    MHAParser::vfloat_t angles;
    MHAParser::mfloat_t w;
    MHAParser::vfloat_t b;
    MHAParser::vfloat_t x;
    MHAParser::vfloat_t y;
    MHAParser::string_t p_name;
    MHAParser::string_t max_p_ind_name;
    MHAParser::string_t vGCC_name;

private:
    void update_cfg();

    /* patch bay for connecting configuration parser
       events with local member functions: */
    MHAEvents::patchbay_t<doasvm_classification> patchbay;

};

#endif // DOASVM_CLASSIFICATION_H

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
