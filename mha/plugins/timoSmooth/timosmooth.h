// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2013 2014 2017 2018 HörTech gGmbH
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

#ifndef TIMOSMOOTH_H
#define TIMOSMOOTH_H

#include "mha_plugin.hh"

class timoSmooth : public MHAPlugin::plugin_t<timoConfig> {

public:
    timoSmooth(algo_comm_t & ac,const std::string & chain_name,
            const std::string & algo_name);
    ~timoSmooth();
    mha_spec_t* process(mha_spec_t*);
    void prepare(mhaconfig_t&);
    void release(void) {/* Do nothing in release */}

private:
    void update_cfg();

    MHAParser::float_t xi_min_db;
    MHAParser::float_t f0_low;
    MHAParser::float_t f0_high;
    MHAParser::float_t delta_pitch;
    MHAParser::float_t lambda_thresh;
    MHAParser::float_t alpha_pitch;
    MHAParser::float_t beta_const;
    MHAParser::float_t kappa_const;
    MHAParser::float_t gain_min_db;

    MHAParser::vfloat_t win_f0;
    MHAParser::vfloat_t alpha_const_vals;
    MHAParser::vfloat_t alpha_const_limits_hz;

    MHAParser::string_t noisePow_name;

    //here is a subparser
    MHAParser::parser_t spp;
    MHAParser::float_t prior_q;
    MHAParser::float_t xi_opt_db;

    /* patch bay for connecting configuration parser
       events with local member functions: */
    MHAEvents::patchbay_t<timoSmooth> patchbay;

    bool prepared;

    void on_model_param_valuechanged();

};

#endif // TIMOSMOOTH_H

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
