// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2008 2010 2014 2015 2017 HörTech gGmbH
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
                        "upsample: Got %d channels, expected %d.",
                        s->num_channels, cfg->num_channels);
    if( cfg->num_frames / ratio.data != s->num_frames )
        throw MHA_Error(__FILE__,__LINE__,
                        "upsample: Got %d frames, expected %d.",
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
MHAPLUGIN_DOCUMENTATION(upsample,"resample signalflow","This plugin performs upsampling by an integer factor n. \n "
                        " In this process the sampling rate and the fragment size is multiplied by n so that the total"
                        " number of process calls stays constant. \n The upsampling is performed by writing only "
                        " to every n-th frame of the output signal. \n An IIR filter can be employed to reduce aliasing.")

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
