// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2008 2010 2013 2014 2015 2017 2018 HörTech gGmbH
// Copyright © 2019 2020 HörTech gGmbH
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

#include "mha_parser.hh"
#include "mha_plugin.hh"
#include "mha_defs.h"
#include "mha_events.h"
#include "mha_signal.hh"

namespace gain {

class scaler_t : public MHASignal::waveform_t {
public:
    scaler_t(const unsigned int& channels,
             const MHAParser::vfloat_t& gains);
};

scaler_t::scaler_t(const unsigned int& channels,
                    const MHAParser::vfloat_t& gains)
    : MHASignal::waveform_t(1,channels)
{
    if( (gains.data.size() != channels) && (gains.data.size() != 1) )
        throw MHA_Error(__FILE__,__LINE__,
                        "The number of entries in the gain vector must be either %u"
                        " (one per channel) or 1 (same gains for all channels)", channels);
    if( gains.data.size() == 1 ){
        for(unsigned int ch=0;ch<num_channels;ch++)
            value(0,ch) = pow(10.0,0.05*gains.data[0]);
    }else{
        for(unsigned int ch=0;ch<num_channels;ch++)
            value(0,ch) = pow(10.0,0.05*gains.data[ch]);
    }
}

class gain_if_t : public MHAPlugin::plugin_t<scaler_t> 
{
public:
    gain_if_t(const algo_comm_t&,const std::string&,const std::string&);
    mha_wave_t* process(mha_wave_t*);
    mha_spec_t* process(mha_spec_t*);
    void prepare(mhaconfig_t&);
    void release();
private:
    void update_gain();
    void update_minmax();
    MHAEvents::patchbay_t<gain_if_t> patchbay;
    MHAParser::vfloat_t gains;
    MHAParser::float_t vmin;
    MHAParser::float_t vmax;
};

gain_if_t::gain_if_t(const algo_comm_t& iac,
                     const std::string&,
                     const std::string&)
    : MHAPlugin::plugin_t<scaler_t>("Gain plugin:\n\nApply a gain to each channel",iac),
      gains("Gain in dB","[0]","[-16,16]"),
      vmin("Minimal gain in dB","-16","[,0]"),
      vmax("Maximal gain in dB","16","[0,]")
{
    set_node_id("gain");
    insert_item("min",&vmin);
    insert_item("max",&vmax);
    insert_item("gains",&gains);
    patchbay.connect(&gains.writeaccess,this,&gain_if_t::update_gain);
    patchbay.connect(&vmin.writeaccess,this,&gain_if_t::update_minmax);
    patchbay.connect(&vmax.writeaccess,this,&gain_if_t::update_minmax);
}

void gain_if_t::prepare(mhaconfig_t& tf)
{
    tftype = tf;
    if( tftype.channels )
        update_gain();
}

void gain_if_t::release()
{
    tftype.channels = 0;
}

mha_wave_t* gain_if_t::process(mha_wave_t* s)
{
    poll_config();
    CHECK_EXPR(cfg->num_channels == s->num_channels);
    unsigned int k,ch;
    for(ch=0;ch<s->num_channels;ch++)
        for(k=0;k<s->num_frames;k++)
            value(s,k,ch) *= value(cfg,0,ch);
    return s;
}

mha_spec_t* gain_if_t::process(mha_spec_t* s)
{
    poll_config();
    CHECK_EXPR(cfg->num_channels == s->num_channels);
    unsigned int k,ch;
    for(ch=0;ch<s->num_channels;ch++)
        for(k=0;k<s->num_frames;k++)
            value(s,k,ch) *= value(cfg,0,ch);
    return s;
}

void gain_if_t::update_gain()
{
    if( tftype.channels )
        push_config(new scaler_t(tftype.channels,gains));
}

void gain_if_t::update_minmax()
{
    std::string s("[");
    s += vmin.parse("?val");
    s += ",";
    s += vmax.parse("?val");
    s += "]";
    gains.set_range(s);
    for(unsigned int ch=0;ch<gains.data.size();ch++){
        if( gains.data[ch] < vmin.data )
            gains.data[ch] = vmin.data;
        if( gains.data[ch] > vmax.data )
            gains.data[ch] = vmax.data;
    }
    update_gain();
}

}

MHAPLUGIN_CALLBACKS(gain,gain::gain_if_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(gain,gain::gain_if_t,spec,spec)
MHAPLUGIN_DOCUMENTATION\
(gain,
 "level-modification",
 "This plugin applies a configurable gain to each channel.\n"
 "The number of entries in the gain vector must be either one per channel or 1 (same gains for all channels)"
 "\n\n"
 "For security reasons, the gain is limited to the range given by"
 " \\texttt{min} and \\texttt{max} which are preconfigured to -16dB and +16dB,"
 " respectively.\n"
 "Maximum and minimum gains are themselves configurable and need to be adjusted"
 " before gains exceeding the range [-16,+16] can be set through variables"
 " \\texttt{gains}.\n\n")


// Local variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
