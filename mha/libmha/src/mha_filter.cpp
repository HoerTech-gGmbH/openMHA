// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2003 2004 2005 2006 2007 2008 2009 2010 2011 HörTech gGmbH
// Copyright © 2012 2013 2014 2016 2017 2018 2019 2020 HörTech gGmbH
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

#include "mha_defs.h"
#include "mha_filter.hh"
#include <math.h>
#include <limits>
#include <valarray>
#include <algorithm>
#include <memory>
using namespace MHAFilter;

MHAFilter::filter_t::filter_t(unsigned int ch,
                              unsigned int ilen_A,
                              unsigned int ilen_B)
    : A(NULL),
      B(NULL),
      len_A(ilen_A),
      len_B(ilen_B),
      len(0),
      channels(ch),
      state(NULL)
{
    unsigned int k;
    // the number of audio channels can not be zero:
    CHECK_EXPR(channels);
    // recursive and non-recursive coefficients need at least one entry:
    len = std::max(len_A,len_B);
    if( std::min(len_A,len_B) == 0 )
        throw MHA_ErrorMsg("invalid filter length: 0");
    // allocate filter coefficient buffers and initialize to identity:
    A = new double[len_A];
    memset(A,0,sizeof(A[0])*len_A);
    A[0] = 1.0;
    B = new double[len_B];
    memset(B,0,sizeof(B[0])*len_B);
    B[0] = 1.0;
    // allocate filter state buffer and initialize to zero:
    state = new double[len*channels];
    for( k=0;k<len*channels;k++)
        state[k] = 0;
}

MHAFilter::filter_t::filter_t(const MHAFilter::filter_t& src)
    : A(new double[src.len_A]),
      B(new double[src.len_B]),
      len_A(src.len_A),
      len_B(src.len_B),
      len(src.len),
      channels(src.channels),
      state(new double[len*channels])
{
    memmove(A,src.A,len_A*sizeof(double));
    memmove(B,src.B,len_B*sizeof(double));
    memmove(state,src.state,len*channels*sizeof(double));
}


MHAFilter::filter_t::filter_t(unsigned int ch,const std::vector<mha_real_t>& vA, const std::vector<mha_real_t>& vB)
    : A(NULL),
      B(NULL),
      len_A(vA.size()),
      len_B(vB.size()),
      len(0),
      channels(ch),
      state(NULL)
{
    unsigned int k;
    // the number of audio channels can not be zero:
    MHA_assert(channels > 0);
    // recursive and non-recursive coefficients need at least one entry:
    MHA_assert(vA.size() > 0);
    MHA_assert(vB.size() > 0);
    len = std::max(len_A,len_B);
    // allocate filter coefficient buffers and initialize to identity:
    A = new double[len_A];
    B = new double[len_B];
    for(k=0;k<len_A;k++)
        A[k] = vA[k];
    for(k=0;k<len_B;k++)
        B[k] = vB[k];
    // allocate filter state buffer and initialize to zero:
    state = new double[len*channels];
    for( k=0;k<len*channels;k++)
        state[k] = 0;
}

void MHAFilter::filter_t::filter(mha_wave_t* dest,
                                 const mha_wave_t* src)
{
    if( dest->num_channels != channels )
        throw MHA_Error(__FILE__,__LINE__,
                        "mismatching number of channels (dest:%u filter:%u)",
                        dest->num_channels,channels);
    if( dest->num_channels != src->num_channels )
        throw MHA_Error(__FILE__,__LINE__,
                        "mismatching number of channels (dest:%u src:%u)",
                        dest->num_channels,src->num_channels);
    if( dest->num_frames != src->num_frames )
        throw MHA_Error(__FILE__,__LINE__,
                        "mismatching number of frames (dest:%u src:%u)",
                        dest->num_frames,src->num_frames);
    filter(dest->buf,src->buf,dest->num_frames,dest->num_channels,1,0,dest->num_channels);
}

mha_real_t MHAFilter::filter_t::filter(mha_real_t x, unsigned int ch)
{
    mha_real_t y = 0;
    filter(&y,&x,1,1,1,ch,ch+1);
    return y;
}

void MHAFilter::filter_t::filter(mha_real_t* dest,
                                 const mha_real_t* src,
                                 unsigned int dframes,
                                 unsigned int frame_dist,
                                 unsigned int channel_dist,
                                 unsigned int channel_begin,
                                 unsigned int channel_end)
{
    // validate channel count:
    if( channel_end > channels )
        throw MHA_Error(__FILE__,__LINE__,
                        "channels out of range (dest:%u-%u filter:%u)",
                        channel_begin,channel_end,channels);
    unsigned int ch, fr, nfr = dframes, n, idx;
    // direct form II, one delay line for each channel
    // A[k] are the recursive filter coefficients (A[0] is typically 1)
    // B[k] are the non recursive filter coefficients
    // loop through all frames, and all channels:
    for(fr=0;fr<nfr;fr++){
        for(ch=channel_begin;ch<channel_end;ch++){
            // index into input/output buffer:
            idx = frame_dist * fr + channel_dist * (ch - channel_begin);
            // shift filter delay line for current channel:
            for(n=len-1; n>0; n--)
                state[channels*n+ch] = state[channels*(n-1)+ch];
            // replace first delay line entry by input signal:
            //state[ch] =  src[idx] / A[0];
            state[ch] =  src[idx];
            // apply recursive coefficients:
            for(n = 1; n < len_A; n++ )
                 state[ch] -= state[channels*n+ch] * A[n];
            make_friendly_number( state[ch] );
            // apply non recursive coefficients to output:
            dest[idx] = 0;
            for(n=0; n<len_B; n++)
                dest[idx] += state[channels*n+ch] * B[n];
            // normalize by first recursive element:
            dest[idx] /= A[0];
            make_friendly_number( dest[idx] );
        }
    }
}

MHAFilter::filter_t::~filter_t()
{
    delete [] A;
    delete [] B;
    delete [] state;
}

std::vector<mha_real_t> diff_coeffs()
{
    std::vector<mha_real_t> r;
    r.push_back(1.0f);
    r.push_back(-1.0f);
    return r;
}

MHAFilter::diff_t::diff_t(unsigned int ch)
    : MHAFilter::filter_t(ch,std::vector<mha_real_t>(1,1.0f),diff_coeffs())
{
}

