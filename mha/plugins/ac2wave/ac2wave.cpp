// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2010 2013 2014 2015 2017 2018 2019 2020 HörTech gGmbH
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
#include "mha_events.h"

class ac2wave_t {
public:
    ac2wave_t(unsigned int frames_,
              unsigned int channels_, 
              algo_comm_t ac_, 
              std::string name_,
              float gain_in_,
              float gain_ac_,
              unsigned int delay_in_,
              unsigned int delay_ac_);
    mha_wave_t* process(mha_wave_t*);
private:
    unsigned int frames;
    unsigned int channels;
    mha_wave_t w;
    algo_comm_t ac;
    std::string name;
    MHASignal::delay_wave_t delay_in;
    MHASignal::delay_wave_t delay_ac;
    mha_real_t gain_in;
    mha_real_t gain_ac;
};

ac2wave_t::ac2wave_t(
    unsigned int frames_,
    unsigned int channels_, 
    algo_comm_t ac_, 
    std::string name_,
    float gain_in_,
    float gain_ac_,
    unsigned int delay_in_,
    unsigned int delay_ac_)
    : frames(frames_),
      channels(channels_),
      ac(ac_),
      name(name_),
      delay_in(delay_in_,frames,channels),
      delay_ac(delay_ac_,frames,channels),
      gain_in(gain_in_),
      gain_ac(gain_ac_)
{
}

mha_wave_t* ac2wave_t::process(mha_wave_t* s)
{
    w = MHA_AC::get_var_waveform(ac,name);
    if( w.num_frames != frames )
        throw MHA_Error(__FILE__,__LINE__,
                        "Mismatching signal dimension (ac: %u frames, expected %u).",
                        w.num_frames,frames);
    if( w.num_channels != channels )
        throw MHA_Error(__FILE__,__LINE__,
                        "Mismatching signal dimension (ac: %u channels, expected %u).",
                        w.num_channels,channels);
    mha_wave_t* s_in_delayed;
    mha_wave_t* s_ac_delayed;
    s_in_delayed = delay_in.process(s);
    s_ac_delayed = delay_ac.process(&w);
    *s_in_delayed *= gain_in;
    *s_ac_delayed *= gain_ac;
    *s_in_delayed += *s_ac_delayed;
    return s_in_delayed;
}

class ac2wave_if_t : public MHAPlugin::plugin_t<ac2wave_t> {
public:
    ac2wave_if_t(const algo_comm_t&,const std::string&,const std::string&);
    mha_wave_t* process(mha_spec_t*);
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
    void release();
private:
    void update();
    MHAParser::string_t name;
    MHAParser::float_t gain_in;
    MHAParser::float_t gain_ac;
    MHAParser::int_t delay_in;
    MHAParser::int_t delay_ac;
    MHASignal::waveform_t* zeros;
    bool prepared;
    MHAEvents::patchbay_t<ac2wave_if_t> patchbay;
};

ac2wave_if_t::ac2wave_if_t(const algo_comm_t& iac,
                           const std::string& ith,
                           const std::string& ial)
    : MHAPlugin::plugin_t<ac2wave_t>(
        "Mix the main input signal with a waveform stored into AC\n"
        "variables. Main and AC signal can be attenuated or delayed\n"
        "by integer fragments. The AC variable and the input waveform have to\n"
        "have the same dimensions.\n\n"
        "Spectral input is discarded and replaced by a zero signal.\n\n",iac),
      name("AC variable name",""),
      gain_in("Linear gain for main input signal","0"),
      gain_ac("Linear gain for AC input signal","1"),
      delay_in("Delay of main input signal in fragments","0","[0,["),
      delay_ac("Delay of AC input signal in fragments","0","[0,["),
      zeros(NULL),
      prepared(false)
{
    insert_item("name",&name);
    insert_item("gain_in",&gain_in);
    insert_item("gain_ac",&gain_ac);
    insert_item("delay_in",&delay_in);
    insert_item("delay_ac",&delay_ac);
    patchbay.connect(&writeaccess,this,&ac2wave_if_t::update);
}

mha_wave_t* ac2wave_if_t::process(mha_spec_t*)
{
    poll_config();
    return cfg->process(zeros);
}

mha_wave_t* ac2wave_if_t::process(mha_wave_t* s)
{
    poll_config();
    return cfg->process(s);
}

void ac2wave_if_t::update()
{
    if( prepared )
        push_config(new ac2wave_t(tftype.fragsize,
                                  tftype.channels,
                                  ac,name.data,
                                  gain_in.data,
                                  gain_ac.data,
                                  delay_in.data,
                                  delay_ac.data));
}

void ac2wave_if_t::prepare(mhaconfig_t& tf)
{
    tf.domain = MHA_WAVEFORM;
    tftype = tf;
    zeros = new MHASignal::waveform_t(tf.fragsize,tf.channels);
    prepared = true;
    update();
}

void ac2wave_if_t::release()
{
    prepared = false;
    delete zeros;
}

MHAPLUGIN_CALLBACKS(ac2wave,ac2wave_if_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(ac2wave,ac2wave_if_t,spec,wave)
MHAPLUGIN_DOCUMENTATION\
(ac2wave,
 "data-flow algorithm-communication",
 "")

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
