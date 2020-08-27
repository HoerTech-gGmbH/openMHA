// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2009 2013 2016 2017 HörTech gGmbH
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

#ifndef MHA_FFTFB_HH
#define MHA_FFTFB_HH

#include "mha_parser.hh"
#include "mha_windowparser.h"
#include "mha_algo_comm.h"
#include "mha_signal.hh"
#include "mha_plugin.hh"
#include "mha_filter.hh"

/**
    \ingroup mhatoolbox
    \brief Namespace for overlapping FFT based filter bank classes and functions
 */ 

namespace MHAOvlFilter {

    // Filterbank band descriptor holding all relevant frequencies:
    class band_descriptor_t {
    public:
        // center frequency of lower neighbour band:
        mha_real_t cf_l;
        // lower edge frequency:
        mha_real_t ef_l;
        // center frequency:
        mha_real_t cf;
        // upper edge frequency:
        mha_real_t ef_h;
        // center frequency of upper neightbour band:
        mha_real_t cf_h;
        // flag to indicate if the band is asymmetric on the low side (flat response):
        bool low_side_flat;
        // flag to indicate if the band is asymmetric on the high side (flat response):
        bool high_side_flat;
    };

    typedef mha_real_t (scale_fun_t)(mha_real_t);

    class scale_var_t : public MHAParser::kw_t {
    public:
        scale_var_t(const std::string& help);
        void add_fun(const std::string& name, scale_fun_t* fun);
        std::string get_name() const { return data.get_value();};
        scale_fun_t* get_fun() const { return funs[data.get_index()];};
        mha_real_t hz2unit(mha_real_t x) const;
        mha_real_t unit2hz(mha_real_t x) const;
    private:
        std::vector<std::string> names;
        std::vector<scale_fun_t*> funs;
    };

    class fscale_t {
    public:
        fscale_t(MHAParser::parser_t& parent);
        std::vector<mha_real_t> get_f_hz() const;
        scale_var_t unit;
        MHAParser::vfloat_t f;
        MHAParser::vfloat_mon_t f_hz;
    private:
        void update_hz();
        MHAEvents::connector_t<fscale_t> updater;
    };

    class fscale_bw_t : public fscale_t {
    public:
        fscale_bw_t(MHAParser::parser_t& parent);
        std::vector<mha_real_t> get_bw_hz() const;
    protected:  
        MHAParser::vfloat_t bw;
        MHAParser::vfloat_mon_t bw_hz;
    private:
        void update_hz();
        MHAEvents::connector_t<fscale_bw_t> updater;
    };

    /**
       \brief Set of configuration variables for FFT-based overlapping filters

       This class enables easy configuration of the FFT-based overlapping
       filterbank.  An instance of fftfb_vars_t creates \mha configuration
       language variables needed for configuring the filterbank, and
       inserts these variables in the \mha configuration tree.

       This way, the variables are visible to the user and can be configured
       using the \mha configuration language.
    */
    class fftfb_vars_t {
      public:
        /** construct a set of \mha configuration language variables suitable
         * for configuring the FFT-based overlapping filterbank.
         * @param p The node of the configuration tree where the variables
         *          created by this instance are inserted. */
        fftfb_vars_t(MHAParser::parser_t & p);
        scale_var_t fscale;      //!< Frequency scale type (lin/bark/log/erb).
        scale_var_t ovltype;        //!< Filter shape (rect/lin/hann).
        MHAParser::float_t plateau;     //!< relative plateau width.
        MHAParser::kw_t ftype;  //!< Flag to decide wether edge or center frequencies are used.
        fscale_t f; //!< Frequency 
        MHAParser::bool_t normalize;    //!< Normalize sum of channels.
        MHAParser::bool_t fail_on_nonmonotonic; //!< Fail if frequency entries are non-monotonic (otherwise sort)
        MHAParser::bool_t fail_on_unique_bins; //!< Fail if center frequencies share the same FFT bin.
        MHAParser::bool_t flag_allow_empty_bands; //!< Allow that frequency bands contain only zeros.
        MHAParser::vfloat_mon_t cf; //!< Final center frequencies in Hz.
        MHAParser::vfloat_mon_t ef; //!< Final edge frequencies in Hz.
        MHAParser::vfloat_mon_t cLTASS; //!< Bandwidth correction for LTASS noise (level of 0 dB RMS LTASS noise)
        MHAParser::mfloat_mon_t shapes;
    };

    /**
       \brief Class for frequency spacing, used by filterbank shape
       generator class.

     */
    class fspacing_t {
    public:
        fspacing_t(const MHAOvlFilter::fftfb_vars_t& par, unsigned int nfft, mha_real_t fs);
        std::vector<unsigned int> get_cf_fftbin() const;
        std::vector<mha_real_t> get_cf_hz() const;
        std::vector<mha_real_t> get_ef_hz() const;
        /** \brief Return number of bands in filter bank.
         */
        unsigned int nbands() const {
            return bands.size();
        };
    protected:
        std::vector<MHAOvlFilter::band_descriptor_t> bands;
        mha_real_t (*symmetry_scale)(mha_real_t);
        void fail_on_nonmonotonic_cf();
        void fail_on_unique_fftbins();
    private:
        unsigned int nfft_; 
        mha_real_t fs_;
        void ef2bands(std::vector<mha_real_t> vef);
        void cf2bands(std::vector<mha_real_t> vcf);
        void equidist2bands(std::vector<mha_real_t> vcf);
    };

    /**
       \brief FFT based overlapping filter bank
     */
    class fftfb_t : public MHAOvlFilter::fspacing_t, private MHASignal::waveform_t {
      public:
        /** Constructor for a FFT-based overlapping filter bank.
         * @param par
         *   Parameters for the FFT filterbank that can not be deduced from
         *   the signal dimensions are taken from this set of configuration
         *   variables.
         * @param nfft FFT length
         * @param fs Sampling rate / Hz */
        fftfb_t(MHAOvlFilter::fftfb_vars_t& par, unsigned int nfft, mha_real_t fs);
         ~fftfb_t();
        void apply_gains(mha_spec_t * s_out, const mha_spec_t * s_in, const mha_wave_t * gains);
        void get_fbpower(mha_wave_t * fbpow, const mha_spec_t * s_in);
        void get_fbpower_db(mha_wave_t * fbpow, const mha_spec_t * s_in);
        std::vector<mha_real_t> get_ltass_gain_db() const;
        /** \brief Return index of first non-zero filter shape window 
         */
        unsigned int bin1(unsigned int band) const {
            return vbin1[band];
        };
        /** \brief Return index of first zero filter shape window above center frequency
         */
        unsigned int bin2(unsigned int band) const {
            return vbin2[band];
        };
        /** \brief Return fft length
         */
        unsigned int get_fftlen() const {
            return fftlen;
        };
        /**
           \brief Return filter shape window at index k in band b
           \param k Frequency index
           \param b Band index
         */
        mha_real_t w(unsigned int k, unsigned int b) const {
            return value(k, b);
        };
      private:
        unsigned int *vbin1;
        unsigned int *vbin2;
        mha_real_t (*shape)(mha_real_t);
        unsigned int fftlen;
        mha_real_t samplingrate;
    };

    /**
       \brief A time-domain minimal phase filter bank with frequency shapes from MHAOvlFilter::fftfb_t
     */
    class overlap_save_filterbank_t : public MHAOvlFilter::fftfb_t, public MHAFilter::fftfilterbank_t
    {
    public:
        class vars_t : public MHAOvlFilter::fftfb_vars_t
        {
        public:
            vars_t(MHAParser::parser_t & p);
            MHAParser::int_t fftlen;
            MHAParser::kw_t phasemodel;
            MHAParser::window_t irswnd;
        };
        overlap_save_filterbank_t(MHAOvlFilter::overlap_save_filterbank_t::vars_t& fbpar, mhaconfig_t channelconfig_in);
        mhaconfig_t get_channelconfig() const { return channelconfig_out_;};
    private:
        mhaconfig_t channelconfig_out_;
    };

    class overlap_save_filterbank_analytic_t : public MHAOvlFilter::overlap_save_filterbank_t {
    public:
        overlap_save_filterbank_analytic_t(MHAOvlFilter::overlap_save_filterbank_t::vars_t &fbpar, mhaconfig_t channelconfig_in);
        void filter_analytic(const mha_wave_t* sIn,mha_wave_t** fltRe,mha_wave_t** fltIm);
    private:
        MHAFilter::fftfilterbank_t imagfb;
    };

    class fftfb_ac_info_t 
    {
    public:
        fftfb_ac_info_t(const MHAOvlFilter::fftfb_t& fb,algo_comm_t ac,const std::string& prefix);
        void insert();
    private:
        /** vector of nominal center frequencies / Hz */
        MHA_AC::waveform_t cfv;
        /** vector of edge frequencies / Hz */
        MHA_AC::waveform_t efv;
        /** vector of band-weigths (sum of squared fft-bin-weigths)/num_frames */
        MHA_AC::waveform_t bwv;
        /** vector of LTASS correction */
        MHA_AC::waveform_t cLTASS;
    };

}

#endif

// Local Variables:
// compile-command: "make -C .."
// c-basic-offset: 4
// coding: utf-8-unix
// indent-tabs-mode: nil
// End:
