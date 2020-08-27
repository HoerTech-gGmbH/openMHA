// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2009 2010 2013 2014 2015 2017 2018 HörTech gGmbH
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
#include "mha_error.hh"
#include "mha_parser.hh"
#include "mha.hh"
#include "mha_plugin.hh"
#include "mha_signal.hh"
#include "mha_events.h"
#include <math.h>
#include "windowselector.h"

inline unsigned int max(unsigned int a, unsigned int b)
{
    if( a > b )
        return a;
    return b;
}

inline unsigned int min(unsigned int a, unsigned int b)
{
    if( a < b )
        return a;
    return b;
}

class hanning_ramps_t {
public:
    hanning_ramps_t(unsigned int,unsigned int);
    ~hanning_ramps_t();
    void operator()(MHASignal::waveform_t&);
private:
    unsigned int len_a;
    unsigned int len_b;
    mha_real_t *ramp_a;
    mha_real_t *ramp_b;
};

class spec2wave_t {
public:
    spec2wave_t(unsigned int nfft_,unsigned int nwnd_,unsigned int nwndshift_,
    unsigned int nch,mha_real_t ramplen,const MHAWindow::base_t& postwin);
    ~spec2wave_t();
    mha_wave_t* process(mha_spec_t*);
private:
    mha_fft_t ft; //!< FFT class
    unsigned int npad1; //!< length of zero padding before window
    unsigned int npad2; //!< length of zero padding after window
    hanning_ramps_t ramps;
    MHASignal::waveform_t calc_out;
    MHASignal::waveform_t out_buf;
    MHASignal::waveform_t write_buf;  
    mha_real_t sc;
    unsigned int nfft;
    unsigned int nwndshift;
    MHAWindow::base_t postwindow;
};

class spec2wave_if_t : public MHAPlugin::plugin_t<spec2wave_t> {
public:
    spec2wave_if_t(const algo_comm_t&,const std::string&,const std::string&);
    void prepare(mhaconfig_t&);
    void release();
    mha_wave_t* process(mha_spec_t*);
private:
    void update();
    void setlock(bool b);
    MHAParser::float_t ramplen;
    windowselector_t window_config;
};

/**********************************************************************/

hanning_ramps_t::hanning_ramps_t(unsigned int la,unsigned int lb)
    : len_a(la),
      len_b(lb),
      ramp_a(new mha_real_t[max(1,len_a)]),
      ramp_b(new mha_real_t[max(1,len_b)])
{
    unsigned int k;
    for(k=0; k<len_a; k++ )
        ramp_a[k] = 0.5 * (1 - cos( M_PI * k / len_a ) );
    for(k=0; k<len_b; k++ )
        ramp_b[k] = 0.5 * (1 + cos( M_PI * k / len_b ) );
}

hanning_ramps_t::~hanning_ramps_t(void)
{
    delete [] ramp_a;
    delete [] ramp_b;
}

void hanning_ramps_t::operator()(MHASignal::waveform_t& b)
{
    unsigned int k;
    unsigned int kb = max(b.num_frames,len_b)-len_b;
    for(k=0; k<min(len_a,b.num_frames);k++)
        b.scale_frame(k,ramp_a[k]);
    for(k=kb;k<b.num_frames;k++)
        b.scale_frame(k,ramp_b[k-kb]);
}

/**********************************************************************/

spec2wave_t::spec2wave_t(unsigned int nfft_,unsigned int nwnd_,unsigned int nwndshift_,unsigned int nch,mha_real_t ramplen,const MHAWindow::base_t& postwin)
    : npad1((unsigned int)(floor(0.5*(nfft_ - nwnd_)))),
      npad2(nfft_ - nwnd_ - npad1),
      ramps((unsigned int)(ramplen*npad1),(unsigned int)(ramplen*npad2)),
      calc_out(nfft_,nch),
      out_buf(nfft_,nch),
      write_buf(nwndshift_,nch),
      sc(2.0*(mha_real_t)nwndshift_/(mha_real_t)nwnd_*sqrt(0.375*nwnd_/nfft_)),
      nfft(nfft_),
      nwndshift(nwndshift_),
      postwindow(postwin)
{
    postwindow *= sc;
    ft = mha_fft_new( nfft );
}

spec2wave_t::~spec2wave_t()
{
    mha_fft_free( ft );
}

mha_wave_t* spec2wave_t::process(mha_spec_t* spec_in)
{
    unsigned int k;
    mha_fft_spec2wave( ft, spec_in, &calc_out );
    postwindow(calc_out);
    ramps(calc_out);
    out_buf.copy_from_at(0,nfft-nwndshift,out_buf,nwndshift);
    for(k=nfft-nwndshift; k<nfft; k++)
        out_buf.assign_frame(k,0);
    out_buf += calc_out;
    write_buf.copy_from_at(0,nwndshift,out_buf,0);
    return &write_buf;
}

spec2wave_if_t::spec2wave_if_t(const algo_comm_t& iac,const std::string&,const std::string&)
    : MHAPlugin::plugin_t<spec2wave_t>("spectrum to waveform iFFT plugin\n"
                                       "Performs inverse FFT, postwindowing,\n"
                                       "hanning ramps at zero-padding,\n"
                                       " overlap-add, normalization.\n"
                                       "Note that normalization only works"
                                       " for mod(wndlen,fragsize)=0.\n"
                                       "Also note that postwindowing only"
                                       " works for wndpos=0.5.\n"
                                       "Alway set ramplen=0 here if wndpos!=0"
                                       " in the corresponding wave2spec.",
                                       iac),
      ramplen("Relative length of post windowing hanning ramps"
              " (for centered analysis window)",
              "1",
              "[0,1]"),
      window_config("rect")
{
    insert_item("ramplen",&ramplen);
    window_config.insert_items(this);
}

void spec2wave_if_t::prepare(mhaconfig_t& t)
{
    try{
        setlock(true);
    if( t.domain != MHA_SPECTRUM )
        throw MHA_ErrorMsg("spec2wave: Spectral input is required.");
    t.domain = MHA_WAVEFORM;
    tftype = t;
    update();
    poll_config();
    }
    catch(MHA_Error& e){
        setlock(false);
    }
}

void spec2wave_if_t::release(){
    setlock(false);
}

mha_wave_t* spec2wave_if_t::process(mha_spec_t* spec_in)
{
    return cfg->process(spec_in);
}

void spec2wave_if_t::update()
{
    if (is_prepared()) {
        if( tftype.fftlen )
            push_config(new spec2wave_t(tftype.fftlen,
                                        tftype.wndlen,
                                        tftype.fragsize,
                                        tftype.channels,
                                        ramplen.data,
                                        window_config.get_window_data(tftype.fftlen)));
        else
            throw MHA_ErrorMsg("unsuitable fftlen == 0");
    }
}

void spec2wave_if_t::setlock(bool b){
    window_config.setlock(b);
    ramplen.setlock(b);
}
MHAPLUGIN_CALLBACKS(spec2wave,spec2wave_if_t,spec,wave)
MHAPLUGIN_DOCUMENTATION\
(spec2wave,
 "signal-transformation overlap-add",
 "This plugin calculates the inverse FFT and overlap add\n"
 "resynthesis. The parameters are taken from the framework overlap add\n"
 "parameters. After the inverse Fourier transform, hanning window ramps\n"
 "are applied to the previously zero-padded regions.\n"
 )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
