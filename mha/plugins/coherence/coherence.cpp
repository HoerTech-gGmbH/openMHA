// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2008 2009 2010 2012 2013 2014 2015 HörTech gGmbH
// Copyright © 2016 2017 2018 2019 2020 HörTech gGmbH
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

#include "mha_toolbox.h"
#include "mha_plugin.hh"
#include "mha_fftfb.hh"
#include "mha_filter.hh"
#include "mha_events.h"
#include <math.h>
#include "mha_defs.h"

namespace coherence {

using namespace MHAOvlFilter;

class vars_t : public fftfb_vars_t {
public:
    vars_t(MHAParser::parser_t*);
    MHAParser::kw_t tau_unit;
    MHAParser::vfloat_t tau;
    MHAParser::vfloat_t alpha;
    MHAParser::float_t limit;
    MHAParser::vfloat_t mapping;
    MHAParser::kw_t average;
    MHAParser::bool_t invert;
    MHAParser::bool_t ltgcomp;
    MHAParser::vfloat_t ltgtau;
    MHAParser::vfloat_t staticgain;
    MHAParser::int_t delay;
};

vars_t::vars_t(MHAParser::parser_t* p)
    : fftfb_vars_t(*p),
      tau_unit("tau unit","seconds","[seconds periods]"),
      tau("Averaging time constant","[0.04]","[0,]"),
      alpha("Gain exponent","[1]","[0,]"),
      limit("gain limit / dB (zero: no limit)","0","[,0]"),
      mapping("mapping interval of coherence estimator to coherence (min max)","[0 1]","[0,1]"),
      average("average mode","ipd","[ipd spec]"),
      invert("Invert filter after mapping, before exponent.","no"),
      ltgcomp("Long term gain compensation?","no"),
      ltgtau("Long term gain estimation time constant / s","[1]","[0,]"),
      staticgain("Static gain in frequency bands / dB","[0]"),
      delay("Delay between analysis and filter (delay of gains), in fragments.","0","[0,]")
{
    p->insert_member(tau_unit);
    p->insert_member(tau);
    p->insert_member(alpha);
    p->insert_member(limit);
    p->insert_member(mapping);
    p->insert_member(average);
    p->insert_member(invert);
    p->insert_member(ltgcomp);
    p->insert_member(ltgtau);
    p->insert_member(staticgain);
    p->insert_member(delay);
}

class cohflt_t : private fftfb_t, private mhaconfig_t {
public:
    cohflt_t(vars_t& v,
             const mhaconfig_t& icf,
             algo_comm_t iac,
             const std::string& name);
    mha_spec_t* process(mha_spec_t*);
    void insert();
private:
    unsigned int nbands;
    bool avg_ipd;
    mha_complex_t cg;
    float g;
    float c_scale;
    float c_min;
    MHASignal::waveform_t alpha;
    float limit;
    MHAFilter::o1flt_lowpass_t lp1r;
    MHAFilter::o1flt_lowpass_t lp1i;
    MHA_AC::spectrum_t coh_c;
    MHA_AC::waveform_t coh_rlp;
    MHASignal::waveform_t gain;
    MHASignal::delay_wave_t gain_delay;
    MHASignal::spectrum_t s_out;
    bool bInvert;
    MHAFilter::o1flt_lowpass_t lp1ltg;
    bool b_ltg;
    std::vector<float> staticgain;
};

class cohflt_if_t : public MHAPlugin::plugin_t<cohflt_t> {
public:
    cohflt_if_t(const algo_comm_t&,
                const std::string&,
                const std::string&);
    void prepare(mhaconfig_t&);
    void release();
    mha_spec_t* process(mha_spec_t*);
private:
    void update();
    MHAEvents::patchbay_t<cohflt_if_t> patchbay;
    vars_t vars;
    const std::string algo;
};

cohflt_t::cohflt_t(vars_t& v,
                   const mhaconfig_t& icf,
                   algo_comm_t ac,
                   const std::string& name)
    : fftfb_t(v,icf.fftlen,icf.srate),
      mhaconfig_t(icf),
      nbands(fftfb_t::nbands()),
      avg_ipd(v.average.data.get_index()==0),
      c_scale(1),
      c_min(0),
      alpha(nbands,1),
      limit(2.0*v.limit.data/M_PI),
      lp1r(MHASignal::dupvec_chk(v.tau.data,nbands),icf.srate/mha_min_1(icf.fragsize)),
      lp1i(MHASignal::dupvec_chk(v.tau.data,nbands),icf.srate/mha_min_1(icf.fragsize)),
      coh_c(ac,name+"_ccoh",nbands,1,false),
      coh_rlp(ac,name+"_rcoh",nbands,1,false),
      gain(nbands,channels),
      gain_delay(v.delay.data,nbands,channels),
      s_out(icf.fftlen/2+1,channels),
      bInvert(v.invert.data),
      lp1ltg(MHASignal::dupvec_chk(v.ltgtau.data,nbands),icf.srate/mha_min_1(icf.fragsize)),
      b_ltg(v.ltgcomp.data),
      staticgain(MHASignal::dupvec_chk(v.staticgain.data,nbands))
{
    if( channels != 2 )
        throw MHA_Error(__FILE__,__LINE__,
                        "Invalid number of channels %u (two channel input expected).",channels);
    if( v.tau_unit.data.get_index()==1 ){
        // tau measured in periods:
        std::vector<float> f_hz(v.f.get_f_hz());
        std::vector<float> periods(MHASignal::dupvec_chk(v.tau.data,nbands));
        for(uint32_t k=0;k<nbands;++k){
            lp1i.set_tau(k,periods[k]/f_hz[k]);
            lp1r.set_tau(k,periods[k]/f_hz[k]);
        }
    }
    std::vector<float> l_alpha(MHASignal::dupvec_chk(v.alpha.data,nbands));
    for(unsigned int k=0;k<nbands;k++){
        alpha(k,0) = l_alpha[k];
        staticgain[k] = pow(10.0,0.05*staticgain[k]);
    }
    if( v.mapping.data.size() != 2 )
        throw MHA_Error(__FILE__,__LINE__,"The mapping vector requires exact two entries (got %zu).",v.mapping.data.size());
    if( v.mapping.data[1] <= v.mapping.data[0] )
        throw MHA_Error(__FILE__,__LINE__,"Entries of mapping vector must be monotonically increasing (%g,%g)",
                        v.mapping.data[0],v.mapping.data[1]);
    c_scale = 1.0f/(v.mapping.data[1] - v.mapping.data[0]);
    c_min = v.mapping.data[0];
}

cohflt_if_t::cohflt_if_t(const algo_comm_t& ac,
                         const std::string& th,
                         const std::string& al)
    : MHAPlugin::plugin_t<cohflt_t>("Coherence filter",ac),
      vars(this),
      algo(al)
{
    patchbay.connect(&writeaccess,this,&cohflt_if_t::update);
}

void cohflt_if_t::prepare(mhaconfig_t& tf)
{
    if( tf.domain != MHA_SPECTRUM )
        throw MHA_ErrorMsg("Only spectral processing is supported.");
    update();
    poll_config();
    cfg->insert();
}

void cohflt_if_t::release()
{
}

mha_spec_t* cohflt_if_t::process(mha_spec_t* s)
{
    return poll_config()->process(s);
}

inline void getcipd(mha_complex_t& c,mha_real_t& a,const mha_complex_t& xl,const mha_complex_t& xr)
{
    c.re = xl.re*xr.re + xl.im*xr.im;
    c.im = xl.im*xr.re - xl.re*xr.im;
    a = abs(c);
    if( a > 0 )
        c /= a;
}

void cohflt_t::insert()
{
    coh_c.insert();
    coh_rlp.insert();
}

mha_spec_t* cohflt_t::process(mha_spec_t* s)
{
    CHECK_VAR(s);
    CHECK_EXPR(s->num_channels == channels);
    CHECK_EXPR(s->num_frames == mhaconfig_t::fftlen/2+1);
    mha_complex_t lcgl, lcgr, s_l, s_r;
    mha_real_t abs_cg, p, p2;
    for(unsigned int band=0;band<nbands;band++){
        if( avg_ipd ){
            // binaural phase ratio is calculated in each FFT bin, then
            // the ratio is averaged across frequency (power weighted).
            cg = mha_complex(0,0);
            for(unsigned int k=bin1(band);k<bin2(band);k++){
                s_l = ::value(s,k,0);
                s_r = ::value(s,k,1);
                s_l *= w(k,band);
                s_r *= w(k,band);
                p = abs2(s_l) + abs2(s_r);
                getcipd(lcgl, p2, s_l, s_r );
                lcgl *= p;
                cg += lcgl;
            }
        }else{
            // the complex phase is averaged amplitude weighted
            // (average spectrum), then the interaural phase ratio is
            // calculated on the average spectrum
            lcgl = mha_complex(0,0);
            lcgr = mha_complex(0,0);
            for(unsigned int k=bin1(band);k<bin2(band);k++){
                s_l = ::value(s,k,0);
                s_r = ::value(s,k,1);
                s_l *= w(k,band);
                s_r *= w(k,band);
                lcgl += s_l;
                lcgr += s_r;
            }
            getcipd(cg, p2, lcgl, lcgr );
        }
        abs_cg = abs(cg);
        if( abs_cg > 0 )
            cg /= abs_cg;
        coh_c.value(band,0) = cg;
        cg.re = lp1r(band,cg.re);
        cg.im = lp1i(band,cg.im);
        g = abs(cg);
        coh_rlp.value(band,0) = g;
        // mapping function:
        g = (g-c_min)*c_scale;
        if( g < 0.0f )
            g = 0.0f;
        if( g > 1.0f )
            g = 1.0f;
        // expotentiation for gain calculation:
        g = pow(g,alpha(band,0));
        if( limit < 0 ){
            if( g < 1e-10 )
                g = pow(10.0,0.05*limit);
            else
                g = pow(10.0,0.05*limit*atan(20*log10(g)/limit));
        }
        // optional invert filter:
        if( bInvert )
            g = 1.0f-g;
        if( b_ltg ){
            double gc = lp1ltg(band,20*log10(std::max(g,1e-10f)));
            g *= pow(10.0,-0.05*gc);
        }
        g *= staticgain[band];
        gain(band,0) = g;
        gain(band,1) = g;
    }
    apply_gains(&s_out,s,gain_delay.process(&gain));
    insert();
    return &s_out;
}

void cohflt_if_t::update()
{
    if( is_prepared() ){
        push_config(new cohflt_t(vars,input_cfg(),ac,algo));
    }
}

}

MHAPLUGIN_CALLBACKS(coherence,coherence::cohflt_if_t,spec,spec)
MHAPLUGIN_DOCUMENTATION\
(coherence,
 "spatial signal-enhancement dereverberation adaptive",
 ""
 )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
