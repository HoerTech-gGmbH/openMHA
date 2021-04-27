// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2013 2014 2015 2017 2018 2019 2021 HörTech gGmbH
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

#include "smooth_cepstrum.hh"
#include "mha_filter.hh"
#include <algorithm>
#include <vector>

namespace{

#define INSERT_VAR(var) insert_item(#var, &var)
#define PATCH_VAR(var) patchbay.connect(&var.valuechanged, this, \
                                        &smooth_cepstrum::smooth_cepstrum_if_t::on_model_param_valuechanged)
#define INSERT_PATCH(var) INSERT_VAR(var); PATCH_VAR(var)
constexpr static mha_real_t POWSPEC_FACTOR=0.0025; //MHA default powspec scaling
constexpr static mha_real_t OVERLAP_FACTOR=2; //redefine if needed, or make into a parameter
constexpr static mha_real_t EPSILON =1e-10;
}

/** Constructs the beamforming plugin. */
smooth_cepstrum::
smooth_cepstrum_if_t::smooth_cepstrum_if_t(algo_comm_t iac, const std::string&)
    : MHAPlugin::plugin_t<smooth_cepstrum_t>("Cepstral smoothing single-channel noise reduction",iac),
      xi_min_db("Minimum a priori SNR for a bin in dB(power)","-27.0","[-50,50]"),
      f0_low("Lower limit for F0 detection in Hz","70.0","[0,400]"),
      f0_high("Upper limit for F0 detection in Hz","300","[0,400]"),
      delta_pitch("Quefrency half-width of pitch-set in samps","2","[0,20]"),
      lambda_thresh("Pitch detection threshold for smooth cepstrum in magnitude","0.2","[0,3]"),
      alpha_pitch("Alpha value to set for pitch range","0.15","[0,4]"),
      beta_const("AR coeff for smoothing of alphas(smoothing-factors)","0.96",""),
      kappa_const("Exponential bias correction constant for a priori SNR estimate","0.2886","[0,1]"),
      gain_min_db("Minimum gain in dB for a frequency bin", "-17", "[-30,0]"),
      win_f0("Window coefficients for cepstral smoothing window",
             "[0.0207 0.0656 0.1664 0.2473 0.2473 0.1664 0.0656 0.0207]","[0,1]"),
      alpha_const_vals("Piecewise values for steady-state alphas", "[0.2 0.4 0.92]","[0,2]"),
      alpha_const_limits_hz("Limits for steady-state alphas given in Hz","[93.75 625.0]","[0,10000]"),
      noisePow_name("Name of est. noise spectrum in AC space","noise_psd_estimator"),
      spp("Subparser for exporting SPP"),
      prior_q("priorQ for computing GLR and SPP from local SNR","0.5","[0,2]"),
      xi_opt_db("xiOpt in dB for computing GLR and SPP from local SNR","15","[0,40]"),
      prepared(false)
{
    INSERT_PATCH(xi_min_db);
    INSERT_PATCH(f0_low);
    INSERT_PATCH(f0_high);
    INSERT_PATCH(delta_pitch);
    INSERT_PATCH(lambda_thresh);
    INSERT_PATCH(alpha_pitch);
    INSERT_PATCH(beta_const);
    INSERT_PATCH(kappa_const);
    INSERT_PATCH(gain_min_db);
    INSERT_PATCH(win_f0);
    INSERT_PATCH(alpha_const_vals);
    INSERT_PATCH(alpha_const_limits_hz);
    INSERT_PATCH(noisePow_name);

    INSERT_VAR(spp);
    spp.INSERT_VAR(prior_q);
    spp.INSERT_VAR(xi_opt_db);

    PATCH_VAR(prior_q);
    PATCH_VAR(xi_opt_db);
}

/** Plugin preparation. This plugin checks that the input signal has the
   * spectral domain and contains at least one channel
   * @param signal_info
   *   Structure containing a description of the form of the signal (domain,
   *   number of channels, frames per block, sampling rate.
   */
void smooth_cepstrum::smooth_cepstrum_if_t::prepare(mhaconfig_t & signal_info)
{
    if (signal_info.domain != MHA_SPECTRUM)
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin can only process spectrum signals.");

    //tell the plugin that it's ok to prepare configurations
    prepared = true;

    /* remember the transform configuration (i.e. channel numbers): */
    tftype = signal_info;
    /* make sure that a valid runtime configuration exists: */
    update_cfg();
}

/* when one of the angles or radii changes, recompute the head model */
void smooth_cepstrum::smooth_cepstrum_if_t::on_model_param_valuechanged()
{
    //only push configurations if prepare has already been called
    if ( prepared ) update_cfg();
}

void smooth_cepstrum::smooth_cepstrum_if_t::update_cfg()
{
    smooth_params params( input_cfg(), xi_min_db.data, f0_low.data, f0_high.data,
                        delta_pitch.data, lambda_thresh.data, alpha_pitch.data,
                        beta_const.data, kappa_const.data, prior_q.data,
                        xi_opt_db.data, gain_min_db.data,
                        win_f0.data,
                        alpha_const_vals.data, alpha_const_limits_hz.data,
                        noisePow_name.data );
    push_config(new smooth_cepstrum_t(ac, params) );
}

/** This plugin implements noise reduction using spectral
   * subtraction: by nonnegative subtraction from the output magnitude
   * of the estimated noise magnitude spectrum.
   * @param signal
   *   Pointer to the input signal structure.
   * @return
   *   Returns a pointer to the input signal structure,
   *   with a the signal modified by this plugin.
   */
mha_spec_t * smooth_cepstrum::smooth_cepstrum_if_t::process(mha_spec_t * signal)
{
    return poll_config()->process(signal);
}

//TODO: "full" variables probably not needed,
//ie we just use the symmetric ffts provided by MHA

smooth_cepstrum::smooth_cepstrum_t::smooth_cepstrum_t(algo_comm_t & ac, smooth_params & params_) :

    ac( ac ), params( params_ ),
    fftlen( params.in_cfg.fftlen ),
    mha_fft( mha_fft_new( fftlen ) ),
    nfreq( fftlen/2+1 ),
    nchan( params.in_cfg.channels ),
    ola_powspec_scale( fftlen * fftlen / POWSPEC_FACTOR / OVERLAP_FACTOR ),
    q_low( floor(params.in_cfg.srate / params.f0_high) ),
    q_high( floor(params.in_cfg.srate / params.f0_low) ),
    winF0( params.winF0 ),
    xi_min( pow(10,params.xi_min_db/10.0) ),
    gain_min( pow(10,params.gain_min_db/20.0) ),
    alpha_const( nfreq, nchan ),
    alpha_prev( nfreq, nchan ),
    noisePow( nfreq, nchan ),
    powSpec( nfreq, nchan ),
    gamma_post( nfreq, nchan ),
    xi_ml( nfreq, nchan ),
    lambda_ml_full( fftlen, nchan ),
    lambda_ml_ceps( fftlen, nchan ),
    lambda_ml_smooth( nfreq, nchan ),
    alpha_hat( nfreq, nchan ),
    alpha_frame( nfreq, nchan ),
    lambda_ceps( fftlen, nchan ),
    lambda_ceps_prev( nfreq, nchan ),
    log_lambda_spec( fftlen, nchan ),
    lambda_spec( nfreq, nchan ),
    xi_est( nfreq, nchan ),
    gain_wiener( nfreq, nchan ),
    spec_out( nfreq, nchan ),
    max_val( new double[nchan] ),
    max_q( new int[nchan] ),
    pitch_set_first( new int[nchan] ),
    pitch_set_last( new int[nchan] ),
    priorFact( params.prior_q / (1-params.prior_q)),
    xiOpt( pow(10.0, params.xi_opt_db/10.0) ),
    logGLRFact( log(1./(1+xiOpt)) ),
    GLRexp( xiOpt/(1+xiOpt) ),
    GLR(nfreq, nchan)
{
    //assume there is one more val than there are limits
    unsigned int b=0;
    unsigned int nbands = params.alpha_const_vals.size();
    for (unsigned int f=0; f<nfreq; ++f)
    {
        float hz = float(f * params.in_cfg.srate) / float(fftlen);

        //go to next band under certain conditions
        while ((b<nbands-1) && //while not last band
               //and hz is bigger than current band limit
               (hz >= params.alpha_const_limits_hz[b]))
            ++b; //advance to the next band

        for(unsigned c=0U; c<nchan; ++c) {
            alpha_const.value(f,c) = params.alpha_const_vals[b];
        }
    }

    alpha_prev.copy(alpha_const);
    lambda_ceps_prev.assign(0);
}

smooth_cepstrum::smooth_cepstrum_t::~smooth_cepstrum_t() {
    delete [] max_val;
    delete [] max_q;
    delete [] pitch_set_first;
    delete [] pitch_set_last;
}

