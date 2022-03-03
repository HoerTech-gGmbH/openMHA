// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2010 2013 2014 2015 2017 2018 2019 2020 HörTech gGmbH
// Copyright © 2021 HörTech gGmbH
// Copyright © 2022 Hörzentrum Oldenburg gGmbH
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
#include <memory>

/** Namespace containing all code for the ac2wave plugin */
namespace ac2wave {

/** ac2wave real-time configuration class */
class ac2wave_t {
public:
    /** Constructor of the real-time configuration
     * @param frames_ Number of frames in the input signal
     * @param channels_ Number of channels in the input signal 
     * @param ac_ Handle to AC space
     * @param name_ Name of the AC variable to be mixed in
     * @param gain_in_ Linear gain for the input signal
     * @param gain_ac_ Linear gain for the AC input
     * @param delay_in_ Delay, in fragments, for the input signal
     * @param delay_ac_ Delay, in fragments, for the AC input
     */
    ac2wave_t(unsigned int frames_,
              unsigned int channels_, 
              MHA_AC::algo_comm_t & ac_, 
              std::string name_,
              float gain_in_,
              float gain_ac_,
              unsigned int delay_in_,
              unsigned int delay_ac_);
    /** Process callback
     * @param s Pointer to input waveform
     * @returns Pointer to output waveform
     */
    mha_wave_t* process(mha_wave_t* s);
private:
    /// frames_ Number of frames in the input signal
    unsigned int frames;
    /// Number of channels in the input signal 
    unsigned int channels;
    /// AC input waveform
    mha_wave_t w;
    /// Handle to AC space
    MHA_AC::algo_comm_t & ac;
    /// Name of the input AC variable
    std::string name;
    /// Delay, in fragments, for the input signal
    MHASignal::delay_wave_t delay_in;
    /// Delay, in fragments, for the AC input
    MHASignal::delay_wave_t delay_ac;
    /// Linear gain for the input signal
    mha_real_t gain_in;
    /// Linear gain for the AC input
    mha_real_t gain_ac;
};

ac2wave_t::ac2wave_t(
    unsigned int frames_,
    unsigned int channels_, 
    MHA_AC::algo_comm_t & ac_, 
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

/** ac2wave plugin interface class */
class ac2wave_if_t : public MHAPlugin::plugin_t<ac2wave_t> {
public:
    /** Constructor.
     * @param iac Handle to the AC space
     * @param configured_name Assigned name of the plugin within the configuration tree
     */
    ac2wave_if_t(MHA_AC::algo_comm_t & iac,
                 const std::string & configured_name);
    /** process callback for spectral input */
    mha_wave_t* process(mha_spec_t*);
    /** process callback for waveform input
     * @param s input Pointer to input waveform
     * @returns Pointer to output waveform. Points to an internal buffer of s_in_delayed
     */
    mha_wave_t* process(mha_wave_t*);
    /** Prepare callback. Changes the output domain to waveform, using the
     * input dimensions in tf.
     * @param tf Input signal configuration
     */
    void prepare(mhaconfig_t& tf);
    /** Release callback */
    void release(){};
private:
    /** Update the real-time configuration after a configuration change */
    void update();
    /// AC variable name
    MHAParser::string_t name;
    /// Linear gain for main input signal
    MHAParser::float_t gain_in;
    /// Linear gain for AC input signal
    MHAParser::float_t gain_ac;
    /// Delay of main input signal in fragments
    MHAParser::int_t delay_in;
    /// Delay of AC input signal in fragments
    MHAParser::int_t delay_ac;
    /// Zero signal, used for Spectral inputs
    std::unique_ptr<MHASignal::waveform_t> zeros;
    /// Patchbay for connecting the configuration parsers to the update callback
    MHAEvents::patchbay_t<ac2wave_if_t> patchbay;
};

ac2wave_if_t::ac2wave_if_t(MHA_AC::algo_comm_t & iac, const std::string &)
    : MHAPlugin::plugin_t<ac2wave_t>(
        "Mix the main input signal with a waveform stored into AC\n"
        "variables. Main and AC signal can be attenuated or delayed\n"
        "by integer fragments. The AC variable and the input waveform have to\n"
        "have the same dimensions.\n\n"
        "Spectral input is discarded and replaced by a zero signal before"
        " the AC input is mixed in.\n\n",iac),
      name("AC variable name",""),
      gain_in("Linear gain for main input signal","0"),
      gain_ac("Linear gain for AC input signal","1"),
      delay_in("Delay of main input signal in fragments","0","[0,["),
      delay_ac("Delay of AC input signal in fragments","0","[0,[")
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
    return cfg->process(zeros.get());
}

mha_wave_t* ac2wave_if_t::process(mha_wave_t* s)
{
    poll_config();
    return cfg->process(s);
}

void ac2wave_if_t::update()
{
    if( is_prepared() )
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
    zeros = std::make_unique<MHASignal::waveform_t>(tf.fragsize,tf.channels);
    update();
}

} //namespace ac2wave
MHAPLUGIN_CALLBACKS(ac2wave,ac2wave::ac2wave_if_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(ac2wave,ac2wave::ac2wave_if_t,spec,wave)
MHAPLUGIN_DOCUMENTATION\
(ac2wave,
 "data-flow algorithm-communication",
 "The 'ac2wave' plugin mixes the input signal with a waveform stored in an AC variable."
 " The input waveform and the AC signal can be independently attenuated by a linear gain and/or"
 " delayed by integer fragments. The AC input signal and the main input signal need to have the same"
 " dimensions. In the case of spectral input, the last time-domain signal dimensions before the transformation"
 " are used."
 " Spectral input is discarded and replaced by a zero signal before the AC input is mixed in.\n\n")

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
