// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2017 HörTech gGmbH
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


#include <stdio.h>
#include "mha_plugin.hh"

/**
   \brief Delay and sum algorithm.
 */
namespace delaysum{

/**
 * This plugin implements the delay and sum algorithm with \f$N_{ch}\f$ input 
 * channels and one output channel. If we denote with \f$s_{in}(n)\f$ and 
 * \f$s_{out}(n)\f$ the input and output signals at time \f$n\f$ respectively, 
 * then we have \f$s_{out}(n)=\sum_{i=1}^{N_{ch}}w_{i}s_{in}(n-\tau_i)\f$; 
 * \f$w_i\f$ are the assigned weights for each channel and \f$\tau_i\f$ are 
 * the respective channel delays.
 */

class delaysum_t {
public:
    delaysum_t(unsigned int,unsigned int, std::vector<mha_real_t>,std::vector<int>);
    mha_wave_t* process(mha_wave_t*);
private:
    // output signal
    MHASignal::waveform_t out;
    // vector of weights
    std::vector<mha_real_t> weights;
    // vector of delays
    std::vector<int> delay;
    // buffer of delayed input signal
    MHASignal::waveform_t in_buffer;
};

class delaysum_if_t : public MHAPlugin::plugin_t<delaysum_t> {
public:
    delaysum_if_t(const algo_comm_t&,const std::string&,const std::string&);
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
    void release();
private:
    void update_cfg();
    /** float vector variable of MHA-parser: channel weights */
    MHAParser::vfloat_t weights;
    /** int vector variable of MHA-parser: channel delays */
    MHAParser::vint_t delay;
    /** patch bay for connecting configuration parser 
       events with local member functions: */
    MHAEvents::patchbay_t<delaysum_if_t> patchbay;
    
};

delaysum_t::delaysum_t(unsigned int inumchannels_in,
                       unsigned int fragsize,
                       std::vector<mha_real_t> iweights,
                       std::vector<int> idelay)
    : out(fragsize,1), //initialize the output signal (only one channel)
      weights(iweights),
      delay(idelay),
      in_buffer(fragsize,inumchannels_in){
    if( weights.size() != inumchannels_in )
        throw MHA_Error(__FILE__,__LINE__,
                        "Invalid number of weights %u (should equal the number of input channels).",weights.size());
    if( inumchannels_in < 2 )
        throw MHA_Error(__FILE__,__LINE__,
                        "Invalid number of channels %u (at least two input channels expected).",channels);
    if( delay.size() != inumchannels_in )
        throw MHA_Error(__FILE__,__LINE__,
                        "Invalid number of delays %u (should equal the number of input channels).",delay.size());
}


mha_wave_t* delaysum_t::process(mha_wave_t* signal)
{
    for(unsigned int frame = 0; frame < signal->num_frames; frame++){
        // Clear buffer 
        value(&out,frame,0) = 0.0;
        for(unsigned int ch = 0; ch < signal->num_channels; ch++){
            if(frame < (unsigned)delay[ch]){
                // Add buffer value to signal
                value(&out,frame,0) += value(&in_buffer, frame, ch)*weights[ch];
                // update buffer
                value(&in_buffer, frame, ch) = value(signal, 
                    signal->num_frames - delay[ch] + frame, ch); 
            }
            else{
                // Add signal value
                value(&out,frame,0) += value(signal,frame-delay[ch],ch)*weights[ch];
            }            
        }
    }
    return &out;
}

delaysum_if_t::delaysum_if_t(
    const algo_comm_t& iac,
    const std::string&,const std::string&)
    :  MHAPlugin::plugin_t<delaysum_t>("delay and sum plugin ",iac),
      /* initialzing variable 'weights' with MHAParser::vfloat_t(char* name, .... ) */
      weights("weights of channels",
                "[1.0 1.0]", 
                "[0.0,["),
      /* initialzing variable 'delay' with MHAParser::int_t(char* name, .... ) */
      delay("delay in number of frames ","[0 0]","[0,["){
    
    /* Register variables to the configuration parser: */
    insert_item("weights",&weights);
    insert_item("delay",&delay);
    
    patchbay.connect(&weights.writeaccess,this,&delaysum_if_t::update_cfg);
    patchbay.connect(&delay.writeaccess,this,&delaysum_if_t::update_cfg);
}

mha_wave_t* delaysum_if_t::process(mha_wave_t* wave)
{
    poll_config();
    // cfg is a pointer to delaysum_t
    return cfg->process(wave);
}

void delaysum_if_t::prepare(mhaconfig_t& tfcfg)
{
    if( tfcfg.domain != MHA_WAVEFORM )
        throw MHA_Error(__FILE__,__LINE__,
                        "delaysum: Only waveform processing is supported.");
    // Check if any delay is larger than block size
    if((unsigned)*std::max_element(delay.data.begin(),delay.data.end()) > tfcfg.fragsize)
        throw MHA_Error(__FILE__,__LINE__,
                            "delaysum: Delay cannot exceed fragment size.");

    // change number of channels for output configuration
    tfcfg.channels = 1;
    
    /* make sure that a valid runtime configuration exists: */
    update_cfg();
}

void delaysum_if_t::update_cfg()
{
      if( input_cfg().channels )
        push_config(new delaysum_t(input_cfg().channels,input_cfg().fragsize,
                    weights.data,delay.data));
}

void delaysum_if_t::release(){
}
}
MHAPLUGIN_CALLBACKS(delaysum,delaysum::delaysum_if_t,wave,wave)
MHAPLUGIN_DOCUMENTATION(delaysum,"delay and sum ","")

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
