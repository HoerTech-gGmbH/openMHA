// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2014 2015 2017 2018 HörTech gGmbH
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

#ifndef ACPOOLING_WAVE_H
// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2015 2018 HörTech gGmbH
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

#define ACPOOLING_WAVE_H

#include "mha_plugin.hh"

class acPooling_wave;

//runtime config
class acPooling_wave_config {

public:
    acPooling_wave_config(algo_comm_t &ac, const mhaconfig_t in_cfg, acPooling_wave *_pooling);
    ~acPooling_wave_config();

    mha_wave_t* process(mha_wave_t*);
    void insert();

    //declare data necessary for processing state here
    algo_comm_t &ac;
    std::string raw_p_name;
    MHA_AC::waveform_t p;
    MHA_AC::waveform_t p_biased;
    MHA_AC::waveform_t p_max;
    MHA_AC::waveform_t like_ratio;
    mha_wave_t c;
    unsigned int pooling_ind;
    unsigned int pooling_option;
    unsigned int pooling_size;
    float up_thresh;
    float low_thresh;
    int neigh;
    float alpha;
    MHASignal::waveform_t pool;
    MHASignal::waveform_t prob_bias_func;

};

class acPooling_wave : public MHAPlugin::plugin_t<acPooling_wave_config> {

public:
    acPooling_wave(algo_comm_t & ac,const std::string & chain_name,
                   const std::string & algo_name);
    ~acPooling_wave();
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
    void release(void) {/* Do nothing in release */}

    //declare MHAParser variables here
    MHAParser::int_t numsamples;
    MHAParser::int_t pooling_wndlen;
    MHAParser::kw_t pooling_type;
    MHAParser::float_t upper_threshold;
    MHAParser::float_t lower_threshold;
    MHAParser::int_t neighbourhood;
    MHAParser::float_t alpha;
    MHAParser::string_t p_name;
    MHAParser::string_t p_biased_name;
    MHAParser::string_t pool_name;
    MHAParser::string_t max_pool_ind_name;
    MHAParser::string_t like_ratio_name;
    MHAParser::vfloat_t prob_bias;

private:
    void update_cfg();

    /* patch bay for connecting configuration parser
       events with local member functions: */
    MHAEvents::patchbay_t<acPooling_wave> patchbay;

};

#endif // ACPOOLING_WAVE_H

// Local Variables:
// c-basic-offset: 4
// indent-tabs-mode: nil
// compile-command: "make"
// coding: utf-8-unix
// End:
