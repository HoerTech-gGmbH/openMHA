// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2009 2012 2013 2016 2017 2018 HörTech gGmbH
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

#include "complex_filter.h"
#include "mha_signal_fft.h"

MHAFilter::complex_bandpass_t::complex_bandpass_t(std::vector<mha_complex_t> A,std::vector<mha_complex_t> B)
    : A_(A),B_(B),Yn(std::vector<mha_complex_t>(A.size(),mha_complex(0.0,0.0)))
{
}


void MHAFilter::complex_bandpass_t::set_weights(std::vector<mha_complex_t> new_B)
{
    MHA_assert_equal(new_B.size(), B_.size());
    B_ = new_B;
}

std::vector<mha_complex_t> MHAFilter::complex_bandpass_t::creator_A(std::vector<mha_real_t> cf,std::vector<mha_real_t> bw,mha_real_t srate,unsigned int order)
{
    std::vector<mha_complex_t> ret;
    MHA_assert_equal(cf.size(), bw.size());
    for(unsigned int k=0;k<cf.size();k++){
        mha_real_t theta = M_PI*bw[k]/srate;
        mha_real_t u = pow(2.0,-1.0/order);
        mha_real_t u2 = u*u;
        mha_real_t ct = cos(theta);
        mha_real_t ct2 = ct*ct;
        mha_real_t lambda =  (sqrt((ct2-1.0)*u2+(2.0-2.0*ct)*u)+ct*u-1.0)/(u-1.0);
        mha_complex_t val;
        expi(val,2.0*M_PI*cf[k]/srate,lambda);
        ret.push_back(val);
    }
    return ret;
}

std::vector<mha_complex_t> MHAFilter::complex_bandpass_t::creator_B(std::vector<mha_complex_t> A,unsigned int order)
{
    std::vector<mha_complex_t> ret;
    for(unsigned int k=0;k<A.size();k++)
        ret.push_back(mha_complex(pow(2.0,1.0/order)*(1.0-abs(A[k])),0));
    return ret;
}

MHAFilter::gamma_flt_t::gamma_flt_t(std::vector<mha_real_t> cf,std::vector<mha_real_t> bw,mha_real_t srate,unsigned int order)
    : A(MHAFilter::complex_bandpass_t::creator_A(cf,bw,srate,order)),
      GF(order,MHAFilter::complex_bandpass_t(A,MHAFilter::complex_bandpass_t::creator_B(A,order))),
      delay(NULL),
      envelope_delay(cf.size(),0),
      resynthesis_gain(cf.size(),0.0f),
      cf_(cf),
      bw_(bw),
      srate_(srate)
{
    MHA_assert(order >= 1);
}


MHAFilter::gamma_flt_t::~gamma_flt_t()
{
    if( delay )
        delete delay;
}

void MHAFilter::gamma_flt_t::reset_state()
{
    for(unsigned int k=0;k<GF.size();k++)
        GF[k].set_state(0);
    if( delay ){
        delete delay;
        delay = new MHASignal::delay_t(envelope_delay,envelope_delay.size());
    }
}

void MHAFilter::gamma_flt_t::phase_correction(unsigned int desired_delay,unsigned int inchannels)
{
    if( delay )
        delete delay;
    delay = NULL;
    MHAFilter::gamma_flt_t ext_gtfb(cf_,bw_,srate_,GF.size());
    std::vector<mha_complex_t> B = ext_gtfb.get_weights();
    set_weights(B);
    if( desired_delay == 0 )
        return;
    unsigned int bands(cf_.size());
    MHASignal::waveform_t delta(desired_delay,bands);
    unsigned int ch;
    unsigned int k;
    for(ch=0;ch<bands;ch++)
        value(delta,0,ch) = 1.0f;
    MHASignal::spectrum_t response(desired_delay,bands);
    ext_gtfb(delta,response);
    unsigned int min_delay = desired_delay;
    for(ch=0;ch<bands;ch++){
        unsigned int env_delay_tmp = 0;
        mha_real_t max_env = 0;
        mha_complex_t cval(mha_complex(0.0f));
        for(k=0;k<response.num_frames;k++){
            if( abs(value(response,k,ch))>max_env ){
                max_env = abs(value(response,k,ch));
                env_delay_tmp = k;
                cval = value(response,k,ch);
            }
        }
        normalize(cval,1e-20);
        B[ch] /= cval;
        envelope_delay[ch] = desired_delay - env_delay_tmp;
        if( desired_delay - env_delay_tmp < min_delay )
            min_delay = desired_delay - env_delay_tmp;
    }
    for(ch=0;ch<bands;ch++)
        envelope_delay[ch] -= min_delay;
    GF[0].set_weights(B);
    delay = new MHASignal::delay_t(envelope_delay,envelope_delay.size());
    MHASignal::waveform_t delta_long((unsigned int)(srate_),bands);
    MHASignal::waveform_t response_long_re((unsigned int)srate_,bands);
    MHASignal::waveform_t response_long_im((unsigned int)srate_,bands);
    for(ch=0;ch<bands;ch++)
        value(delta_long,0,ch) = 1.0f;
    operator()(delta_long,response_long_re,response_long_im);
    MHASignal::waveform_t response_long_1ch((unsigned int)srate_,1);
    for(ch=0;ch<bands;ch++)
        for(k=0;k<response_long_re.num_frames;k++)
            value(response_long_1ch,k,0) += value(response_long_re,k,ch);
    MHASignal::spectrum_t H_1hz(delta_long.num_frames/2+1,1);
    MHASignal::fft_t fft(delta_long.num_frames);
    fft.wave2spec(&response_long_1ch,&H_1hz,false);
    H_1hz *= response_long_1ch.num_frames;
    for(ch=0;ch<bands;ch++)
        resynthesis_gain[ch] = inchannels / abs(value(H_1hz,(unsigned int)(cf_[ch]+0.5),0));
    reset_state();
}

