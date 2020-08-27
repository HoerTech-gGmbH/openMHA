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

#include "mha_plugin.hh"
#include "mha_signal.hh"
#include "mha_defs.h"

class combc_t {
public:
    combc_t(algo_comm_t ac,mhaconfig_t cfg_input,mhaconfig_t cfg_output,std::vector<float> channel_gains,const std::string& element_gain_name, bool interleaved);
    mha_wave_t* process(mha_wave_t* s);
    mha_spec_t* process(mha_spec_t* s);
private:
    algo_comm_t ac_;
    bool interleaved_;
    unsigned int nbands;
    MHASignal::waveform_t w_out;
    MHASignal::spectrum_t s_out;
    std::vector<mha_real_t> channel_gains_;
    std::string element_gain_name_;
};


combc_t::combc_t(algo_comm_t ac,mhaconfig_t cfg_input,mhaconfig_t cfg_output,std::vector<float> channel_gains,const std::string& element_gain_name,bool interleaved)
    : ac_(ac),
      interleaved_(interleaved),
      nbands(cfg_input.channels/cfg_output.channels),
      w_out(cfg_input.fragsize,cfg_output.channels),
      s_out(cfg_input.fftlen/2+1,cfg_output.channels),
      channel_gains_(channel_gains),
      element_gain_name_(element_gain_name)
{
    if( cfg_output.channels * nbands != cfg_input.channels )
        throw MHA_Error(__FILE__,__LINE__,"Invalid channel settings:\n%u bands, %u output channels, %u input channels",
                        nbands,cfg_output.channels,cfg_input.channels);
    if( channel_gains_.size() ){
        if( channel_gains_.size() != cfg_input.channels )
            throw MHA_Error(__FILE__,__LINE__,
                            "Gain vector contains %zu entries, expected empty vector or %u entries.",
                            channel_gains_.size(),cfg_input.channels);
    }else{
        channel_gains_ = std::vector<mha_real_t>(cfg_input.channels,1.0f);
    }
}

mha_wave_t* combc_t::process(mha_wave_t* s)
{
    unsigned int k, ch, kband;
    mha_real_t val;
    clear(w_out);
    if( element_gain_name_.size() ){
        mha_wave_t elem_gain = MHA_AC::get_var_waveform(ac_,element_gain_name_);
        *s *= elem_gain;
    }
    if( interleaved_ ){
        for(k=0; k<s->num_frames; k++){
            for(ch=0; ch<w_out.num_channels; ch++){
                for(kband=0; kband<nbands; kband++){
                    val = value(s,k,ch+w_out.num_channels*kband);
                    val *= channel_gains_[ch+w_out.num_channels*kband];
                    w_out.value(k,ch) += val;
                }
            }
        }
    }else{
        for(k=0; k<s->num_frames; k++){
            for(ch=0; ch<w_out.num_channels; ch++){
                for(kband=0; kband<nbands; kband++){
                    val = value(s,k,ch*nbands+kband);
                    val *= channel_gains_[ch*nbands+kband];
                    w_out.value(k,ch) += val;
                }
            }
        }
    }
    return &w_out;
}

mha_spec_t* combc_t::process(mha_spec_t* s)
{
    unsigned int ch, k, kband;
    mha_complex_t val;
    clear(s_out);
    for(k=0; k<s->num_frames; k++){
        for(ch=0; ch<s_out.num_channels; ch++){
            for(kband=0; kband<nbands; kband++){
                val = value(s,k,ch*nbands+kband);
                val *= channel_gains_[ch*nbands+kband];
                s_out.value(k,ch) += val;
            }
        }
    }
    return &s_out;
}


//***************************************************************
//
//                     Interface
//
//***************************************************************

class combc_if_t : public MHAPlugin::plugin_t<combc_t> {
public:
    combc_if_t(const algo_comm_t&,
               const std::string&,
               const std::string&);
    void prepare(mhaconfig_t&);
    mha_wave_t* process(mha_wave_t*);
    mha_spec_t* process(mha_spec_t*);
private:
    MHAParser::int_t outchannels;
    MHAParser::bool_t interleaved;
    MHAParser::string_t channel_gain_name;
    MHAParser::string_t element_gain_name;
};

combc_if_t::combc_if_t(const algo_comm_t& iac,
                       const std::string&,
                       const std::string&)
    : MHAPlugin::plugin_t<combc_t>("Channel combiner",iac),
      outchannels("Number of output channels","1","[1,]"),
      interleaved("Input signal has interleaved channel order?","no"),
      channel_gain_name("Name of channel gain AC variable (looked up during prepare, can be empty)",""),
      element_gain_name("Name of element wise gain AC variable (looked up during waveform process, can be empty)","")
{
    insert_member(outchannels);
    insert_member(interleaved);
    insert_member(channel_gain_name);
    insert_member(element_gain_name);
}

void combc_if_t::prepare(mhaconfig_t& chcfg)
{
    chcfg.channels = outchannels.data;
    std::vector<mha_real_t> channel_gains;
    if( channel_gain_name.data.size() )
        channel_gains = std_vector_float(MHA_AC::get_var_waveform(ac,channel_gain_name.data));
    push_config(new combc_t(ac,input_cfg(),chcfg,channel_gains,element_gain_name.data,interleaved.data));
}

mha_wave_t* combc_if_t::process(mha_wave_t* s)
{
    return poll_config()->process(s);
}

mha_spec_t* combc_if_t::process(mha_spec_t* s)
{
    return poll_config()->process(s);
}

MHAPLUGIN_CALLBACKS(combinechannels,combc_if_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(combinechannels,combc_if_t,spec,spec)
MHAPLUGIN_DOCUMENTATION\
(combinechannels,
 "data-flow audio-channels filterbank",
 "Several filterbank bands can be combined into one or more output"
 " channels by summing-up the input channels. This plugin is intended as a"
 " filter resynthesis of linear-phase filter banks.\n\nThe input signal"
 " is by default expected to have a non-interleaved channel order, i.e., first all"
 " bands of first output channel, then all bands of second channel, etc. This behaviour"
 " can be controlled by the \"interleaved\" configuration variable.\n"
 "It is also possible to apply independent channel-wise and element-wise gains from AC variables"
 " to the signal before summation. This can be done by setting the configuration variables"
 " \"element\\_gain\\_name\" and \"channel\\_gain\\_name\" variables.")

// Local Variables:
// compile-command: "make"
// coding: utf-8-unix
// indent-tabs-mode: nil
// c-basic-offset: 4
// End:
