// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2015 2018 2021 HörTech gGmbH
// Copyright © 2022 Hörzentrum Oldenburg gGmbH
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

#ifndef ACTRANSFORM_WAVE_H
#define ACTRANSFORM_WAVE_H

#include "mha_plugin.hh"

class acTransform_wave;

//runtime config
class acTransform_wave_config {

public:
    acTransform_wave_config(MHA_AC::algo_comm_t & ac,
                            acTransform_wave *_transform);
    ~acTransform_wave_config();

    mha_wave_t* process(mha_wave_t*);

    /// Insert or reinsert AC variables rotated_p, rotated_i
    void insert_ac_variables();

    //declare data necessary for processing state here
    MHA_AC::algo_comm_t & ac;
    std::string ang_name;
    std::string raw_p_name;
    std::string raw_p_max_name;
    MHA_AC::waveform_t rotated_p;
    MHA_AC::int_t rotated_i;

    unsigned int offset;
    unsigned int resolution;
    unsigned int to_from;

};

class acTransform_wave : public MHAPlugin::plugin_t<acTransform_wave_config> {

public:
    acTransform_wave(MHA_AC::algo_comm_t & iac,
                     const std::string & configured_name);
    ~acTransform_wave();
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
    void release(void) {/* Do nothing in release */}

    //declare MHAParser variables here
    MHAParser::string_t ang_name;
    MHAParser::string_t raw_p_name;
    MHAParser::string_t raw_p_max_name;
    MHAParser::string_t rotated_p_name;
    MHAParser::string_t rotated_p_max_name;
    MHAParser::int_t numsamples;
    MHAParser::bool_t to_from;

private:
    void update_cfg();

    /* patch bay for connecting configuration parser
       events with local member functions: */
    MHAEvents::patchbay_t<acTransform_wave> patchbay;

};

#endif // ACTRANSFORM_WAVE_H

// Local Variables:
// c-basic-offset: 4
// indent-tabs-mode: nil
// compile-command: "make"
// coding: utf-8-unix
// End:
