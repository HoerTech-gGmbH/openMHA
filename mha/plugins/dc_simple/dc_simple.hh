// This file is part of the open HörTech Master Hearing Aid (openMHA)
// Copyright © 2007 2008 2009 2010 2013 2014 2015 2017 2018 2019 HörTech gGmbH
// Copyright © 2020 2021 HörTech gGmbH
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

#include <limits>
#include "mha_plugin.hh"
#include "mha_filter.hh"

namespace dc_simple {
    using namespace MHAPlugin;

    /** Checks size of vector.
     * @param [in] v The vector to check the size of.
     * @param [in] s Expected size of vector \c v.
     * @param [in] name Name of vector to include in error message when size does not match.
     * @throw MHA_Error if the size of \c v is neither \c s nor 1. */
    void test_fail(const std::vector<float>& v, unsigned int s, const std::string& name);

    /** Creates a copy of vector \c v with \c s elements, provided that \v has either s
     * elements or 1 elements.
     * @param [in] v The vector to copy elements from.
     * @param [in] s The desired number of elements in the output vector.
     * @param [in] name Name of vector to include in error message when input size does not match expectation.
     * @return A copy of \c v with \c s elements.
     * @throw MHA_Error if size of \c v is neither \c s nor 1. */
    std::vector<float> force_resize(const std::vector<float>& v, unsigned int s, const std::string& name);

    /** Helper function to throw an error if \c x is 0. 
     * @param [in] x The value to check.
     * @param [in] comment Optional explanation for error message.
     * @throw MHA_Error if \c x == 0.*/
    mha_real_t not_zero(mha_real_t x,const std::string& comment);

    /// class for dc_simple plugin which registers variables to MHAParser.
    class dc_vars_t {
    public:
        dc_vars_t(MHAParser::parser_t& p);
        MHAParser::vfloat_t g50;
        MHAParser::vfloat_t g80;
        MHAParser::vfloat_t maxgain;
        MHAParser::vfloat_t expansion_threshold;
        MHAParser::vfloat_t expansion_slope;
        MHAParser::vfloat_t limiter_threshold;
        MHAParser::vfloat_t tauattack;
        MHAParser::vfloat_t taudecay;
        MHAParser::bool_t bypass;
    };
    /// Helper class to check sizes of configuration variable vectors.
    class dc_vars_validator_t {
    public:
        /** Checks that all vectors in \c v have size \c s or size 1.
         * @param v Aggregation of vectors to check the sizes of.
         * @param s Desired vector size for all vectors
         * @throw MHA_Error if \c s == 0.
         * @throw MHA_Error if the size of any vector in \c v is neither \c s nor 1. */
        dc_vars_validator_t(const dc_vars_t& v,unsigned int s);
    };

    // Not clear about level_smoother class and dc_t. They have different constructors, 
    // where dc_t has an additional parameter level_db in constructor but I can't figure out what this does
    class level_smoother_t : private dc_vars_validator_t {
    public:
        level_smoother_t(const dc_vars_t& vars,
                                mha_real_t filter_rate,
                                mhaconfig_t buscfg);
        mha_wave_t* process(mha_spec_t* s);
        mha_wave_t* process(mha_wave_t* s);
    private:
        MHAFilter::o1flt_lowpass_t attack;
        MHAFilter::o1flt_maxtrack_t decay;
        unsigned int nbands;
        unsigned int fftlen;
        MHASignal::waveform_t level_wave;
        MHASignal::waveform_t level_spec;
    };

    /// Runtime config class for dc_simple plugin.
    class dc_t : private dc_vars_validator_t {
    public:
        dc_t(const dc_vars_t& vars, mha_real_t filter_ratem, unsigned int nch, unsigned int fftlen_);
        mha_spec_t* process(mha_spec_t* s, mha_wave_t* level_db);
        mha_wave_t* process(mha_wave_t* s, mha_wave_t* level_db);
    private:
        class line_t {
        public:
            line_t(mha_real_t x1,mha_real_t y1,mha_real_t x2,mha_real_t y2);
            line_t(mha_real_t x1,mha_real_t y1,mha_real_t m_);
            inline mha_real_t operator()(mha_real_t x){return m*x+y0;};
        private:
            mha_real_t m, y0;
        };
        std::vector<mha_real_t> expansion_threshold;
        std::vector<mha_real_t> limiter_threshold;
        std::vector<line_t> compression;
        std::vector<line_t> expansion;
        std::vector<line_t> limiter;
        std::vector<mha_real_t> maxgain;
        unsigned int nbands;
    public:
        std::vector<float> mon_l, mon_g;
    };

    typedef MHAPlugin::plugin_t<dc_t> DC;
    typedef MHAPlugin::config_t<level_smoother_t> LEVEL;
    
    /// interface class
    class dc_if_t : public DC, public LEVEL, public dc_vars_t {
    public:
        dc_if_t(algo_comm_t iac, const std::string & configured_name);
        void prepare(mhaconfig_t&);
        void release();
        mha_spec_t* process(mha_spec_t* s);
        mha_wave_t* process(mha_wave_t* s);
    private:
        void update_dc();
        void update_level();
        void has_been_modified(){modified.data = 1;};
        void read_modified(){modified.data = 0;};
        void update_level_mon();
        void update_gain_mon();
        MHAParser::string_t clientid;
        MHAParser::string_t gainrule;
        MHAParser::string_t preset;
        MHAParser::int_mon_t modified;
        MHAParser::vfloat_mon_t mon_l, mon_g;
        MHAParser::string_t filterbank;
        MHAParser::vfloat_mon_t center_frequencies;
        MHAParser::vfloat_mon_t edge_frequencies;
        MHAEvents::patchbay_t<dc_if_t> patchbay;
        bool prepared;
    };

}
