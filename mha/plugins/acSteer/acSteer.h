// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2014 2017 2018 2021 HörTech gGmbH
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

#ifndef ACSTEER_H
#define ACSTEER_H

#include "mha_plugin.hh"

#include <iostream>
#include <fstream>

using namespace std;

class acSteer;

//runtime config
class acSteer_config {

public:
    acSteer_config(MHA_AC::algo_comm_t & ac,
                   const mhaconfig_t in_cfg,
                   acSteer *acSteer);
    ~acSteer_config();
    void insert();

    //declare data necessary for processing state here
    unsigned int nchan;
    unsigned int nfreq;
    unsigned int nsteerchan;
    unsigned int nrefmic;
    unsigned int nangle;

    MHA_AC::spectrum_t specSteer1;
    MHA_AC::spectrum_t specSteer2;
};

class acSteer : public MHAPlugin::plugin_t<acSteer_config> {

public:
    acSteer(MHA_AC::algo_comm_t & iac, const std::string & configured_name);
    ~acSteer();
    mha_spec_t* process(mha_spec_t*);
    void prepare(mhaconfig_t&);
    void release(void) {/* Do nothing in release */}

    //declare MHAParser variables here
    MHAParser::string_t steerFile;
    MHAParser::string_t acSteerName1;
    MHAParser::string_t acSteerName2;
    MHAParser::int_t nsteerchan;
    MHAParser::int_t nrefmic;

private:
    void update_cfg();

    /* patch bay for connecting configuration parser
       events with local member functions: */
    MHAEvents::patchbay_t<acSteer> patchbay;

};

#endif // ACSTEER_H

/*
 * Local Variables:
 * compile-command: "make"
 * indent-tabs-mode: nil
 * c-basic-offset: 4
 * coding: utf-8-unix
 * End:
 */