void MHAFilter::gamma_flt_t::set_weights(unsigned int stage,std::vector<mha_complex_t> new_B)
{
    GF[stage].set_weights(new_B);
}

void MHAFilter::gamma_flt_t::set_weights(std::vector<mha_complex_t> new_B)
{
    for(unsigned int stage=0;stage<GF.size();stage++)
        GF[stage].set_weights(new_B);
}


void MHAFilter::complex_bandpass_t::set_state(mha_real_t val)
{
    for(unsigned int k=0;k<Yn.size();k++)
        Yn[k] = mha_complex(val,0);
}

void MHAFilter::complex_bandpass_t::set_state(std::vector<mha_real_t> val)
{
    MHA_assert_equal(val.size(), Yn.size());
    for(unsigned int k=0;k<Yn.size();k++)
        Yn[k] = mha_complex(val[k],0);
}

void MHAFilter::complex_bandpass_t::set_state(mha_complex_t val)
{
    for(unsigned int k=0;k<Yn.size();k++)
        Yn[k] = val;
}


//*******************************************
//
// 3rd octave ANALYZER
//
//*******************************************

MHAFilter::thirdoctave_analyzer_t::thirdoctave_analyzer_t(mhaconfig_t cfg)
    : cfg_(cfg),
      cf(cf_generator(cfg)),
      fb(dup(cf_generator(cfg),cfg),dup(bw_generator(cfg),cfg),cfg.srate,4),
      out_chunk(cfg.fragsize,cfg.channels*cf.size()),
      out_chunk_im(cfg.fragsize,cfg.channels*cf.size())
{
}

mha_wave_t* MHAFilter::thirdoctave_analyzer_t::process(mha_wave_t* sIn)
{
    for(unsigned int ch=0;ch<sIn->num_channels;ch++)
        for(unsigned int band=0;band<nbands();band++)
            out_chunk.copy_channel(*sIn,ch,nbands()*ch+band);
    fb(out_chunk,out_chunk,out_chunk_im);
    return &out_chunk;
}

unsigned int MHAFilter::thirdoctave_analyzer_t::nbands()
{
    return cf.size();
}

unsigned int MHAFilter::thirdoctave_analyzer_t::nchannels()
{
    return cfg_.channels;
}

std::vector<mha_real_t> MHAFilter::thirdoctave_analyzer_t::get_cf_hz()
{
    return cf;
}

std::vector<mha_real_t> MHAFilter::thirdoctave_analyzer_t::cf_generator(mhaconfig_t cfg)
{
    std::vector<mha_real_t> cf;
    for(int kb=-12;kb<=12;kb++){
        float f = 1000.0f*powf(2.0f,(float)kb/3.0f);
        if( f < 0.5*cfg.srate ){
            cf.push_back(f);
        }
    }
    return cf;
}

std::vector<mha_real_t> MHAFilter::thirdoctave_analyzer_t::bw_generator(mhaconfig_t cfg)
{
    std::vector<mha_real_t> cf(cf_generator(cfg));
    std::vector<mha_real_t> bw;
    float df1 = powf(2.0f,1.0f/12.0f);
    float df2 = powf(2.0f,-1.0f/12.0f);
    for(unsigned int k=0;k<cf.size();k++){
        bw.push_back(cf[k]*df1-cf[k]*df2);
    }
    return bw;
}

std::vector<mha_real_t> MHAFilter::thirdoctave_analyzer_t::dup(std::vector<mha_real_t> vec,mhaconfig_t cfg)
{
    std::vector<mha_real_t> veco;
    for(unsigned int ch=0;ch<cfg.channels;ch++)
        for(unsigned int k=0;k<vec.size();k++)
            veco.push_back(vec[k]);
    return veco;
}


// Local Variables:
// mode: c++
// c-basic-offset: 4
// compile-command: "make -C .."
// coding: utf-8-unix
// indent-tabs-mode: nil
// End:
