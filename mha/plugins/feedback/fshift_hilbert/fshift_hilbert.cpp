// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2008 2009 2010 2011 2014 2015 2017 HörTech gGmbH
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

#include "mha_plugin.hh"
#include "mha_signal.hh"
#include "mha_parser.hh"
#include "mha_defs.h"
#include "mha_events.h"
#include "mha_filter.hh"
#include <math.h>
#include <limits>
#include "sfftw.h"
#include "srfftw.h"

class hilbert_shifter_t : public MHASignal::spectrum_t
{
public:
    hilbert_shifter_t(unsigned int fftlen,
                      unsigned int channels,
                      mha_real_t srate,
                      unsigned int kmin,
                      unsigned int kmax,
                      std::vector<mha_real_t> dphi,
                      unsigned int frameshift,
                      unsigned int maxirslen,
                      unsigned int phasemode);
    ~hilbert_shifter_t();
    void process(mha_spec_t*);
private:
    MHASignal::spectrum_t fullspec;
    MHASignal::spectrum_t analytic;
    MHASignal::waveform_t shifted;
    MHASignal::spectrum_t mixw_shift;
    MHASignal::spectrum_t mixw_ref;
    fftw_plan plan_spec2analytic;
    mha_fft_t mhafft;
    //mha_real_t phi, dphi;
    MHASignal::waveform_t phi;
    MHASignal::waveform_t dphi;
    unsigned int kmin;
    unsigned int kmax;
    unsigned int frameshift;
};

hilbert_shifter_t::hilbert_shifter_t(unsigned int fftlen,
                                     unsigned int channels,
                                     mha_real_t srate,
                                     unsigned int kmin_,
                                     unsigned int kmax_,
                                     std::vector<mha_real_t> dphi_,
                                     unsigned int frameshift_,
                                     unsigned int maxirslen,
                                     unsigned int phasemode)
    : MHASignal::spectrum_t(fftlen/2+1,channels),
      fullspec(fftlen,channels),
      analytic(fftlen,channels),
      shifted(fftlen,channels),
      mixw_shift(fftlen/2+1,channels),
      mixw_ref(fftlen/2+1,channels),
      //phi(0),
      //dphi(dphi_),
      phi(1,channels),
      dphi(MHASignal::dupvec_chk(dphi_,channels)),
      kmin(kmin_),
      kmax(kmax_),
      frameshift(frameshift_)
{
    //dphi = dphi_;
    mhafft = mha_fft_new(fftlen);
    plan_spec2analytic = fftw_create_plan( fftlen, FFTW_BACKWARD, FFTW_ESTIMATE );
    unsigned int k, ch;
    for(ch=0; ch<channels;ch++){
        dphi[ch] *= M_PI*2.0/srate;
        for(k=0;k<fftlen/2+1;k++){
            if( (k >= kmin) && (k <= kmax) )
                mixw_shift.value(k,ch) = mha_complex(1,0);
            else
                mixw_ref.value(k,ch) = mha_complex(1,0);
        }
    }
    MHAFilter::smoothspec_t* smsp;
    switch( phasemode ){
    case 0 : // none
        smsp = NULL;
        break;
    case 1 : // linear
        smsp = new MHAFilter::smoothspec_t(fftlen,channels,MHAWindow::fun_t(maxirslen,&MHAWindow::hanning,-1,1),false);
        break;
    case 2 : // minimal
        smsp = new MHAFilter::smoothspec_t(fftlen,channels,MHAWindow::fun_t(maxirslen,&MHAWindow::hanning,0,1),true);
        break;
    default:
        throw MHA_Error(__FILE__,__LINE__,"Invalid phase mode %d",phasemode);
    }
    if( smsp ){
        smsp->smoothspec( mixw_shift );
        smsp->smoothspec( mixw_ref );
        delete smsp;
    }
}

hilbert_shifter_t::~hilbert_shifter_t()
{
    mha_fft_free(mhafft);
}

void hilbert_shifter_t::process(mha_spec_t* s)
{
    unsigned int k, ch;
    mha_real_t lphi;
    mha_complex_t cph;
    
    clear(fullspec);
    for(ch=0;ch<num_channels;ch++){
        for(k=0;k<s->num_frames;k++){
            fullspec(k,ch) = ::value(s,k,ch);
            fullspec(k,ch) *= 2;
            fullspec(k,ch) *= mixw_shift(k,ch);
            ::value(s,k,ch) *= mixw_ref(k,ch);
        }
        fullspec(0,ch) *= 0.5;
        if( 2*(s->num_frames-1) == fullspec.num_frames ) // even FFT length
            fullspec(s->num_frames-1,ch) *= 0.5;
    }
    for(ch=0;ch<num_channels;ch++){
        fftw_one(plan_spec2analytic,
                 (fftw_complex*)(&(fullspec.buf[fullspec.num_frames*ch])),
                 (fftw_complex*)(&(analytic.buf[analytic.num_frames*ch])));
    }
    for(ch=0;ch<num_channels;ch++){
        lphi = phi[ch];
        for(k=0;k<shifted.num_frames;k++){
            expi(cph,lphi);
            lphi += dphi[ch];
            analytic(k,ch) *= cph;
            shifted(k,ch) = analytic(k,ch).re;
        }
        phi[ch] += frameshift * dphi[ch];
        // @todo: review next line, this works only for positive phi?
        phi[ch] -= 2.0f*M_PI*floor(phi[ch] / (2.0f*M_PI));
    }
    mha_fft_wave2spec(mhafft,&shifted,this);
    *s += *this;
}

class frequency_translator_t : public MHAPlugin::plugin_t<hilbert_shifter_t> {
public:
    frequency_translator_t(const algo_comm_t&,const std::string&,const std::string&);
    mha_spec_t* process(mha_spec_t*);
    void prepare(mhaconfig_t&);
    void release();
private:
    void update();
    MHAEvents::patchbay_t<frequency_translator_t> patchbay;
    MHAParser::vfloat_t df;
    MHAParser::float_t fmin;
    MHAParser::float_t fmax;
    MHAParser::int_t irslen;
    MHAParser::kw_t phasemode;
};

//MHASignal::dupvec_chk
frequency_translator_t::frequency_translator_t(const algo_comm_t& iac,const std::string& ith,const std::string& ial)
    : MHAPlugin::plugin_t<hilbert_shifter_t>("Pitch shifter",iac),
      df("frequency to shift the bins / Hz","40",""),
      fmin("lower boundary for frequency shifter","4000","[0,]"),
      fmax("upper boundary for frequency shifter","16000","[0,]"),
      irslen("Bandpass: maximum length of cut off filter response","1","[1,]"),
      phasemode("Bandpass: mode of gain smoothing","none","[none linear minimal]")
{
    insert_item("df",&df);
    insert_item("fmin",&fmin);
    insert_item("fmax",&fmax);
    insert_item("irslen",&irslen);
    insert_item("phasemode",&phasemode);
    patchbay.connect(&writeaccess,this,&frequency_translator_t::update);
}

mha_spec_t* frequency_translator_t::process(mha_spec_t* s)
{
    poll_config();
    cfg->process(s);
    return s;
}

void frequency_translator_t::prepare(mhaconfig_t& tf)
{
    if( tf.domain != MHA_SPECTRUM )
        throw MHA_ErrorMsg("frequency_translator: Only spectral processing is supported.");
    if( tf.srate > 0 ){
        mha_real_t frate = 0.5*tf.srate;
        for(unsigned int k=0;k<df.data.size();k++){
            if( df.data[k] > frate )
                df.data[k] = frate;
            if( df.data[k] < -frate )
                df.data[k] = -frate;
        }
        char tmp[140];
        sprintf(tmp,"[%g,%g]",-frate,frate);
        df.set_range(tmp);
    }
    tftype = tf;
    update();
}

void frequency_translator_t::release()
{
    df.set_range("");
}

void frequency_translator_t::update()
{
    if( tftype.srate > 0 )
        push_config(new hilbert_shifter_t(
                        tftype.fftlen,tftype.channels,tftype.srate,
                        (unsigned int)(fmin.data*tftype.fftlen/tftype.srate),
                        (unsigned int)(fmax.data*tftype.fftlen/tftype.srate),
                        df.data,
                        tftype.fragsize,
                        irslen.data,
                        phasemode.data.get_index()
                        ));
}

MHAPLUGIN_CALLBACKS(fshift_hilbert,frequency_translator_t,spec,spec)
MHAPLUGIN_DOCUMENTATION(fshift_hilbert,"feedback","")

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
