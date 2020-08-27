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

class ds_t : public MHAPlugin::plugin_t<MHASignal::waveform_t> {
public:
    ds_t(algo_comm_t,std::string,std::string);
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
    void release();
private:
    MHAParser::int_t ratio;
    MHAFilter::iir_filter_t antialias;
};

ds_t::ds_t(algo_comm_t iac,std::string,std::string)
    : MHAPlugin::plugin_t<MHASignal::waveform_t>("Downsampling by integer fractions",iac),
      ratio("downsampling ratio","3","[1,]")
{
    insert_item("ratio",&ratio);
    insert_item("antialias",&antialias);
}      

void ds_t::prepare(mhaconfig_t& cf)
{
    ratio.setlock(true);
    tftype = cf;
    unsigned int new_fragsize = cf.fragsize / ratio.data;
    if( new_fragsize * ratio.data != cf.fragsize )
        throw MHA_Error(__FILE__,__LINE__,
                        "The fragment size (%u) is not divisible by %d.",
                        cf.fragsize,ratio.data);
    push_config(new MHASignal::waveform_t(new_fragsize,cf.channels));
    cf.fragsize = new_fragsize;
    cf.srate /= ratio.data;
    antialias.resize(cf.channels);
}

void ds_t::release()
{
    ratio.setlock(false);
}

mha_wave_t* ds_t::process(mha_wave_t* s)
{
    poll_config();
    if( cfg->num_channels != s->num_channels )
        throw MHA_Error(__FILE__,__LINE__,
                        "downsample: Got %u channels, expected %u.",
                        s->num_channels, cfg->num_channels);
    if( cfg->num_frames * ratio.data != s->num_frames )
        throw MHA_Error(__FILE__,__LINE__,
                        "downsample: Got %u frames, expected %u.",
                        s->num_frames, cfg->num_frames * ratio.data );
    antialias.filter(s,s);
    unsigned int ch, fr;
    for(ch=0;ch<cfg->num_channels;ch++)
        for(fr=0;fr<cfg->num_frames;fr++)
            value(cfg,fr,ch) = value(s,ratio.data*fr,ch);
    return cfg;
}

MHAPLUGIN_CALLBACKS(downsample,ds_t,wave,wave)
MHAPLUGIN_DOCUMENTATION(
downsample,                 // The name of this plugin.
"signal-transformation filter",// Categories of this plugin. Main category first
"This plugin performs downsampling by an integer factor named \\texttt{ratio}."
" The input fragment size needs to be divisible by \\texttt{ratio}."
"\n\n"
" As result of the downsammpling, the output signal has a lower sampling rate"
" ($srate$) as well as a smaller fragment size ($fragsize$) with respect to"
" the input signal of the \\texttt{downsample} plugin"
" (both are divided by the downsampling factor \\texttt{ratio})."
"\n\n"
" The signal duration ($T_{signal}$) of the audio blocks processed in each"
" invocation of the \\texttt{process} callbacks of \\mha plugins is"
" \\[T_{signal} = \\frac{fragsize}{srate}"
"               = \\frac{fragsize / ratio}{srate / ratio}\\] and is not changed"
" by the \\texttt{downsample} plugin. The total"
" number of invocations of the process method is not modified for"
" downstream plugins by the downsampling."
"\n\n"
" The downsampling is performed by copying only every n-th audio sample of the"
" low-pass filtered input signal over to the output signal."
" A low-pass filter is required to reduce aliasing in the output signal and"
" can be configured through the \\texttt{antialias} configuration setting.")

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
