// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2008 2009 2010 2013 2011 2014 2015 HörTech gGmbH
// Copyright © 2016 2017 2018 2019 2020 HörTech gGmbH
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

#ifndef DC_H
#define DC_H

#include <limits>
#include <algorithm>
#include "mha.hh"
#include "mha_algo_comm.hh"
#include "mha_filter.hh"
#include "mha_plugin.hh"
#include "mha_signal.hh"
#include "mha_tablelookup.hh"


namespace dc {

  class dc_vars_t{
  public:
    explicit dc_vars_t(MHAParser::parser_t&);
    MHAParser::mfloat_t gtdata;
    MHAParser::vfloat_t gtmin;
    MHAParser::vfloat_t gtstep;
    MHAParser::vfloat_t taurmslevel;
    MHAParser::vfloat_t tauattack;
    MHAParser::vfloat_t taudecay;
    MHAParser::vfloat_t offset;
    MHAParser::string_t filterbank;
    std::string cf_name, ef_name, bw_name;
    MHAParser::string_t chname;
    MHAParser::bool_t bypass;
    MHAParser::bool_t log_interp;
    MHAParser::string_t clientid;
    MHAParser::string_t gainrule;
    MHAParser::string_t preset;
    MHAParser::int_mon_t modified;
    MHAParser::vfloat_mon_t input_level;
    MHAParser::vfloat_mon_t filtered_level;
    MHAParser::vfloat_mon_t center_frequencies;
    MHAParser::vfloat_mon_t edge_frequencies;
    MHAParser::vfloat_mon_t band_weights;
  };

  class dc_vars_validator_t {
    public:
        dc_vars_validator_t(dc_vars_t & v,
                            unsigned int s,
                            mha_domain_t domain);
    };

  class dc_t : private dc_vars_validator_t {
    public:
        dc_t(dc_vars_t vars,
             mha_real_t filter_rate,
             unsigned int nch_,
             algo_comm_t ac,
             mha_domain_t domain,
             unsigned int fftlen,
             const std::string& algo,
             const std::vector<mha_real_t>& rmslevel_state={},
             const std::vector<mha_real_t>& attack_state={},
             const std::vector<mha_real_t>& decay_state={}
             );
        mha_wave_t* process(mha_wave_t*);
        mha_spec_t* process(mha_spec_t*);

        void explicit_insert();

        /** Number of frequency bands accessor. */
        unsigned get_nbands() const {return nbands;}
        /** Total number of channels accessor. */
        unsigned get_nch() const {return nch;}
        const MHASignal::waveform_t & get_level_in_db() const
        {return level_in_db;}
        const MHASignal::waveform_t & get_level_in_db_adjusted() const
        {return level_in_db_adjusted;}
        std::vector<mha_real_t> get_rmslevel_filter_state() const {
            return rmslevel.flatten();
        }
        std::vector<mha_real_t> get_attack_filter_state() const {
            return attack.flatten();
        }
        std::vector<mha_real_t> get_decay_filter_state() const {
            return decay.flatten();
        }

    private:
        std::vector<MHATableLookup::linear_table_t> gt;
        std::vector<mha_real_t> offset;
        MHAFilter::o1flt_lowpass_t rmslevel;
        MHAFilter::o1flt_lowpass_t attack;
        MHAFilter::o1flt_maxtrack_t decay;
        bool bypass;
        bool log_interp;
        unsigned int naudiochannels;
        unsigned int nbands;
        unsigned int nch;
        MHA_AC::waveform_t level_in_db;
        MHA_AC::waveform_t level_in_db_adjusted;
        unsigned int fftlen;
    };

  class dc_if_t : public MHAPlugin::plugin_t<dc_t>, public dc_vars_t {
    public:
        dc_if_t(const algo_comm_t& ac_,
                const std::string& th_,
                const std::string& al_);
        void prepare(mhaconfig_t& tf);
        mha_wave_t* process(mha_wave_t*);
        mha_spec_t* process(mha_spec_t*);
    private:
        /** Called from within the processing routines: updates the monitor
         * variables. */
        void update_monitors();

        /** Called by MHA configuration change event mechanism: creates new
         * runtime configuration */
        void update();

        std::string algo;
        MHAEvents::patchbay_t<dc_if_t> patchbay;
    };
}


#endif
