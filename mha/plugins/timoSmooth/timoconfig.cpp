// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2014 2016 2017 HörTech gGmbH
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

/*
 * Single channel noise reduction algorithm from Breithaupt et al,
 * based on cepstral smoothing. This is the realtime processing class.
 */

#include "mha_plugin.hh"

#include "timoconfig.h"
#include "timosmooth.h"

#include "mha_filter.hh"

#include <algorithm> //std::min

#define LPSCALE (5.2429e+007) //large power scaling
#define POWSPEC_FACTOR  0.0025 //MHA default powspec scaling
#define OVERLAP_FACTOR  2   //redefine if needed, or make into a parameter

#define EPSILON (1e-10)

#define CHANLOOP for ( unsigned int c=0; c<nchan; ++c )

//TODO: "full" variables probably not needed,
//ie we just use the symmetric ffts provided by MHA

timoConfig::timoConfig(algo_comm_t & ac, timo_params & params) :

    ac( ac ), params( params ),
    fftlen( params.in_cfg.fftlen ),
    mha_fft( mha_fft_new( fftlen ) ),
    nfreq( fftlen/2+1 ),
    nchan( params.in_cfg.channels ),
    tAC(ac, fftlen, nfreq, nchan),
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

        CHANLOOP {
            alpha_const.value(f,c) = params.alpha_const_vals[b];
        }
    }

    alpha_prev.copy(alpha_const);
    lambda_ceps_prev.assign(0);
}

timoConfig::~timoConfig() {
    delete [] max_val;
    delete [] max_q;
    delete [] pitch_set_first;
    delete [] pitch_set_last;
}

/* TODO: go back and make sure things are duplicated (OR NOT) across channels */
mha_spec_t *timoConfig::process(mha_spec_t *noisyFrame)
{
    tAC.insert();

    //get the latest noise spectrum estimation
    mha_wave_t noisePowFrame = MHA_AC::get_var_waveform(ac, params.noisePow_name);
    noisePow.copy(noisePowFrame);

    powSpec.powspec( *noisyFrame );

    for( unsigned int k=0;k<size(powSpec);k++) {
        powSpec[k] = powSpec[k] * ola_powspec_scale;
    }

    for (unsigned int f=0; f<nfreq; ++f)
    {
        CHANLOOP
        {
            float denom = std::max((mha_real_t) EPSILON,noisePow.value(f,c));
            gamma_post.value(f,c) = powSpec.value(f,c) / denom;

            xi_ml.value(f,c) = gamma_post.value(f,c) - 1;
            lambda_ml_full.value(f,c).re = noisePow.value(f,c) * std::max( xi_ml.value(f,c), xi_min );

        }
    }

    for (unsigned int f=0; f<nfreq; ++f)
    {
        CHANLOOP
        {
            //take the log in anticipation of cepstrum
            lambda_ml_full.value(f,c).re = log( lambda_ml_full.value(f,c).re );
            MHAFilter::make_friendly_number( lambda_ml_full.value(f,c).re );
        }
    }

    //complete the right half of the spectrum
    for (unsigned int f=1; f<nfreq; ++f)
    {
        CHANLOOP
        {
            lambda_ml_full.value( fftlen-f, c ).re = lambda_ml_full.value(f,c).re;
        }
    }

    //TODO: we might be able to use spec2wav for this
    mha_fft_backward_scale(mha_fft, &lambda_ml_full, &lambda_ml_ceps);
    for (unsigned int f=0; f<fftlen; ++f)
    {
        CHANLOOP
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
            CHANLOOP
            {
                if (int(f+w)-halfWin < 0) continue;
                if (f+w-halfWin >= nfreq) continue;

                lambda_ml_smooth.value(f,c) += lambda_ml_ceps.value(f+w-halfWin,c).re * winF0[w];
            }
        }
    }

    //init peak-picking
    CHANLOOP
    {
        max_val[c] = -10000;
        max_q[c] = -1;
    }

    //pick a cepstral peak
    for (unsigned int q=q_low; q<=q_high; ++q)
    {
        CHANLOOP
        {
            if ( lambda_ml_smooth.value(q,c) > max_val[c] )
            {
                max_val[c] = lambda_ml_smooth.value(q,c);
                max_q[c] = q;
            }
        }
    }

    //define the pitch set
    CHANLOOP
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
        CHANLOOP
        {
            //watch out for denormals here
            mha_real_t f = params.beta_const * alpha_prev.value(q,c) + (1-params.beta_const) * alpha_const.value(q,c);
            MHAFilter::make_friendly_number( f );
            alpha_hat.value(q,c) = f;
        }
    }

    alpha_frame.copy( alpha_hat ); //default vals

    //in pitch set, use alpha_pitch
    CHANLOOP
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
        CHANLOOP
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
        CHANLOOP
        {
            lambda_ceps.value( fftlen-q, c ).re = lambda_ceps.value(q,c).re;
        }
    }

    mha_fft_forward_scale(mha_fft, &lambda_ceps, &log_lambda_spec);
    for (unsigned int f=0; f<fftlen; ++f)
    {
        CHANLOOP
        {
            log_lambda_spec.value(f,c).im = 0; //kill the imaginary
        }
    }

    spec_out.copy( *noisyFrame ); //copy input

    for (unsigned int f=0; f<nfreq; ++f)
    {
        CHANLOOP
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
        CHANLOOP
        {
            GLR(f,c) = priorFact * exp(std::min(logGLRFact + GLRexp * xi_est(f,c),(float)50.0));
            MHAFilter::make_friendly_number( GLR(f,c) );

            tAC.SPP(f,c) = GLR(f,c)/(1+GLR(f,c));
        }
    }

    copy_AC(tAC);

    return &spec_out;
}

void timoConfig::copy_AC(timo_AC &tAC)
{
    //copy AC variables
    tAC.gamma_post_AC.copy( gamma_post );
    tAC.xi_ml_AC.copy( xi_ml );
    tAC.lambda_ml_AC.copy( lambda_ml_full );
    tAC.lambda_ml_ceps_AC.copy( lambda_ml_ceps );
    tAC.lambda_ml_smooth_AC.copy( lambda_ml_smooth );

    CHANLOOP
    {
        tAC.max_q_AC.value(0,c) = max_q[c];
        tAC.max_val_AC.value(0,c) = max_val[c];
        tAC.pitch_set_first_AC(0,c) = pitch_set_first[c];
        tAC.pitch_set_last_AC(0,c) = pitch_set_last[c];
    }

    tAC.alpha_hat_AC.copy( alpha_hat );
    tAC.alpha_frame_AC.copy( alpha_frame );
    tAC.lambda_ceps_AC.copy( lambda_ceps );
    tAC.log_lambda_spec_AC.copy( log_lambda_spec );
    tAC.lambda_spec_AC.copy( lambda_spec );
    tAC.xi_est_AC.copy( xi_est );
    tAC.gain_wiener_AC.copy( gain_wiener );
}

void timo_AC::insert()
{
    gamma_post_AC.insert();
    xi_ml_AC.insert();
    lambda_ml_AC.insert();
    lambda_ml_ceps_AC.insert();
    lambda_ml_smooth_AC.insert();
    max_q_AC.insert();
    max_val_AC.insert();
    alpha_hat_AC.insert();
    alpha_frame_AC.insert();
    lambda_ceps_AC.insert();
    log_lambda_spec_AC.insert();
    lambda_spec_AC.insert();
    xi_est_AC.insert();
    gain_wiener_AC.insert();
}

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