/* TODO: go back and make sure things are duplicated (OR NOT) across channels */
mha_spec_t *smooth_cepstrum::smooth_cepstrum_t::process(mha_spec_t *noisyFrame)
{
    //get the latest noise spectrum estimation
    mha_wave_t noisePowFrame = MHA_AC::get_var_waveform(ac, params.noisePow_name);
    noisePow.copy(noisePowFrame);

    powSpec.powspec( *noisyFrame );

    for( unsigned int k=0;k<size(powSpec);k++) {
        powSpec[k] = powSpec[k] * ola_powspec_scale;
    }

    for (unsigned int f=0; f<nfreq; ++f)
    {
        for(unsigned c=0U; c<nchan; ++c)
        {
            float denom = std::max((mha_real_t) EPSILON,noisePow.value(f,c));
            gamma_post.value(f,c) = powSpec.value(f,c) / denom;

            xi_ml.value(f,c) = gamma_post.value(f,c) - 1;
            lambda_ml_full.value(f,c).re = noisePow.value(f,c) * std::max( xi_ml.value(f,c), xi_min );

        }
    }

    for (unsigned int f=0; f<nfreq; ++f)
    {
        for(unsigned c=0U; c<nchan; ++c)
        {
            //take the log in anticipation of cepstrum
            lambda_ml_full.value(f,c).re = log( lambda_ml_full.value(f,c).re );
            MHAFilter::make_friendly_number( lambda_ml_full.value(f,c).re );
        }
    }

    //complete the right half of the spectrum
    for (unsigned int f=1; f<nfreq; ++f)
    {
        for(unsigned c=0U; c<nchan; ++c)
        {
            lambda_ml_full.value( fftlen-f, c ).re = lambda_ml_full.value(f,c).re;
        }
    }

    //TODO: we might be able to use spec2wav for this
    mha_fft_backward_scale(mha_fft, &lambda_ml_full, &lambda_ml_ceps);
    for (unsigned int f=0; f<fftlen; ++f)
    {
        for(unsigned c=0U; c<nchan; ++c)
        {
            lambda_ml_ceps.value(f,c).im = 0; //kill the imaginary
        }
    }

    lambda_ml_smooth.assign(0); //init smooth

    //centered filtering of lambda_ml_ceps
    int halfWin = winF0.num_frames/2;
    for (unsigned int f=0; f<nfreq; ++f)
    {
        for (unsigned int w=0; w<winF0.num_frames; ++w)
        {
            for(unsigned c=0U; c<nchan; ++c)
            {
                if (int(f+w)-halfWin < 0) continue;
                if (f+w-halfWin >= nfreq) continue;

                lambda_ml_smooth.value(f,c) += lambda_ml_ceps.value(f+w-halfWin,c).re * winF0[w];
            }
        }
    }

    //init peak-picking
    for(unsigned c=0U; c<nchan; ++c)
    {
        max_val[c] = -10000;
        max_q[c] = -1;
    }

    //pick a cepstral peak
    for (unsigned int q=q_low; q<=q_high; ++q)
    {
        for(unsigned c=0U; c<nchan; ++c)
        {
            if ( lambda_ml_smooth.value(q,c) > max_val[c] )
            {
                max_val[c] = lambda_ml_smooth.value(q,c);
                max_q[c] = q;
            }
        }
    }

    //define the pitch set
    for(unsigned c=0U; c<nchan; ++c)
    {

        if ( (max_val[c] > params.lambda_thresh) && (lambda_ml_ceps.value(1,c).re > 0) )
        {
            pitch_set_first[c] = max_q[c] - params.delta_pitch;
            pitch_set_last[c] = max_q[c] + params.delta_pitch;
        }
        else
        {
            pitch_set_first[c] = -1;
            pitch_set_last[c] = -1;
        }
    }

    //exponentially filter previous smoothing factors
    for (unsigned int q=0; q<nfreq; ++q)
    {
        for(unsigned c=0U; c<nchan; ++c)
        {
            //watch out for denormals here
            mha_real_t f = params.beta_const * alpha_prev.value(q,c) + (1-params.beta_const) * alpha_const.value(q,c);
            MHAFilter::make_friendly_number( f );
            alpha_hat.value(q,c) = f;
        }
    }

    alpha_frame.copy( alpha_hat ); //default vals

    //in pitch set, use alpha_pitch
    for(unsigned c=0U; c<nchan; ++c)
    {
        if ( pitch_set_first[c] > 0 )
        {
            for (int q=pitch_set_first[c]; q<=pitch_set_last[c]; ++q)
            {
                alpha_frame.value(q,c) = params.alpha_pitch;
            }
        }
    }

    //save last smoothing factors
    alpha_prev.copy( alpha_frame );

    //quefrency adaptive smoothing
    for (unsigned int q=0; q<nfreq; ++q)
    {
        for(unsigned c=0U; c<nchan; ++c)
        {
            //also watch out for denormals here
            mha_real_t f;
            f = alpha_frame.value(q,c) * lambda_ceps_prev.value(q,c) +
                    (1-alpha_frame.value(q,c)) * lambda_ml_ceps.value(q,c).re;
            MHAFilter::make_friendly_number( f );
            lambda_ceps.value(q,c).re = f;

            lambda_ceps_prev.value(q,c) = f;
        }
    }

    //complete the spectrum
    for (unsigned int q=1; q<nfreq; ++q)
    {
        for(unsigned c=0U; c<nchan; ++c)
        {
            lambda_ceps.value( fftlen-q, c ).re = lambda_ceps.value(q,c).re;
        }
    }

    mha_fft_forward_scale(mha_fft, &lambda_ceps, &log_lambda_spec);
    for (unsigned int f=0; f<fftlen; ++f)
    {
        for(unsigned c=0U; c<nchan; ++c)
        {
            log_lambda_spec.value(f,c).im = 0; //kill the imaginary
        }
    }

    spec_out.copy( *noisyFrame ); //copy input

    for (unsigned int f=0; f<nfreq; ++f)
    {
        for(unsigned c=0U; c<nchan; ++c)
        {
            lambda_spec.value(f,c) = exp( params.kappa_const + log_lambda_spec.value(f,c).re );
            MHAFilter::make_friendly_number( lambda_spec.value(f,c) );

            float denom = std::max( noisePow.value(f,c), (mha_real_t) EPSILON );
            xi_est.value(f,c) = std::max( lambda_spec.value(f,c) / denom, xi_min );

            gain_wiener.value(f,c) = xi_est.value(f,c) / (1 + xi_est.value(f,c));
            gain_wiener.value(f,c) = std::max( gain_wiener.value(f,c), gain_min );

            //apply filter
            spec_out.value(f,c) *= gain_wiener.value(f,c);
        }
    }

    //compute SPP HERE
    for (unsigned int f=0; f<nfreq; ++f)
    {
        for(unsigned c=0U; c<nchan; ++c)
        {
            GLR(f,c) = priorFact * exp(std::min(logGLRFact + GLRexp * xi_est(f,c),(float)50.0));
            MHAFilter::make_friendly_number( GLR(f,c) );
        }
    }

    return &spec_out;
}





