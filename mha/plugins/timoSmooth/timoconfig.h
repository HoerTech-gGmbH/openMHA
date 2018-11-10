// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2014 2017 HörTech gGmbH
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

#ifndef TIMOCONFIG_H
#define TIMOCONFIG_H

#include "mha_plugin.hh"
#include <vector>

//AC variables for debugging
class timo_AC {
public:

    timo_AC(algo_comm_t &ac, unsigned int fftlen,
            unsigned int nfreq, unsigned int nchan) :
        gamma_post_AC(ac, "gamma_post_AC", nfreq, nchan, true),
        xi_ml_AC(ac, "xi_ml_AC", nfreq, nchan, true),
        lambda_ml_AC(ac, "lambda_ml_AC", fftlen, nchan, true),
        lambda_ml_ceps_AC(ac, "lambda_ml_ceps_AC", fftlen, nchan, true),
        lambda_ml_smooth_AC(ac, "lambda_ml_smooth_AC", nfreq, nchan, true),
        max_q_AC(ac, "max_q_AC", 1, nchan, true),
        max_val_AC(ac, "max_val_AC", 1, nchan, true),
        pitch_set_first_AC(ac, "pitch_set_first_AC", 1, nchan, true),
        pitch_set_last_AC(ac, "pitch_set_last_AC", 1, nchan, true),
        alpha_hat_AC(ac, "alpha_hat_AC", nfreq, nchan, true),
        alpha_frame_AC(ac, "alpha_frame_AC", nfreq, nchan, true),
        lambda_ceps_AC(ac, "lambda_ceps_AC", fftlen, nchan, true),
        log_lambda_spec_AC(ac, "log_lambda_spec_AC", fftlen, nchan, true),
        lambda_spec_AC(ac, "lambda_spec_AC", nfreq, nchan, true),
        xi_est_AC(ac, "xi_est_AC", nfreq, nchan, true),
        gain_wiener_AC(ac, "gain_wiener_AC", nfreq, nchan, true),
        winF0_AC(ac, "winF0_AC", 8, 1, true),
        SPP(ac, "SPP", nfreq, nchan, true) {}

    void copy();
    void insert();

    MHA_AC::waveform_t gamma_post_AC;
    MHA_AC::waveform_t xi_ml_AC;
    MHA_AC::spectrum_t lambda_ml_AC;
    MHA_AC::spectrum_t lambda_ml_ceps_AC;
    MHA_AC::waveform_t lambda_ml_smooth_AC;

    MHA_AC::waveform_t max_q_AC;
    MHA_AC::waveform_t max_val_AC;
    MHA_AC::waveform_t pitch_set_first_AC;
    MHA_AC::waveform_t pitch_set_last_AC;

    MHA_AC::waveform_t alpha_hat_AC;
    MHA_AC::waveform_t alpha_frame_AC;
    MHA_AC::spectrum_t lambda_ceps_AC;
    MHA_AC::spectrum_t log_lambda_spec_AC;
    MHA_AC::waveform_t lambda_spec_AC;
    MHA_AC::waveform_t xi_est_AC;
    MHA_AC::waveform_t gain_wiener_AC;

    MHA_AC::waveform_t winF0_AC;
    MHA_AC::waveform_t SPP;

};

class timo_params {
public:
    timo_params(const mhaconfig_t &_in_cfg,
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

class timoConfig {

public:
    timoConfig(algo_comm_t & ac, timo_params & params);
    ~timoConfig();
    mha_spec_t* process(mha_spec_t*);

private:

    void copy_AC(timo_AC & tAC);

    algo_comm_t ac;
    timo_params params;

    unsigned int fftlen;
    mha_fft_t mha_fft;

    unsigned int nfreq;
    unsigned int nchan;

    timo_AC tAC;

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

#endif // TIMOCONFIG_H

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
