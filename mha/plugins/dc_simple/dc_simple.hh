// This file is part of the open HörTech Master Hearing Aid (openMHA)
// Copyright © 2007 2008 2009 2010 2013 2014 2015 2017 2018 2019 HörTech gGmbH
// Copyright © 2020 2021 HörTech gGmbH
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

    /** Class which computes smoothed input levels on individual bands, using an
     * attack and release filter, which are a first order low pass filter and a
     * maximum tracker filter, respectively */
    class level_smoother_t : private dc_vars_validator_t {
    public:
        level_smoother_t(const dc_vars_t& vars,
                                mha_real_t filter_rate,
                                mhaconfig_t buscfg);
        /** Process callback. Computes smoothed levels from the input mha type spectrum 
         * by applying a lowpass and maximum tracker filter
         * @return smoothed input levels in dB SPL
         * @param s input signal */
        mha_wave_t* process(mha_spec_t* s);
        /** Process callback. Computes smoothed levels from the input mha type waveform 
         * over individual bands by applying a lowpass and maximum tracker filter
         * @return smoothed input levels in dB SPL
         * @param s input/output signal */
        mha_wave_t* process(mha_wave_t* s);
    private:
        /** first order low pass attack filter */
        MHAFilter::o1flt_lowpass_t attack; 
        /** maximum tracker decay filter */
        MHAFilter::o1flt_maxtrack_t decay;
        /** Total number of frequency bands of this compressor */
        unsigned int nbands;
        unsigned int fftlen;
        MHASignal::waveform_t level_wave;
        MHASignal::waveform_t level_spec;
    };

    /// Runtime config class for dc_simple plugin.
    class dc_t : private dc_vars_validator_t {
    public:
        /** Constructor */
        dc_t(const dc_vars_t& vars, unsigned int nch);

        /** Process callback. Compresses, expands or limits depending on the gain settings and 
         * filtered signal levels.
         * Compresses the spectrum input signal in individual bands.
         * @param s input/output signal 
         * @param level_db smoothed levels of input signal in dB SPL 
         * @return s. The input signal is modified in place.*/
        mha_spec_t* process(mha_spec_t* s, mha_wave_t* level_db);

        /** Process callback. Compresses, expands or limits depending on the gain settings and 
         * filtered signal levels.
         * Compresses the waveform input signal in individual bands.
         * @param s input/output signal
         * @param level_db smoothed levels of input signal in dB SPL 
         * @return s. The input signal is modified in place.*/
        mha_wave_t* process(mha_wave_t* s, mha_wave_t* level_db);

    private:
        /** Helper class for usage in computing compression, expansion and limiting. */
        class line_t {
        public:
            /** Constructor used for compression which takes two x and y coordinates each to find m and y0  */
            line_t(mha_real_t x1,mha_real_t y1,mha_real_t x2,mha_real_t y2);

            /** Constructor used for expansion and limiting which takes x and y coordinates and a gradient, giving y0*/
            line_t(mha_real_t x1,mha_real_t y1,mha_real_t m_);

            /** Operator overload which returns 
             * @param x
             * @return y values mapped to x by a linear equation with gradient m and intercept y0 */
            inline mha_real_t operator()(mha_real_t x){return m*x+y0;};

        private:
            /** The gradient and y-intercept */
            mha_real_t m, y0; 
        };
        std::vector<mha_real_t> expansion_threshold;    //!< Threshold below which to apply expansion
        std::vector<mha_real_t> limiter_threshold;      //!< Threshold below which to compress
        std::vector<line_t> compression;                //!< The linear function for applying compression
        std::vector<line_t> expansion;                  //!< The linear function for applying expansion
        std::vector<line_t> limiter;                    //!< The linear function for applying limiting
        std::vector<mha_real_t> maxgain;                //!< Gain should not exceed this value
        unsigned int nbands;                            //!< Number of bands
        
    public:
        // monitor level and monitor gain
        std::vector<float> mon_l, mon_g;
    };

    /** Define alternate name for runtime_cfg_t */
    typedef MHAPlugin::plugin_t<dc_t> DC;

    /** Define alternate name for config_t */
    typedef MHAPlugin::config_t<level_smoother_t> LEVEL;
    
    /// interface class for dc_simple
    class dc_if_t : public DC, public LEVEL, public dc_vars_t {
    public:
        /** Constructor instantiates one dc_simple plugin */
        dc_if_t(MHA_AC::algo_comm_t & iac, 
                const std::string & configured_name);

        /** Prepare dc_simple plugin for signal processing
         *   @param tf signal_dimensions */
        void prepare(mhaconfig_t& tf);

        /** Sets prepared back to False */
        void release();

        /** Main process callback. Takes mhatype spectrum input and 
         * calls type DC and LEVEL process methods, returning mhatype spectrum.
         * @param s input/output signal */
        mha_spec_t* process(mha_spec_t* s);

        /** Main process callback. Takes mhatype wave input and 
         * calls type DC and LEVEL process methods, returning mhatype wave.
         * @param s input/output signal */
        mha_wave_t* process(mha_wave_t* s);
    private:
        /** Update dc_t runtime config when configuration parameters have changed */
        void update_dc();

        /** Update level_smoother_t runtime config when configuration parameters have changed */
        void update_level();
        void has_been_modified(){modified.data = 1;};
        void read_modified(){modified.data = 0;};

        /** Updates the data of variable mon_l in dc_t */
        void update_level_mon();

        /** Updates the data of variable mon_g in dc_t */
        void update_gain_mon();

        /** MHA Parser variables */
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
// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
