// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2008 2009 2010 2013 2014 2015 2018 2019 2020 HörTech gGmbH
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
#include "mha_filter.hh"

#define DEBUG(x) std::cerr << __FILE__ << ":" << __LINE__ << " " #x "=" << x << std::endl

namespace fader_wave {

class level_adapt_t : public MHASignal::waveform_t
{
public:
    level_adapt_t(mhaconfig_t cf,mha_real_t adapt_len,std::vector<float> l_new_,std::vector<float> l_old_);
    void update_frame();
    std::vector<float> get_level() const {return l_new;};
    bool can_update() const {return pos==0;};
private:
    unsigned int ilen;
    unsigned int pos;
    MHAWindow::fun_t wnd;
    std::vector<float> l_new;
    std::vector<float> l_old;
};

level_adapt_t::level_adapt_t(mhaconfig_t cf,mha_real_t adapt_len,std::vector<float> l_new_,std::vector<float> l_old_)
    : MHASignal::waveform_t(cf.fragsize,cf.channels),
      ilen(std::max(1u,(unsigned int)(cf.srate*adapt_len))),
      pos(ilen-1),
      wnd(ilen,MHAWindow::hanning,0,1),
      l_new(l_new_),
      l_old(l_old_)
{
    if( l_new.size() != cf.channels )
        throw MHA_Error(__FILE__,__LINE__,"Invalid number of entries in new level vector (expected %u, got %zu).",
                        cf.channels,l_new.size());
    if( l_old.size() != cf.channels )
        throw MHA_Error(__FILE__,__LINE__,"Invalid number of entries in previous level vector (expected %u, got %zu).",
                        cf.channels,l_old.size());
}

void level_adapt_t::update_frame()
{
    unsigned int k, ch;
    for(k=0;k<num_frames;k++){
        for( ch=0;ch<num_channels;ch++ )
            value(k,ch) = wnd.buf[pos]*l_new[ch]+(1.0f-wnd.buf[pos])*l_old[ch];
        if( pos )
            pos--;
    }
}

typedef MHAPlugin::plugin_t<level_adapt_t> level_adaptor;

class fader_wave_if_t : public level_adaptor
{
public:
    fader_wave_if_t(algo_comm_t,const char*,const char*);
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
    void release();
private:
    void set_level();
    MHAParser::vfloat_t gain;
    MHAParser::float_t ramplen;
    MHAEvents::patchbay_t<fader_wave_if_t> patchbay;
    bool prepared;
};

fader_wave_if_t::fader_wave_if_t(algo_comm_t iac,const char*,const char*)
    : level_adaptor("Apply level",iac),
      gain("Gain (linear)","[1]"),
      ramplen("Length of hanning ramp at gain changes in seconds","0","[0,]"),
      prepared(false)
{
    insert_item("gain",&gain);
    insert_item("ramplen",&ramplen);
    patchbay.connect(&gain.writeaccess,this,&fader_wave_if_t::set_level);
}

void fader_wave_if_t::set_level()
{
    if( prepared ){
        if( level_adaptor::cfg )
            level_adaptor::push_config(new level_adapt_t(tftype,ramplen.data,gain.data,level_adaptor::cfg->get_level()));
        else
            level_adaptor::push_config(new level_adapt_t(tftype,ramplen.data,gain.data,std::vector<float>(tftype.channels,0.0f)));
    }
}

void fader_wave_if_t::prepare(mhaconfig_t& tf)
{
    if( tf.domain != MHA_WAVEFORM )
        throw MHA_ErrorMsg("Only waveform processing supported.");
    tftype = tf;
    prepared = true;
    set_level();
    level_adaptor::poll_config();
}

void fader_wave_if_t::release()
{
    prepared = false;
}

mha_wave_t* fader_wave_if_t::process(mha_wave_t* s)
{
    if( level_adaptor::cfg->can_update() )
        level_adaptor::poll_config();
    level_adaptor::cfg->update_frame();
    *s *= *level_adaptor::cfg;
    return s;
}

}

MHAPLUGIN_CALLBACKS(fader_wave,fader_wave::fader_wave_if_t,wave,wave)
MHAPLUGIN_DOCUMENTATION\
(fader_wave,
 "data-flow audio-channels cross-fade level-modification",
 "")

/*
 * Local Variables:
 * compile-command: "make"
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * coding: utf-8-unix
 * End:
 */
