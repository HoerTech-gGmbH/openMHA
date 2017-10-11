// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2008 2010 2013 2014 2015 2017 HörTech gGmbH
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
#include <math.h>
#include <time.h>

class cfg_t {
public:
    cfg_t(mhaconfig_t chcfg,mha_real_t newlev,bool replace,mha_real_t len);
    inline void process(mha_wave_t*);
    inline void process(mha_spec_t*);
private:
    mha_real_t gain_wave_;
    mha_real_t gain_spec_;
    bool replace_;
    bool use_frozen_;
    MHASignal::waveform_t frozen_noise_;
    unsigned int pos;
};

cfg_t::cfg_t(mhaconfig_t chcfg,mha_real_t newlev,bool replace,mha_real_t len)
    : gain_wave_(2.0 * sqrt(3.0) * 2e-5 * pow(10.0,newlev/20.0)),
      gain_spec_(1.0f/16.0f * sqrt(3.0) * 2e-5 * pow(10.0,newlev/20.0)),
      replace_(replace),
      use_frozen_(len>0),
      frozen_noise_(std::max(1u,(unsigned int)(len*chcfg.srate)),chcfg.channels),
      pos(0)
{
    for(unsigned int k=0;k<size(frozen_noise_);k++)
        frozen_noise_.buf[k] = gain_wave_ * ((mha_real_t)rand() / (mha_real_t)RAND_MAX - 0.5);
}

void cfg_t::process(mha_wave_t* s)
{
    if( replace_ )
        clear(s);
    if( use_frozen_ ){
        MHA_assert_equal(s->num_channels, frozen_noise_.num_channels);
        for( unsigned int k=0; k < s->num_frames; k++ ){
            if( pos == frozen_noise_.num_frames )
                pos = 0;
            for( unsigned int ch=0; ch < s->num_channels; ch++ )
                value(s,k,ch) += frozen_noise_.value(k,ch);
            pos++;
        }
    }else{
        for( unsigned int k=0; k < size(s); k++){
            s->buf[k] += gain_wave_ * ((mha_real_t)rand() / (mha_real_t)RAND_MAX - 0.5);
        }
    }
}

void cfg_t::process(mha_spec_t* s)
{
    if( replace_ )
        clear(s);
    for( unsigned int k=0;k < size(s); k++){
        s->buf[k].re += gain_spec_ * ((mha_real_t)rand() / (mha_real_t)RAND_MAX - 0.5);
        s->buf[k].im += gain_spec_ * ((mha_real_t)rand() / (mha_real_t)RAND_MAX - 0.5);
    }
}

class noise_t : public MHAPlugin::plugin_t<cfg_t> {
public:
    noise_t(const algo_comm_t&,const std::string&,const std::string&);
    mha_wave_t* process(mha_wave_t*);
    mha_spec_t* process(mha_spec_t*);
    void prepare(mhaconfig_t&);
    void update_cfg();
private:
    MHAParser::float_t lev;
    MHAParser::kw_t mode;
    MHAParser::float_t frozennoise_length;
    MHAEvents::patchbay_t<noise_t> patchbay;
};


/********************************************************************/

void noise_t::update_cfg()
{
    if( is_prepared() )
        push_config(new cfg_t(input_cfg(),lev.data,mode.isval("replace"),frozennoise_length.data));
}

noise_t::noise_t(const algo_comm_t& iac,const std::string&,const std::string&)
    : MHAPlugin::plugin_t<cfg_t>(
        "white noise generator\n\n"
        "Waveform and spectral domain are supported. Please\n"
        "note that only in the waveform domain, real continuous\n"
        "white noise is created. In the spectral domain, some\n"
        "modulation and spectral shaping might occur.\n\n"
        ,iac),
      lev("noise RMS level in dB SPL","0"),
      mode("operation mode","add","[add replace]"),
      frozennoise_length("Length of frozen noise in s, or 0 for running noise.","0","[0,]")
{
    insert_member(lev);
    insert_member(mode);
    insert_member(frozennoise_length);
    patchbay.connect(&writeaccess,this,&noise_t::update_cfg);
    srand( time(NULL) );
    set_node_id("noise");
}

void noise_t::prepare(mhaconfig_t& tf)
{
    tftype = tf;
    update_cfg();
}

mha_wave_t* noise_t::process(mha_wave_t* s)
{
    poll_config()->process(s);
    return s;
}

mha_spec_t* noise_t::process(mha_spec_t* s)
{
    poll_config()->process(s);
    return s;
}

MHAPLUGIN_CALLBACKS(noise,noise_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(noise,noise_t,spec,spec)
MHAPLUGIN_DOCUMENTATION(noise,"generator","")

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