void MHAFilter::butter_stop_ord1(double* A,double* B,double f1,double f2,double fs)
{
    double uf1 = 2.0*fs*tan(M_PI*f1/fs);
    double uf2 = 2.0*fs*tan(M_PI*f2/fs);
    double w = sqrt(uf1*uf2);
    double q = w/(uf2-uf1);
    double t = 1.0/fs;
    double sigma = w*t/2;
    double D = sigma*sigma + sigma/q + 1.0;
    double sigma_D = sigma/D;
    double ssigma_D = sigma*sigma_D;
    double d1 = sigma_D/q;
    double d = 1.0 - d1;
    double tmp_a = 1.0/D - d1 - ssigma_D + 2.0*d1/D;
    double tmp_b = 2.0*sigma_D*(1 + d1);
    double tmp_c = -tmp_b;
    double tmp_d = 1.0/D + d1 - ssigma_D - 2.0*ssigma_D*d1;
    A[0] = 1.0;
    A[1] = 2.0*ssigma_D - 2.0/D;
    A[2] = ssigma_D*ssigma_D + d1*d1*(2.0*q*q-1.0)+1.0/(D*D);
    B[0] = d;
    B[1] = -(tmp_a + tmp_d) + (d-1.0)*A[1];
    B[2] = tmp_a*tmp_d - tmp_b*tmp_c + (d-1)*A[2];
}

std::vector<float> MHAFilter::fir_lp(float f_pass_, float f_stop_, float fs_,
                                     unsigned order_) {
  MHASignal::spectrum_t f(fs_ / 2 + 1, 1);
  unsigned int k;
  MHA_assert(f_stop_>=f_pass_);
  MHA_assert(f_stop_<=fs_/2);
  // pass band:
  for (k = 0; k < f_pass_ && k<f.num_frames; k++) {
    f[k] = mha_complex(1, 0);
  }
  // Hann ramp:
  for (k = f_pass_; k < f_stop_ && k<f.num_frames; k++) {
      f[k] = mha_complex(MHAWindow::hanning(0.5*(k - f_pass_) / (f_stop_ - f_pass_)),0);
  }
  // stop band:
  for (k = f_stop_; k < f.num_frames; k++) {
    f[k] = mha_complex(0, 0);
  }
  auto coeffs = std::unique_ptr<MHASignal::waveform_t>(MHAFilter::spec2fir(
      &f, fs_, MHAWindow::fun_t(order_, MHAWindow::hamming, -1, 1, true, true),
      false));
  return coeffs->flatten();
}

MHAFilter::adapt_filter_t::adapt_filter_t(std::string help)
    : MHAParser::parser_t(help),
      mu("adaptation coefficient","0.01","]0,["),
      ntaps("filter length","10","]0,["),
      err_in("use error (instead of desired) signal","no"),
      nchannels(1)
{
    insert_item("mu",&mu);
    insert_item("ntaps",&ntaps);
    insert_item("errin",&err_in);
    connector.connect(&mu.writeaccess,this,&adapt_filter_t::update_mu);
    connector.connect(&err_in.writeaccess,this,&adapt_filter_t::update_mu);
    connector.connect(&ntaps.writeaccess,this,&adapt_filter_t::update_ntaps);
}

void MHAFilter::adapt_filter_t::filter(mha_wave_t y,mha_wave_t e,mha_wave_t x,mha_wave_t d)
{
    MHAPlugin::config_t<adapt_filter_state_t>::poll_config();
    MHAPlugin::config_t<adapt_filter_param_t>::poll_config();
    MHAPlugin::config_t<adapt_filter_state_t>::cfg->filter(y,e,x,d,
                                                           MHAPlugin::config_t<adapt_filter_param_t>::cfg->mu,
                                                           MHAPlugin::config_t<adapt_filter_param_t>::cfg->err_in);
}

void MHAFilter::adapt_filter_t::set_channelcnt(unsigned int nch)
{
    nchannels = nch;
    update_mu();
    update_ntaps();
}

void MHAFilter::adapt_filter_t::update_mu()
{
    MHAPlugin::config_t<adapt_filter_param_t>::push_config(new adapt_filter_param_t(mu.data,err_in.data));
}

void MHAFilter::adapt_filter_t::update_ntaps()
{
    MHAPlugin::config_t<adapt_filter_state_t>::push_config(new adapt_filter_state_t(ntaps.data,nchannels));
}

MHAFilter::adapt_filter_param_t::adapt_filter_param_t(mha_real_t imu,bool ierr_in) 
    : mu(imu),
      err_in(ierr_in)
{
}

MHAFilter::adapt_filter_state_t::adapt_filter_state_t(int intaps,int inchannels)
    : ntaps(intaps),
      nchannels(inchannels),
      W(ntaps,nchannels),
      X(ntaps,nchannels),
      od(1,nchannels),
      oy(1,nchannels)
{
    for(int ch=0;ch<nchannels; ch++)
        W.buf[ch] = 1;
}


void MHAFilter::adapt_filter_state_t::filter(mha_wave_t y,mha_wave_t e,mha_wave_t x,mha_wave_t d,mha_real_t mu, bool err_in)
{
    // validate arguments:
    unsigned int nframes = x.num_frames;
    if( y.num_frames != nframes )
        throw MHA_Error(__FILE__,__LINE__,
                        "Target signal y has mismatching length (y:%u input:%u).",
                        y.num_frames,nframes);
    if( d.num_frames != nframes )
        throw MHA_Error(__FILE__,__LINE__,
                        "Desired signal d has mismatching length (d:%u input:%u).",
                        d.num_frames,nframes);
    if( (int)(y.num_channels) != nchannels )
        throw MHA_Error(__FILE__,__LINE__,
                        "Target signal y has mismatching channel number (y:%u adapt. filter:%d).",
                        y.num_channels,nchannels);
    if( (int)(x.num_channels) != nchannels )
        throw MHA_Error(__FILE__,__LINE__,
                        "Input signal x has mismatching channel number (x:%u adapt. filter:%d).",
                        x.num_channels,nchannels);
    if( (int)(d.num_channels) != nchannels )
        throw MHA_Error(__FILE__,__LINE__,
                        "Desired signal d has mismatching channel number (d:%u adapt. filter:%d).",
                        d.num_channels,nchannels);
    unsigned int ch, kf, kh, idx, fidx;
    mha_real_t err;
    for(ch=0;(int)ch<nchannels;ch++)
        for(kf=0;kf<nframes;kf++){
            // index into external buffers:
            idx = ch+nchannels*kf;
            y.buf[idx] = 0;
            if( err_in )
                err = mu * od.buf[ch];
            else
                err = mu * (od.buf[ch] - oy.buf[ch]);
            od.buf[ch] = d.buf[idx];
            for(kh=ntaps-1;kh>0;kh--){
                fidx = ch+nchannels*kh;
                W.buf[fidx] += err * X.buf[fidx];
                if( W.buf[fidx] > 1.0e20 )
                    W.buf[fidx] = 1.0e20;
                if( W.buf[fidx] < -1.0e20 )
                    W.buf[fidx] = -1.0e20;
                X.buf[fidx] = X.buf[fidx - nchannels];
                y.buf[idx] += W.buf[fidx] * X.buf[fidx];
            }
            W.buf[ch] += err * X.buf[ch];
            if( W.buf[ch] > 1.0e20 )
                W.buf[ch] = 1.0e20;
            if( W.buf[ch] < -1.0e20 )
                W.buf[ch] = -1.0e20;
            X.buf[ch] = x.buf[idx];
            oy.buf[ch] = y.buf[idx] += W.buf[ch] * X.buf[ch];
            e.buf[idx] = d.buf[idx] - y.buf[idx];
        }
}


MHAFilter::iir_filter_state_t::iir_filter_state_t(unsigned int channels,std::vector<float> cf_A,std::vector<float> cf_B)
    : MHAFilter::filter_t(channels,cf_A.size(),cf_B.size())
{
    unsigned int k;
    mha_real_t A0 = 1;
    if( cf_A.size() ){
        A0 = cf_A[0];
        A[0] = 1;
    }
    for( k=1;k<cf_A.size();k++)
        A[k] = cf_A[k] / A0;
    for( k=0;k<cf_B.size();k++)
        B[k] = cf_B[k] / A0;
}

MHAFilter::iir_filter_t::iir_filter_t(std::string help,std::string def_A,std::string def_B,unsigned int channels)
    : MHAParser::parser_t(help),
      A("recursive filter coefficients",def_A),
      B("non-recursive filter coefficients",def_B),
      nchannels(channels)
{
    insert_item("A",&A);
    insert_item("B",&B);
    connector.connect(&A.writeaccess,this,&iir_filter_t::update_filter);
    connector.connect(&B.writeaccess,this,&iir_filter_t::update_filter);
    update_filter();
}

void MHAFilter::iir_filter_t::filter(mha_wave_t* y,const mha_wave_t* x)
{
    poll_config();
    cfg->filter(y,x);
}

mha_real_t MHAFilter::iir_filter_t::filter(mha_real_t x,unsigned int ch)
{
    poll_config();
    return cfg->filter(x,ch);
}

void MHAFilter::iir_filter_t::resize(unsigned int channels)
{
    nchannels = channels;
    update_filter();
}

void MHAFilter::iir_filter_t::update_filter()
{
    push_config(new iir_filter_state_t(nchannels,A.data,B.data));
}


/**
   \brief Constructor of low pass filter, sets sampling rate and time constants

   \param tau Vector of time constants
   \param fs Sampling rate
   \param startval Initial internal state value
 */
MHAFilter::o1flt_lowpass_t::o1flt_lowpass_t(const std::vector<mha_real_t>& tau,
                                            mha_real_t fs,
                                            mha_real_t startval)
    : MHAFilter::o1_ar_filter_t(tau.size(),fs)
{
    assign(startval);
    for(unsigned int k=0;k<tau.size();k++)
        set_tau(k,tau[k]);
}

/**
   \brief Constructor of low pass filter, sets sampling rate and time constants

   \param tau Vector of time constants
   \param fs Sampling rate
   \param startval Initial internal state value
*/
MHAFilter::o1flt_lowpass_t::o1flt_lowpass_t(const std::vector<mha_real_t>& tau,
                                            mha_real_t fs,
                                            const std::vector<mha_real_t>& startval)
    : MHAFilter::o1_ar_filter_t(tau.size(),fs)
{
    if(tau.size()!=startval.size())
        throw MHA_Error(__FILE__,__LINE__,"o1flt_lowpass_t: Size of tau vector and initial state vector not equal"
                        "(Got %zu and %zu)",
                        tau.size(),startval.size()
                        );
    for(unsigned int k=0;k<tau.size();k++){
        set_tau(k,tau[k]);
    }
    std::copy(std::begin(startval),std::end(startval),buf);
}

void MHAFilter::o1flt_lowpass_t::set_tau(unsigned int k,mha_real_t tau)
{
    set_tau_attack(k,tau);
    set_tau_release(k,tau);
}

void MHAFilter::o1flt_lowpass_t::set_tau(mha_real_t tau)
{
    for(unsigned int k=0;k<num_channels;k++)
        set_tau(k,tau);
}

/**
   \brief Constructor of low pass filter, sets sampling rate and time constants

   \param tau Vector of time constants
   \param fs Sampling rate
   \param startval Initial internal state value
 */
MHAFilter::o1flt_maxtrack_t::o1flt_maxtrack_t(const std::vector<mha_real_t>& tau,
                                             mha_real_t fs,
                                             mha_real_t startval)
    : MHAFilter::o1flt_lowpass_t(tau,fs,startval)
{
    assign(startval);
    for(unsigned int k=0;k<tau.size();k++)
        set_tau(k,tau[k]);
}

MHAFilter::o1flt_maxtrack_t::o1flt_maxtrack_t(const std::vector<mha_real_t>& tau,
                                              mha_real_t fs,
                                              const std::vector<mha_real_t>& startval)
    : MHAFilter::o1flt_lowpass_t(tau,fs,startval)
{
    for(unsigned int k=0;k<tau.size();k++)
        set_tau(k,tau[k]);
}

MHAFilter::o1flt_mintrack_t::o1flt_mintrack_t(const std::vector<mha_real_t>& tau,
                                             mha_real_t fs_,
                                             mha_real_t startval)
    : MHAFilter::o1flt_lowpass_t(tau,fs_,startval)
{
    assign(startval);
    for(unsigned int k=0;k<tau.size();k++)
        set_tau(k,tau[k]);
}

MHAFilter::o1flt_mintrack_t::o1flt_mintrack_t(const std::vector<mha_real_t>& tau,
                                              mha_real_t fs,
                                              const std::vector<mha_real_t>& startval)
    : MHAFilter::o1flt_lowpass_t(tau,fs,startval)
{
    for(unsigned int k=0;k<tau.size();k++)
        set_tau(k,tau[k]);
}

void MHAFilter::o1flt_maxtrack_t::set_tau(unsigned int k,mha_real_t tau)
{
    set_tau_attack(k,0);
    set_tau_release(k,tau);
}

void MHAFilter::o1flt_mintrack_t::set_tau(unsigned int k,mha_real_t tau)
{
    set_tau_attack(k,tau);
    set_tau_release(k,0);
}

void MHAFilter::o1flt_mintrack_t::set_tau(mha_real_t tau)
{
    for(unsigned int k=0;k<num_channels;k++)
        set_tau(k,tau);
}

void MHAFilter::o1flt_maxtrack_t::set_tau(mha_real_t tau)
{
    for(unsigned int k=0;k<num_channels;k++)
        set_tau(k,tau);
}

MHAFilter::fftfilter_t::fftfilter_t(unsigned int fragsize_,
                                    unsigned int channels_,
                                    unsigned int fftlen_)
    : fragsize(fragsize_),
      channels(channels_),
      fftlen(fftlen_),
      wInput_fft(fftlen,channels),
      wInput(range(wInput_fft,fftlen-fragsize,fragsize)),
      wOutput_fft(fftlen,channels),
      wOutput(range(wOutput_fft,fftlen-fragsize,fragsize)),
      sInput(fftlen/2+1,channels),
      sWeights(fftlen/2+1,channels),
      wIRS_fft(fftlen,channels)
{
    if( fftlen < fragsize )
        throw MHA_Error(__FILE__,__LINE__,
                        "The FFT length (%u) is less than "
                        "the fragment size (%u).",
                        fftlen, fragsize );
    fft = mha_fft_new(fftlen);
}

MHAFilter::fftfilter_t::~fftfilter_t()
{
    mha_fft_free(fft);
}

void MHAFilter::fftfilter_t::update_coeffs(const mha_wave_t* pwIRS)
{
    unsigned int maxl = std::min(pwIRS->num_frames,fftlen-fragsize+1);
    mha_wave_t h_maxl_src = range(*pwIRS,0,maxl);
    mha_wave_t h_maxl_dest = range(wIRS_fft,0,maxl);
    mha_wave_t h_remain = range(wIRS_fft,maxl,fftlen-maxl);
    clear(h_remain);
    assign(h_maxl_dest,h_maxl_src);
    h_maxl_dest *= fftlen;
    mha_fft_wave2spec( fft, &wIRS_fft, &sWeights );
}

void MHAFilter::fftfilter_t::filter(const mha_wave_t* pwIn, mha_wave_t** ppwOut, const mha_wave_t* pwIRS)
{
    update_coeffs( pwIRS );
    filter(pwIn,ppwOut,&sWeights);
}

void MHAFilter::fftfilter_t::filter(const mha_wave_t* pwIn, mha_wave_t** ppwOut)
{
    filter(pwIn,ppwOut,&sWeights);
}

void MHAFilter::fftfilter_t::filter(const mha_wave_t* pwIn, mha_wave_t** ppwOut,const mha_spec_t* psWeights)
{
    timeshift(wInput_fft,-fragsize);
    assign(wInput,*pwIn);
    mha_fft_wave2spec(fft, &wInput_fft, &sInput );
    sInput *= *psWeights;
    mha_fft_spec2wave(fft, &sInput, &wOutput_fft);
    *ppwOut = &wOutput;
}

void MHAFilter::o1_lp_coeffs(const mha_real_t tau,
                             const mha_real_t fs,
                             mha_real_t& c1,
                             mha_real_t& c2)
{
    if( (tau > 0) && (fs > 0) )
        c1 = exp( -1.0/(tau * fs) );
    else
        c1 = 0;
    c2 = 1.0 - c1;
}

MHAFilter::o1_ar_filter_t::o1_ar_filter_t(unsigned int channels,mha_real_t fs_,std::vector<mha_real_t> tau_a,std::vector<mha_real_t> tau_r)
    : MHASignal::waveform_t(1,channels),
      c1_a(1,channels),
      c2_a(1,channels),
      c1_r(1,channels),
      c2_r(1,channels),
      fs(fs_)
{
    if( fs < 0 )
        throw MHA_Error(__FILE__,__LINE__,"Invalid sampling rate (%g)",fs);
    tau_a = MHASignal::dupvec_chk(tau_a,channels);
    tau_r = MHASignal::dupvec_chk(tau_r,channels);
    for(unsigned int k=0;k<channels;k++){
        set_tau_attack(k,tau_a[k]);
        set_tau_release(k,tau_r[k]);
    }
}

void MHAFilter::o1_ar_filter_t::set_tau_attack(unsigned int ch,mha_real_t tau)
{
    if( ch >= num_channels )
        throw MHA_Error(__FILE__,__LINE__,"The filter channel is out of range (got %u, %u channels).",
                        ch,num_channels);
    MHAFilter::o1_lp_coeffs(tau,fs,c1_a[ch],c2_a[ch]);
}

void MHAFilter::o1_ar_filter_t::set_tau_release(unsigned int ch,mha_real_t tau)
{
    if( ch >= num_channels )
        throw MHA_Error(__FILE__,__LINE__,"The filter channel is out of range (got %u, %u channels).",
                        ch,num_channels);
    MHAFilter::o1_lp_coeffs(tau,fs,c1_r[ch],c2_r[ch]);
}

MHASignal::waveform_t* MHAFilter::spec2fir(const mha_spec_t* spec,const unsigned int fftlen,const MHAWindow::base_t& window,const bool minphase)
{
    CHECK_VAR(spec);
    MHAFilter::smoothspec_t smooth(fftlen,spec->num_channels,window,minphase);
    MHASignal::waveform_t* retv = new MHASignal::waveform_t(window.num_frames,spec->num_channels);
    smooth.spec2fir(*spec,*retv);
    return retv;
}

MHAFilter::smoothspec_t::smoothspec_t(unsigned int fftlen_,
                                      unsigned int nchannels_,
                                      const MHAWindow::base_t& window_,
                                      bool minphase_,bool linphase_asym)
    : fftlen(fftlen_),
      nchannels(nchannels_),
      window(window_),
      tmp_wave(fftlen,nchannels),
      tmp_spec(fftlen/2+1,nchannels),
      minphase(NULL),
      _linphase_asym(linphase_asym),
      fft(mha_fft_new(fftlen))
{
    if( minphase_ )
        minphase = new MHASignal::minphase_t(fftlen,nchannels);
    if( fftlen < window.num_frames )
        throw MHA_Error(__FILE__,__LINE__,"Invalid window length %u (longer than FFT length %u).",
                        window.num_frames,fftlen);
}

void MHAFilter::smoothspec_t::internal_fir(const mha_spec_t& s_in)
{
    unsigned int k, ch;
    tmp_spec.copy(s_in);
    if( minphase )
        (*minphase)(&tmp_spec);
    mha_fft_spec2wave(fft,&tmp_spec,&tmp_wave);
    if( minphase || _linphase_asym ){
        // apply complete window at beginning of IRS:
        for(ch=0;ch<nchannels;ch++){
            for(k=0;k<window.num_frames;k++)
                tmp_wave.value(k,ch) *= window[k];
            for(k=window.num_frames;k<fftlen;k++)
                tmp_wave.value(k,ch) = 0;
        }
    }else{
        // apply second half of window at beginning and first half at
        // end of IRS, set middle part to zero:
        unsigned int wnd2b = window.num_frames/2;
        unsigned int wnd2a = window.num_frames-wnd2b;
        for(ch=0;ch<nchannels;ch++){
            for(k=0;k<wnd2a;k++)
                tmp_wave.value(k,ch) *= window[wnd2b+k];
            for(k=wnd2a;k<fftlen-wnd2b;k++)
                tmp_wave.value(k,ch) = 0;
            for(k=0;k<wnd2b;k++)
                tmp_wave.value(k+fftlen-wnd2b,ch) *= window[k];
        }
    }
}

void MHAFilter::smoothspec_t::smoothspec(const mha_spec_t& s_in,mha_spec_t& s_out)
{
    internal_fir(s_in);
    mha_fft_wave2spec(fft,&tmp_wave,&s_out);
}

void MHAFilter::smoothspec_t::spec2fir(const mha_spec_t& spec,mha_wave_t& fir)
{
    if( fir.num_frames < window.num_frames )
        throw MHA_Error(__FILE__,__LINE__,"Provided FIR variable too short (got"
                        " %u, expected %u).",fir.num_frames,window.num_frames);
    if( fir.num_channels != nchannels )
        throw MHA_Error(__FILE__,__LINE__,
                        "Mismatching number of channels in FIR variable (got %u, expected %u).",
                        fir.num_channels,nchannels);
    internal_fir(spec);
    tmp_wave *= 1.0f/tmp_wave.num_frames;
    unsigned int k, ch;
    clear(fir);
    if( minphase ){
        // copy to beginning, IRS is causal:
        for( ch=0;ch<nchannels;ch++)
            for(k=0;k<window.num_frames;k++)
                value(fir,k,ch) = tmp_wave.value(k,ch);
    }else{
        // copy shifted, IRS is a-causal:
        unsigned int wnd2a = window.num_frames/2;
        unsigned int wnd2b = window.num_frames-wnd2a;
        for( ch=0;ch<nchannels;ch++){
            for(k=0;k<wnd2b;k++)
                value(fir,k,ch) = tmp_wave.value(fftlen-wnd2b+k,ch);
            for(k=0;k<wnd2a;k++)
                value(fir,wnd2b+k,ch) = tmp_wave.value(k,ch);
        }
    }
}

MHAFilter::smoothspec_t::~smoothspec_t()
{
    mha_fft_free(fft);
    if( minphase )
        delete minphase;
}

MHAFilter::fftfilterbank_t::fftfilterbank_t(unsigned int fragsize_,
                                            unsigned int inputchannels_,
                                            unsigned int firchannels_,
                                            unsigned int fftlen_)
    : fragsize(fragsize_),
      inputchannels(inputchannels_),
      firchannels(firchannels_),
      outputchannels(inputchannels*firchannels),
      fftlen(fftlen_),
      hw(fftlen,firchannels),
      Hs(fftlen/2+1,firchannels),
      xw(fftlen,inputchannels),
      Xs(fftlen/2+1,inputchannels),
      yw(fragsize,outputchannels),
      Ys(fftlen/2+1,outputchannels),
      yw_temp(fftlen,outputchannels),
      tail(std::max(fftlen,fragsize)-fragsize,outputchannels)
{
    if( fftlen < fragsize )
        throw MHA_Error(__FILE__,__LINE__,
                        "The FFT length (%u) is less than "
                        "the fragment size (%u).",
                        fftlen, fragsize );
    fft = mha_fft_new(fftlen);
}

MHAFilter::fftfilterbank_t::~fftfilterbank_t()
{
    mha_fft_free(fft);
}

void MHAFilter::fftfilterbank_t::update_coeffs(const mha_wave_t* h)
{
    if( h->num_frames + fragsize - 1 > fftlen )
        throw MHA_Error(__FILE__,__LINE__,
                        "The filter length is longer than the available space.");
    // irs is the zero padded impulse response:
    clear(hw);
    hw.copy_from_at(0,h->num_frames,*h,0);
    mha_fft_wave2spec( fft, &hw, &Hs );
    Hs *= fftlen;
}

void MHAFilter::fftfilterbank_t::filter(const mha_wave_t* s_in, 
                                        mha_wave_t** s_out, 
                                        const mha_wave_t* h)
{
    update_coeffs( h );
    filter( s_in, s_out );
}

void MHAFilter::fftfilterbank_t::filter(const mha_wave_t* s_in, 
                                    mha_wave_t** s_out)
{
    unsigned int ch, k, band;
    clear(xw);
    xw.copy_from_at( 0, s_in->num_frames, *s_in, 0);
    mha_fft_wave2spec( fft, &xw, &Xs );
    // copy input spectrum to output spectrum:
    // apply filter in the spectral domain and transform to time
    // domain (irs is again a temporary object):
    for(ch=0;ch<inputchannels;ch++)
        for( band=0;band < firchannels;band++)
            for( k=0;k<Xs.num_frames;k++){
                Ys(k,ch*firchannels+band) = Xs(k,ch);
                Ys(k,ch*firchannels+band) *= Hs(k,band);
            }
    mha_fft_spec2wave( fft, &Ys, &yw_temp );
    for(ch=0;ch<outputchannels;ch++){
        // copy the first part to output:
        for(k=0;k<fragsize;k++)
            yw.value(k,ch) = yw_temp.value(k,ch);
        // add the beginning of the last tail:
        for(k=0;k<std::min(fftlen-fragsize,fragsize);k++)
            yw.value(k,ch) += tail.value(k,ch);
        // shift the tail by fragsize and add input:
        for(k=0;k<std::max(fftlen-fragsize,fragsize)-fragsize;k++)
            tail.value(k,ch) = tail.value(k+fragsize,ch) + 
                yw_temp.value(k+fragsize,ch);
        // copy the tail (unshifted part):
        for(k=std::max(fftlen-fragsize,fragsize)-fragsize;k<fftlen-fragsize;k++)
            tail.value(k,ch) = yw_temp.value(k+fragsize,ch);
    }
    *s_out = &yw;
}


MHAFilter::transfer_function_t::
transfer_function_t(unsigned int source_channel_index_,
                    unsigned int target_channel_index_,
                    const std::vector<float> & impulse_response_)
    : source_channel_index(source_channel_index_),
      target_channel_index(target_channel_index_),
      impulse_response(impulse_response_)
{}

MHAFilter::partitioned_convolution_t::
partitioned_convolution_t(unsigned int fragsize_,
                          unsigned int nchannels_in_,
                          unsigned int nchannels_out_,
                          const transfer_matrix_t & transfer)
    : fragsize(fragsize_),
      nchannels_in(nchannels_in_),
      nchannels_out(nchannels_out_),
      output_partitions(transfer.partitions(fragsize).max()),
      filter_partitions(transfer.non_empty_partitions(fragsize).sum()),
      input_signal_wave(2*fragsize, nchannels_in),
      current_input_signal_buffer_half_index(0U),
      input_signal_spec(fragsize+1, nchannels_in),
      frequency_response(fragsize+1, filter_partitions),
      bookkeeping(filter_partitions),
      output_signal_spec(output_partitions, 
                         MHASignal::spectrum_t(fragsize+1, nchannels_out)),
      current_output_partition_index(0U),
      output_signal_wave(fragsize, nchannels_out),
      fft(mha_fft_new(2*fragsize))
{
    /* break up impulse responses into partitions */
    
    /// index into transfer
    unsigned transfer_function_index;

    /// ammount of delay, in blocks
    unsigned delay_index;

    /// index into frequency_response and partitions (channel index)
    unsigned filter_partition_index = 0;

    /// buffer for storing impulse response partitions
    MHASignal::waveform_t partitions(2*fragsize, filter_partitions);

    for (transfer_function_index = 0;
         transfer_function_index < transfer.size();
         ++transfer_function_index) {
        for (delay_index = 0; 
             delay_index < 
                 transfer[transfer_function_index].partitions(fragsize);
             ++delay_index) {
            if (! transfer[transfer_function_index].isempty(fragsize,
                                                            delay_index)) {
                if (filter_partition_index >= filter_partitions) {
                    // should not happen
                    throw MHA_Error(__FILE__,__LINE__,
                                    "Error calculating the number of needed"
                                    " filter partitions (%u)",  
                                    filter_partitions);
                }
                for (unsigned f = 0;
                     f < fragsize
                         &&
                         (f + delay_index * fragsize)
                         <
                         (transfer[transfer_function_index].
                          impulse_response.size());
                     ++f) {
                    partitions.value(f, filter_partition_index) =
                        transfer[transfer_function_index].
                        impulse_response[f + delay_index * fragsize];
                }
                bookkeeping[filter_partition_index].source_channel_index =
                    transfer[transfer_function_index].source_channel_index;
                if (bookkeeping[filter_partition_index].source_channel_index
                    >= nchannels_in)
                    throw MHA_Error(__FILE__,__LINE__,
                                    "Source channel index %u is out of range",
                                    bookkeeping[filter_partition_index].
                                    source_channel_index);
                bookkeeping[filter_partition_index].target_channel_index =
                    transfer[transfer_function_index].target_channel_index;
                if (bookkeeping[filter_partition_index].target_channel_index
                    >= nchannels_out)
                    throw MHA_Error(__FILE__,__LINE__,
                                    "Target channel index %u is out of range",
                                    bookkeeping[filter_partition_index].
                                    target_channel_index);
                bookkeeping[filter_partition_index].delay = delay_index;

                ++filter_partition_index;
            }
        }
    }
    mha_fft_wave2spec(fft, &partitions, &frequency_response);

    // Forward and inverse transform of the signal will lose a factor
    // fftlength. Normalize by multiplying the frequency response with
    // fftlength.
    frequency_response *= mha_real_t(2*fragsize);
}

MHAFilter::partitioned_convolution_t::~partitioned_convolution_t()
{
    mha_fft_free(fft);
}

mha_wave_t * 
MHAFilter::partitioned_convolution_t::process(const mha_wave_t * s_in)
{
    if (s_in == 0)
        throw MHA_ErrorMsg("Input signal pointer is NULL");
    if (s_in->num_frames != fragsize)
        throw MHA_Error(__FILE__,__LINE__, 
                        "Input signal num_frames (%u)"
                        " differs from fragsize (%u)",
                        s_in->num_frames, fragsize);
    if (s_in->num_channels != nchannels_in)
        throw MHA_Error(__FILE__,__LINE__, 
                        "Input signal num_channels (%u)"
                        " differs from nchannels_in (%u)",
                        s_in->num_channels, nchannels_in);
    
    // replace half of the input buffer with the new input signal.
    // The other half buffer retains the signal of the previous call
    // to this method.
    input_signal_wave.copy_from_at(fragsize *
                                   current_input_signal_buffer_half_index,
                                   fragsize, *s_in, 0);

    // fft
    mha_fft_wave2spec(fft, &input_signal_wave, &input_signal_spec,
                      bool(current_input_signal_buffer_half_index));

    // filter
    mha_complex_t temp;
    for (unsigned p = 0; // filter partition index
         p < filter_partitions;
         ++p)    {
        unsigned src = bookkeeping[p].source_channel_index;
        unsigned tgt = bookkeeping[p].target_channel_index;
        unsigned dly = 
            (bookkeeping[p].delay + current_output_partition_index)
            % output_partitions;
        // apply
        for (unsigned k = 0; k < fragsize+1; ++k) {
            temp = input_signal_spec.value(k,src);
            temp *= frequency_response.value(k, p);
            output_signal_spec[dly].value(k, tgt) += temp;
        }
    }
    
    // ifft
    mha_fft_spec2wave(fft,
                      &output_signal_spec[current_output_partition_index],
                      &output_signal_wave,
                      0);

    // clear output buffer for reuse
    clear(output_signal_spec[current_output_partition_index]);

    // update counters
    current_output_partition_index++;
    if (current_output_partition_index >= output_partitions)
        current_output_partition_index = 0;
    current_input_signal_buffer_half_index = 
        1U - current_input_signal_buffer_half_index;

    return &output_signal_wave;
}

MHAFilter::resampling_filter_t::resampling_filter_t(unsigned int fftlen, unsigned int irslen, unsigned int channels, unsigned int Nup, unsigned int Ndown, double fCutOff)
    : MHAFilter::fftfilter_t(fragsize_validator(fftlen,irslen),channels,fftlen),
      fragsize(fragsize_validator(fftlen,irslen))
{
    double q(fCutOff/std::max(Nup,Ndown));
    MHASignal::waveform_t h(irslen,channels);
    for(unsigned int k=0;k<h.num_frames;k++){
        h.value(k,0) = sinc(M_PI * q*(k-(irslen-1.0)/2.0));
    }
    for(unsigned int ch=1;ch<h.num_channels;ch++)
        copy_channel(h,h,0,ch);
    MHAWindow::hanning_t hann(h.num_frames);
    hann(h);
    h *= 1.0/h.sum();
    update_coeffs(&h);
}

unsigned int MHAFilter::resampling_filter_t::fragsize_validator(unsigned int fftlen, unsigned int irslen)
{
    if( fftlen < irslen )
        throw MHA_Error(__FILE__,__LINE__,
                        "The FFT length cannot be less than the filter length (FFT length: %u, Filter length: %u)",
                        fftlen,irslen);
    return fftlen-irslen+1;
}

double MHAFilter::sinc(double x)
{
    if( x == 0 )
        return 1.0;
    return sin(x)/x;
}

std::pair<unsigned,unsigned>
MHAFilter::resampling_factors(float source_sampling_rate,
                              float target_sampling_rate,
                              float factor)
{
    std::pair<unsigned,unsigned> result;
    if (factor <= 0 || source_sampling_rate <= 0 || target_sampling_rate <= 0)
        throw MHA_Error(__FILE__,__LINE__,
                        "MHAFilter::resampling_factors(%g,%g,%g): Called"
                        " with non-positive parameter",
                        source_sampling_rate, target_sampling_rate, factor);

    float src_rate_product = source_sampling_rate * factor;
    if (src_rate_product != roundf(src_rate_product))
        throw MHA_Error(__FILE__,__LINE__,
                        "MHAFilter::resampling_factors(%g,%g,%g):"
                        " source_sampling_rate * factor is not an integer",
                        source_sampling_rate, target_sampling_rate, factor);
    unsigned src_rate_integer = static_cast<unsigned>(src_rate_product);
    if (static_cast<float>(src_rate_integer) != src_rate_product)
        throw MHA_Error(__FILE__,__LINE__,
                        "MHAFilter::resampling_factors(%g,%g,%g):"
                        " source_sampling_rate * factor cannot be represented"
                        " by an unsigned integer instance",
                        source_sampling_rate, target_sampling_rate, factor);

    float tgt_rate_product = target_sampling_rate * factor;
    if (tgt_rate_product != roundf(tgt_rate_product))
        throw MHA_Error(__FILE__,__LINE__,
                        "MHAFilter::resampling_factors(%g,%g,%g):"
                        " target_sampling_rate * factor is not an integer",
                        source_sampling_rate, target_sampling_rate, factor);
    unsigned tgt_rate_integer = static_cast<unsigned>(tgt_rate_product);
    if (static_cast<float>(tgt_rate_integer) != tgt_rate_product)
        throw MHA_Error(__FILE__,__LINE__,
                        "MHAFilter::resampling_factors(%g,%g,%g):"
                        " target_sampling_rate * factor cannot be represented"
                        " by an unsigned integer instance",
                        source_sampling_rate, target_sampling_rate, factor);
    unsigned gcd = MHAFilter::gcd(tgt_rate_integer, src_rate_integer);

    result.first = tgt_rate_integer / gcd;
    result.second = src_rate_integer / gcd;
    return result;
}