/*
 * This macro connects the plugin1_t class with the MHA plugin C interface
 * The first argument is the class name, the other arguments define the 
 * input and output domain of the algorithm.
 */
MHAPLUGIN_CALLBACKS(smooth_cepstrum,smooth_cepstrum::smooth_cepstrum_if_t,spec,spec)

/*
 * This macro creates code classification of the plugin and for
 * automatic documentation.
 *
 * The first argument to the macro is a space separated list of
 * categories, starting with the most relevant category. The second
 * argument is a LaTeX-compatible character array with some detailed
 * documentation of the plugin.
 */
MHAPLUGIN_DOCUMENTATION\
(smooth_cepstrum,
 "noise-suppression signal-enhancement adaptive",
 "Single-channel noise reduction applying cepstral smoothing based on\n"
"noise power spectral density (PSD). The PSD must be provided by another plugin as an AC variable.\n"
"The PSD computed by the 'noise\\_psd\\_estimator' plugin is compatible with this plugin.\n"
" The name of the AC variable to read the PSD can be changed in the parameter \\emph{noisePow\\_name}.\n"
"\n"
"References:\n"
"\n"
"Colin Breithaupt, Timo Gerkmann, Rainer Martin, \"A Novel A Priori SNR\n"
"Estimation Approach Based on Selective Cepstro-Temporal Smoothing\", IEEE\n"
"Int. Conf. Acoustics, Speech, Signal Processing, Las Vegas, NV, USA,\n"
"Apr. 2008.\n"
"\n"
"Timo Gerkmann, Rainer Martin, \"On the Statistics of Spectral Amplitudes\n"
"After Variance Reduction by Temporal Cepstrum Smoothing and Cepstral\n"
"Nulling\", IEEE Trans. Signal Processing, Vol. 57, No. 11, pp. 4165-4174,\n"
"Nov. 2009.\n"
"\n"
"Patent:\n"
"\n"
"Colin Breithaupt, Timo Gerkmann, and Rainer Martin: \"Spectral Smoothing\n"
"Method for Noisy Signals\", European Patent EP2158588B1, granted Oct.\n"
"2010, Danish Patent DK2158588T3, granted Feb. 2011, US Patent\n"
"US8892431B2, granted Nov. 2014.\n"
)

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
