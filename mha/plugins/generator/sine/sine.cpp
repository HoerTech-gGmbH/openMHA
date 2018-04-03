// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2009 2010 2013 2014 2015 HörTech gGmbH
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

struct sine_cfg_t {
    double phase_increment_div_2pi;
    double amplitude;
    int    mix;
    const std::vector<int> channels;

    sine_cfg_t(double sampling_rate,
               mha_real_t frequency, 
               mha_real_t newlev,
               int        _mix,
               const std::vector<int> & _channels)
        : phase_increment_div_2pi(frequency / sampling_rate),
          amplitude(2e-5 * pow(10.0, newlev / 20.0) * sqrt(2)),
          mix(_mix),
          channels(_channels)
    {}
};

class sine_t : public MHAPlugin::plugin_t<sine_cfg_t> {
public:
    sine_t(
           const algo_comm_t&,
           const std::string& chain_name,
           const std::string& algo_name);
    ~sine_t();
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
private:
    void update_cfg();
    MHAParser::float_t lev;
    MHAParser::float_t frequency;
    MHAParser::kw_t mode;
    MHAParser::vint_t channels;
    double phase_div_2pi;
    MHAEvents::patchbay_t<sine_t> patchbay;
};

/********************************************************************/

using MHAParser::StrCnv::val2str;

void sine_t::update_cfg()
{
    sine_cfg_t * c = new sine_cfg_t(tftype.srate, frequency.data, lev.data,
                                    mode.data.get_index(), channels.data);
    push_config(c);
}

sine_t::sine_t(const algo_comm_t& iac,
               const std::string& chain_name,
               const std::string& algo_name)
    : MHAPlugin::plugin_t<sine_cfg_t>("Sine wave generator.",iac),
      lev("sine RMS level in dB SPL FF","0"),
      frequency("Frequency in Hz", "0","[0,["),
      mode("Replace input signal with tone or mix tone into input signal", 
           "replace", "[replace mix]"),
      channels("List of audio channels to feed with tone "\
               "(all other audio channels are not affected)", "[]")
{
    insert_item("lev",&lev);
    insert_item("f", &frequency);
    insert_item("mode", &mode);
    insert_item("channels", &channels);
    patchbay.connect(&lev.writeaccess,this,&sine_t::update_cfg);
    patchbay.connect(&frequency.writeaccess,this,&sine_t::update_cfg);
    patchbay.connect(&mode.writeaccess,this,&sine_t::update_cfg);
    patchbay.connect(&channels.writeaccess,this,&sine_t::update_cfg);
}

sine_t::~sine_t()
{
}

void sine_t::prepare(mhaconfig_t& tf)
{
    if( tf.domain != MHA_WAVEFORM )
        throw MHA_ErrorMsg("sine: Only waveform processing is supported.");
    channels.set_range("[0," + val2str(int(tf.channels)) + "[");
    tftype = tf;
    update_cfg();
}

mha_wave_t* sine_t::process(mha_wave_t* s)
{
    mha_real_t old_amplitude = cfg ? cfg->amplitude : 0;
    poll_config();
    // if frequency is 0, don't generate sound
    if (cfg->phase_increment_div_2pi == 0)
        this->phase_div_2pi = 0;
    std::vector<int>::const_iterator ch_begin = cfg->channels.begin();
    std::vector<int>::const_iterator ch_end = cfg->channels.end();
    std::vector<int>::const_iterator ch;
    for (unsigned frame = 0; frame < s->num_frames; ++frame) {
        mha_real_t amplitude = old_amplitude +
            (cfg->amplitude - old_amplitude) * frame / s->num_frames;
        this->phase_div_2pi += cfg->phase_increment_div_2pi;
        this->phase_div_2pi -= floor(this->phase_div_2pi);
        mha_real_t tone_sample = amplitude * sin(2 * M_PI * phase_div_2pi);
        for (ch = ch_begin; ch != ch_end; ++ch) {
            (::value(s, frame, *ch) *= cfg->mix) += tone_sample;
        }
    }
    return s;
}

MHAPLUGIN_CALLBACKS(sine,sine_t,wave,wave)
    MHAPLUGIN_DOCUMENTATION(sine,"generator","")

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
