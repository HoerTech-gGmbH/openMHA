// This file is part of the HörTech Master Hearing Aid (MHA)
// Copyright © 2015 2018 2021 HörTech gGmbH
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

#ifndef ACCONCAT_WAVE_H
#define ACCONCAT_WAVE_H

#include <stdio.h>

#include <vector>
#include <string>

#include "mha_plugin.hh"

using namespace std;

class acConcat_wave;

//runtime config
class acConcat_wave_config {

public:
    acConcat_wave_config(algo_comm_t &ac, acConcat_wave *_concat);
    ~acConcat_wave_config();

    mha_wave_t* process(mha_wave_t*);

    //declare data necessary for processing state here
    algo_comm_t &ac;
    std::vector<std::string> strNames_AC;
    std::vector<int> numSamples_AC;
    mha_wave_t vGCC;
    MHA_AC::waveform_t *vGCC_con;

};

class acConcat_wave : public MHAPlugin::plugin_t<acConcat_wave_config> {

public:
    acConcat_wave(algo_comm_t iac, const std::string & configured_name);
    ~acConcat_wave();
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
    void release(void) {/* Do nothing in release */}

    //declare MHAParser variables here
    MHAParser::int_t num_AC;
    MHAParser::string_t prefix_names_AC;
    MHAParser::vint_t samples_AC;
    MHAParser::string_t name_con_AC;
    MHAParser::int_t numchannels;

private:
    void update_cfg();

    /* patch bay for connecting configuration parser
       events with local member functions: */
    MHAEvents::patchbay_t<acConcat_wave> patchbay;

};

#endif // ACCONCAT_WAVE_H

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
