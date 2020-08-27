// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2008 2010 2013 2014 2015 2017 2018 2019 2020 HörTech gGmbH
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

class us_t : public MHAPlugin::plugin_t<MHASignal::waveform_t> {
public:
    us_t(algo_comm_t,std::string,std::string);
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
    void release();
private:
    MHAParser::int_t ratio;
    MHAFilter::iir_filter_t antialias;
};

us_t::us_t(algo_comm_t iac,std::string,std::string)
    : MHAPlugin::plugin_t<MHASignal::waveform_t>("Upsampling by integer fractions",iac),
      ratio("upsampling ratio","3","[1,]")
{
    insert_item("ratio",&ratio);
    insert_item("antialias",&antialias);
}      

void us_t::prepare(mhaconfig_t& cf)
{
    ratio.setlock(true);
    tftype = cf;
    unsigned int new_fragsize = cf.fragsize * ratio.data;
    push_config(new MHASignal::waveform_t(new_fragsize,cf.channels));
    cf.fragsize = new_fragsize;
    cf.srate *= ratio.data;
    antialias.resize(cf.channels);
}

void us_t::release()
{
    ratio.setlock(false);
}

mha_wave_t* us_t::process(mha_wave_t* s)
{
    poll_config();
    if( cfg->num_channels != s->num_channels )
        throw MHA_Error(__FILE__,__LINE__,
                        "upsample: Got %u channels, expected %u.",
                        s->num_channels, cfg->num_channels);
    if( cfg->num_frames / ratio.data != s->num_frames )
        throw MHA_Error(__FILE__,__LINE__,
                        "upsample: Got %u frames, expected %u.",
                        s->num_frames, cfg->num_frames / ratio.data );
    unsigned int ch, fr;
    clear(cfg);
    for(ch=0;ch<s->num_channels;ch++)
        for(fr=0;fr<s->num_frames;fr++)
            value(cfg,fr * ratio.data,ch) = ratio.data * value(s,fr,ch);
    antialias.filter(cfg,cfg);
    return cfg;
}

MHAPLUGIN_CALLBACKS(upsample,us_t,wave,wave)
MHAPLUGIN_DOCUMENTATION(
upsample,                     // The name of this plugin.
"signal-transformation filter", // Categories of this plugin. Main category 1st.
"This plugin performs upsampling by an integer factor named \\texttt{ratio}."
"\n\n"
" As result of the upsampling, the output signal has a higher sampling rate"
" ($srate$) as well as a larger fragment size ($fragsize$) with respect to"
" the input signal of the \\texttt{upsample} plugin"
" (both are multiplied by the upsampling factor \\texttt{ratio})."
"\n\n"
" The signal duration ($T_{signal}$) of the audio blocks processed in each"
" invocation of the \\texttt{process} callbacks of \\mha plugins"
" \\[T_{signal} = \\frac{fragsize}{srate}"
"               = \\frac{fragsize \\cdot ratio}{srate \\cdot ratio}\\]"
" is not changed by the \\texttt{upsample} plugin so that the total"
" number of invocations of the process method is not modified for"
" downstream plugins by the upsampling."
"\n\n"
" The upsampling is performed by spreading consecutive input audio samples"
" to only every n-th sample of the output signal while setting"
" the output samples in between consecutive input samples to value 0. "
" A low-pass filter is required to reduce aliasing in the output signal and"
" can be configured through the \\texttt{antialias} configuration setting.")

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
