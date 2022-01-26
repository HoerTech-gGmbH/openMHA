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

#ifndef STEERBF_H
#define STEERBF_H

#include "mha_plugin.hh"

class steerbf;

class parser_int_dyn : public MHAParser::int_t {
public:
    parser_int_dyn(const std::string &help_text, const std::string &initial_value, const std::string &range) :
        MHAParser::int_t(help_text,initial_value,range) {}

    void set_max_angle_ind( unsigned int max_ind ) {
        up_limit = max_ind;
        up_incl = true;
    }
};

class steerbf_config {

public:
    steerbf_config(MHA_AC::algo_comm_t & ac,
                   const mhaconfig_t in_cfg,
                   steerbf *steerbf);
    ~steerbf_config();
    mha_spec_t* process(mha_spec_t*);

private:
    unsigned int nchan;
    unsigned int nfreq;
    MHASignal::spectrum_t outSpec;
    mha_spec_t bf_vec;
    unsigned int nangle;
    steerbf *_steerbf;
    MHA_AC::algo_comm_t & ac;
    std::string bf_src_copy;
};

//this plugin does its own real-time processing
class steerbf : public MHAPlugin::plugin_t<steerbf_config> {

public:
    steerbf(MHA_AC::algo_comm_t & iac, const std::string & configured_name);
    ~steerbf();
    mha_spec_t* process(mha_spec_t*);
    void prepare(mhaconfig_t&);
    void release(void) {/* Do nothing in release */}

    //declare MHAParser variables here
    MHAParser::string_t bf_src;
    parser_int_dyn angle_ind;
    MHAParser::string_t angle_src;

private:
    void update_cfg();

    /* patch bay for connecting configuration parser
       events with local member functions: */
    MHAEvents::patchbay_t<steerbf> patchbay;

};

#endif // STEERBF_H

/*
 * Local Variables:
 * compile-command: "make"
 * indent-tabs-mode: nil
 * c-basic-offset: 4
 * coding: utf-8-unix
 * End:
 */