MHAFilter::polyphase_resampling_t::polyphase_resampling_t
(unsigned n_up, unsigned n_down, 
 mha_real_t nyquist_ratio,
 unsigned n_irs,
 unsigned n_ringbuffer, unsigned n_channels,
 unsigned n_prefill)
    : upsampling_factor(n_up),
      downsampling_factor(n_down),
      now_index(n_up * n_prefill-1), underflow(false),
      impulse_response(n_irs),
      ringbuffer(n_ringbuffer, n_channels, n_prefill)
{
    // The remaining code in constructor computes the coefficients of the 
    // windowed sinc filter.
    // The coefficients of the ideal sinc filter are defined as 
    // h(n) = sinc(pi*q*n)
    // where q = 1 would mean that the cutoff is the nyquist frequency,
    // and n extends to positive and negative infinity.

    // The nyquist ratio applies to the smaller frequency. We are filtering
    // at the interpolation rate (generally higher), and have to compute the
    // cutoff frequency relative to that.
    double q = nyquist_ratio / std::max(n_up, n_down);

    double sum = 0; // for normalization
    for(unsigned k=0; k < n_irs; ++k)
        sum += (impulse_response.value(k,0) *= sinc(M_PI*q*(k-(n_irs-1.0)/2.0)));
    for(unsigned k=0; k < n_irs; ++k)
        // normalize filter output. Account for the fact that only
        // one in n_up samples is present at the interpolation rate.
        impulse_response.value(k,0) *= n_up / sum;
}
void MHAFilter::polyphase_resampling_t::write(mha_wave_t & signal) {
    if (underflow)
        throw MHA_Error(__FILE__,__LINE__,
                        "MHAFilter::polyphase_resampling_t::write: This"
                        " polyphase resampling instance has experienced an"
                        " underflow and may not be used anymore.");
    ringbuffer.write(signal);
}
void MHAFilter::polyphase_resampling_t::read(mha_wave_t & signal) {
    if (underflow)
        throw MHA_Error(__FILE__,__LINE__,
                        "MHAFilter::polyphase_resampling_t::write: This"
                        " polyphase resampling instance has experienced an"
                        " underflow and may not be used anymore.");

    assign(signal, 0.0f);

    unsigned input_index = unsigned(-1); // initializer needed if loop is empty

    for   (unsigned output_index = 0;
           output_index < signal.num_frames;
           ++output_index, now_index += downsampling_factor) {

        // latest interpolated signal index at which input signal is != 0
        // is at (now_index / upsampling_factor) * upsampling_factor
        input_index = now_index / upsampling_factor;


        // The corresponding index into the impulse response is
        // now_index - ((now_index / upsampling_factor) * upsampling_factor)
        unsigned convolution_index = now_index - input_index*upsampling_factor;
        for (;
             convolution_index < impulse_response.num_frames;
             --input_index, convolution_index += upsampling_factor) {
            if (input_index == (unsigned)(-1)) {
                underflow = true;
                throw MHA_Error(__FILE__,__LINE__,
                                "MHAFilter::polyphase_resampling_t::read: Not"
                                " enough input data: underflow detected when"
                                " producing output for"
                                " frame=%u convolution_index=%u",
                                output_index, convolution_index);
            }
            for (unsigned channel = 0; channel < signal.num_channels; ++channel)
            {
                float & input = ringbuffer.value(input_index, channel);
                float & filter_tap =
                    impulse_response.value(convolution_index, 0);
                float & accumulator = value(signal,output_index,channel);
                accumulator += input * filter_tap;
            }
        
          // input_index now contains one lower than the lowest ringbuffer index
          // in the input signal that was needed to produce this output frame.
        }
    }
    // input_index  now contains one lower than the lowest ringbuffer index
    // in the input signal that was needed to produce the last output frame.
    // it may be unsigned(-1), in which case we do not discard anything.
    // To produce the next sample, we may need the input sample at input_index
    // since the slice of the impulse response may be one tap longer and
    // the input signal index may not have advanced. (It may be possible that it
    // can be proven that this cannot happen, but I do not have time to
    // investigate. Therefore, better safe than sorry.)
    if (input_index != unsigned(-1)) {
        ringbuffer.discard(input_index);
        now_index -= (input_index) * upsampling_factor;
    }
}
MHAFilter::blockprocessing_polyphase_resampling_t::
blockprocessing_polyphase_resampling_t(float source_srate,
                                       unsigned source_fragsize,
                                       float target_srate,
                                       unsigned target_fragsize,
                                       float nyquist_ratio,
                                       float irslen,
                                       unsigned nchannels,
                                       bool add_delay)
    : resampling(0),
      fragsize_in(source_fragsize), fragsize_out(target_fragsize),
      num_channels(nchannels)
{
    std::pair<unsigned, unsigned> factors = 
        resampling_factors(source_srate, target_srate);
    unsigned & upsampling_factor = factors.first;
    unsigned & downsampling_factor = factors.second;
    float interpolation_srate = source_srate * upsampling_factor;
    unsigned irslen_samples = 
        static_cast<unsigned>(ceilf(irslen * interpolation_srate));
    unsigned interpolation_source_fragsize =
        source_fragsize * upsampling_factor;
    unsigned interpolation_target_fragsize =
        target_fragsize * downsampling_factor;
    unsigned interpolation_minimum_delay;
    if (add_delay) // Delay necessary to prevent underruns
        interpolation_minimum_delay = 
            interpolation_target_fragsize - 
            MHAFilter::gcd(interpolation_target_fragsize,
                           interpolation_source_fragsize);
    else
        interpolation_minimum_delay =
            interpolation_source_fragsize -
            MHAFilter::gcd(interpolation_source_fragsize,
                           interpolation_target_fragsize);
    unsigned source_minimum_delay = 0U;
    if (interpolation_minimum_delay > 0U)
        source_minimum_delay = 
            (interpolation_minimum_delay - 1U) / upsampling_factor + 1U;
    unsigned sufficient_interpolation_ringbuffer_size =
        source_minimum_delay * upsampling_factor +
        2U * std::max(interpolation_source_fragsize,
                      interpolation_target_fragsize) +
        irslen_samples + 1U;
    unsigned sufficient_source_ringbuffer_size =
        (sufficient_interpolation_ringbuffer_size - 1U) / upsampling_factor
        + 1U; 
    unsigned source_prefill = (irslen_samples - 1U) / upsampling_factor + 1U;
    resampling = new MHAFilter::
        polyphase_resampling_t(upsampling_factor, downsampling_factor,
                               nyquist_ratio,
                               irslen_samples,
                               sufficient_source_ringbuffer_size,
                               nchannels,
                               source_prefill);
    if (add_delay && (source_minimum_delay > 0U)) {
        MHASignal::waveform_t delay(source_minimum_delay, nchannels);
        resampling->write(delay);
    }
}

void MHAFilter::blockprocessing_polyphase_resampling_t::
write(mha_wave_t & signal)
{
    if (signal.num_frames != fragsize_in)
        throw MHA_Error(__FILE__,__LINE__,
                        "MHAFilter::"
                        "blockprocessing_polyphase_resampling_t::write:"
                        " Number %u of signal frames does not match"
                        " configured input block size %u",
                        signal.num_frames, fragsize_in);
    resampling->write(signal);
}

void MHAFilter::blockprocessing_polyphase_resampling_t::
read(mha_wave_t & signal) {
    if (signal.num_frames != fragsize_out)
        throw MHA_Error(__FILE__,__LINE__,
                        "MHAFilter::"
                        "blockprocessing_polyphase_resampling_t::read:"
                        " Number %u of frames in signal structure does"
                        " not match configured output block size %u",
                        signal.num_frames, fragsize_out);
    if (signal.num_channels != num_channels)
        throw MHA_Error(__FILE__,__LINE__,
                        "MHAFilter::"
                        "blockprocessing_polyphase_resampling_t::read: Number"
                        " %u of channels in signal structure does not"
                        " match configured number %u of channels",
                        signal.num_channels, num_channels);
    resampling->read(signal);
}   
            

MHAFilter::iir_ord1_real_t::iir_ord1_real_t(std::vector<mha_real_t> A,std::vector<mha_real_t> B)
    : A_(A),B_(B),Yn(std::vector<mha_complex_t>(A.size(),mha_complex(0.0,0.0)))
{
}

MHAFilter::iir_ord1_real_t::iir_ord1_real_t(std::vector<mha_real_t> tau,mha_real_t srate)
    : A_(tau),B_(tau),Yn(std::vector<mha_complex_t>(tau.size(),mha_complex(0.0,0.0)))
{
    for(unsigned int k=0;k<A_.size();k++)
        o1_lp_coeffs(tau[k],srate,A_[k],B_[k]); 
}

void MHAFilter::iir_ord1_real_t::set_state(mha_real_t val)
{
    for(unsigned int k=0;k<Yn.size();k++)
        Yn[k] = mha_complex(val,0);
}

void MHAFilter::iir_ord1_real_t::set_state(std::vector<mha_real_t> val)
{
    MHA_assert_equal(val.size(), Yn.size());
    for(unsigned int k=0;k<Yn.size();k++)
        Yn[k] = mha_complex(val[k],0);
}

void MHAFilter::iir_ord1_real_t::set_state(mha_complex_t val)
{
    for(unsigned int k=0;k<Yn.size();k++)
        Yn[k] = val;
}


// Local Variables:
// compile-command: "make -C .."
// c-basic-offset: 4
// coding: utf-8-unix
// indent-tabs-mode: nil
// End:
