// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2013 2014 2017 2018 2019 2021 HörTech gGmbH
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

#ifndef SMOOTH_CEPSTRUM_HH
#define SMOOTH_CEPSTRUM_HH

#include "mha_plugin.hh"

namespace smooth_cepstrum {

    class smooth_params {
    public:
        smooth_params(const mhaconfig_t &_in_cfg,
                      float _xi_min_db,
                      float _f0_low, float _f0_high,
                      float _delta_pitch, float _lambda_thresh,
                      float _alpha_pitch, float _beta_const,
                      float _kappa_const, float _prior_q,
                      float _xi_opt_db, float _gain_min_db,
                      std::vector<float> &_winF0,
                      std::vector<float> &_alpha_const_vals,
                      std::vector<float> &_alpha_const_limits_hz,
                      std::string &_noisePow_name) :
            in_cfg(_in_cfg),
            xi_min_db(_xi_min_db),
            f0_low(_f0_low), f0_high(_f0_high),
            delta_pitch(_delta_pitch), lambda_thresh(_lambda_thresh),
            alpha_pitch(_alpha_pitch), beta_const(_beta_const),
            kappa_const(_kappa_const), prior_q(_prior_q),
            xi_opt_db(_xi_opt_db), gain_min_db(_gain_min_db),
            winF0( _winF0),
            alpha_const_vals( _alpha_const_vals ),
            alpha_const_limits_hz( _alpha_const_limits_hz ),
            noisePow_name(_noisePow_name)
        {
        }

        const mhaconfig_t in_cfg;
        float xi_min_db;
        float f0_low;
        float f0_high;
        float delta_pitch;
        float lambda_thresh;
        float alpha_pitch;
        float beta_const;
        float kappa_const;
        float prior_q;
        float xi_opt_db;
        float gain_min_db;

        std::vector<float> winF0;
        std::vector<float> alpha_const_vals;
        std::vector<float> alpha_const_limits_hz;
        std::string noisePow_name;
    };

    class smooth_cepstrum_t {

    public:
        smooth_cepstrum_t(algo_comm_t & ac, smooth_params & params);
        smooth_cepstrum_t(const smooth_cepstrum_t&)=delete;
        smooth_cepstrum_t& operator=(const smooth_cepstrum_t&)=delete;
        ~smooth_cepstrum_t();
        mha_spec_t* process(mha_spec_t*);

    private:

        algo_comm_t ac;
        smooth_params params;

        unsigned int fftlen;
        mha_fft_t mha_fft;

        unsigned int nfreq;
        unsigned int nchan;

        //constant to override scale of powspec within overlapadd
        float ola_powspec_scale;

        float q_low;
        float q_high;

        MHASignal::waveform_t winF0;

        float xi_min;
        float gain_min;

        MHASignal::waveform_t alpha_const;
        MHASignal::waveform_t alpha_prev;

        //mha_wave_t noisePow;
        MHASignal::waveform_t noisePow;
        MHASignal::waveform_t powSpec;

        MHASignal::waveform_t gamma_post;
        MHASignal::waveform_t xi_ml;
        MHASignal::spectrum_t lambda_ml_full;
        MHASignal::spectrum_t lambda_ml_ceps;
        MHASignal::waveform_t lambda_ml_smooth;

        MHASignal::waveform_t alpha_hat;
        MHASignal::waveform_t alpha_frame;

        MHASignal::spectrum_t lambda_ceps;
        MHASignal::waveform_t lambda_ceps_prev;

        MHASignal::spectrum_t log_lambda_spec;
        MHASignal::waveform_t lambda_spec;

        MHASignal::waveform_t xi_est;
        MHASignal::waveform_t gain_wiener;

        MHASignal::spectrum_t spec_out;

        double * max_val;
        int * max_q;
        int * pitch_set_first;
        int * pitch_set_last;

        //constants used for converting SNR to SPP
        float priorFact;
        float xiOpt;
        float logGLRFact;
        float GLRexp;

        MHASignal::waveform_t GLR;

    };

    class smooth_cepstrum_if_t : public MHAPlugin::plugin_t<smooth_cepstrum_t> {

    public:
        smooth_cepstrum_if_t(algo_comm_t iac, const std::string & configured_name);
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
        MHAEvents::patchbay_t<smooth_cepstrum_if_t> patchbay;

        bool prepared;

        void on_model_param_valuechanged();

    };
} //namespace smooth_cepstrum

#endif // SMOOTH_CEPSTRUM_HH

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
