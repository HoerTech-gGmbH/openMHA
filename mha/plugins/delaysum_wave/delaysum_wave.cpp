// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2017 2018 2019 2020 HörTech gGmbH
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

/** This namespace contains the delaysum plugin. */
namespace delaysum{
    /** Runtime configuration of the delaysum_wave plugin. Inherits from the already
     * present delay_t class. The constructor initializes and validates the
     * runtime configuration and forwards the delay vector to the delay_t
     * class. The process function first calls delay_t::process and then
     * multiplies every output channel with its weight and adds them into the
     * output channel.
     */
    class delaysum_wave_t : public MHASignal::delay_t  {
    public:
        /** Constructor of the runtime configuration.
         * @param nch Number of input channels.
         * @param fragsize Size of one input fragment in frames.
         * @param weights_ Vector of weights for each channel.
         * @param delays_ Vector of delays, one entry per channel.
         */
        delaysum_wave_t(unsigned int nch, unsigned int fragsize,
                   const std::vector<mha_real_t>& weights_,
                   const std::vector<int>& delays_);
        mha_wave_t* process(mha_wave_t*);
    private:
        ///Relative weights for each channel. Order is [chan0, chan1, ...]
        std::vector<mha_real_t> weights;
        ///Output waveform
        MHASignal::waveform_t out;
    };


    /** Interface class for the delaysum plugin.
     * This plugin allows to delay and
     * sum multiple input channels using individual delays
     * and weights. After each channel gets delayed it
     * is multiplied with the given weight and then
     * added to the single outout channel.
     */
    class delaysum_wave_if_t : public MHAPlugin::plugin_t<delaysum_wave_t> {
    public:
        delaysum_wave_if_t(const algo_comm_t&,const std::string&,const std::string&);
        mha_wave_t* process(mha_wave_t*);
        void prepare(mhaconfig_t&);
        void release();
    private:
        void update_cfg();
        
        /// Linear weights to be multiplied with the audio signal, one
        /// factor for each channel.  Order is [chan0, chan1, ...]
        MHAParser::vfloat_t weights;

        /// vector of channel-specific delays, in samples.
        MHAParser::vint_t delay;
        
        /// The patchbay to react to config changes.
        MHAEvents::patchbay_t<delaysum_wave_if_t> patchbay;
    };

    delaysum_wave_t::delaysum_wave_t(unsigned int nch,
                           unsigned int fragsize,
                           const std::vector<mha_real_t>& weights_,
                           const std::vector<int>& delays_)
        :MHASignal::delay_t(delays_,nch),
        weights(weights_),
        out(fragsize,1)
    {
        if( weights.size() != nch )
            throw MHA_Error(__FILE__,__LINE__,
                            "Invalid number of weights %zu (should equal the "
                            "number of input channels).",weights.size());
        if( delays_.size() != nch )
            throw MHA_Error(__FILE__,__LINE__,
                            "Invalid number of delays %zu (should equal the "
                            "number of input channels).",delays_.size());
    }


    mha_wave_t* delaysum_wave_t::process(mha_wave_t* signal)
    {
        signal=MHASignal::delay_t::process(signal);
        clear(out);
        for(unsigned int frame=0; frame<signal->num_frames;++frame){
            for(unsigned channel=0; channel<signal->num_channels;++channel){
                (::value(out,frame,0))+=
                    weights[channel]*(::value(signal,frame,channel));
            }
        }
        return &out;
    }

    delaysum_wave_if_t::delaysum_wave_if_t(
                                 const algo_comm_t& iac,
                                 const std::string&,const std::string&)
        :  MHAPlugin::plugin_t<delaysum_wave_t>("delay and sum plugin. Mixes all "
                                           "channels into a single output "
                                           "channel after applying channel-"
                                           "specific weights and delays.",iac),
        weights("weights of channels.  Each entry is multiplied to its\n"
                "respective channel.  Needs one entry per channel.","[1 1]","[,]"),
        delay("delay in number of frames. The nth channel is delayed\n"
              "by the number of frames found in the nth entry. ","[0 0]","[0,]")
    {
        insert_item("weights",&weights);
        insert_item("delay",&delay);
        patchbay.connect(&weights.writeaccess,this,&delaysum_wave_if_t::update_cfg);
        patchbay.connect(&delay.writeaccess,this,&delaysum_wave_if_t::update_cfg);
    }

    mha_wave_t* delaysum_wave_if_t::process(mha_wave_t* wave)
    {
        poll_config();
        return cfg->process(wave);
    }

    void delaysum_wave_if_t::prepare(mhaconfig_t& tfcfg)
    {
        if( tfcfg.domain != MHA_WAVEFORM )
            throw MHA_Error(__FILE__,__LINE__,
                            "delaysum: Only waveform processing is supported.");
        // change number of channels for output configuration
        tfcfg.channels = 1;
        // make sure that a valid runtime configuration exists
        update_cfg();
    }

    void delaysum_wave_if_t::update_cfg()
    {
        if( input_cfg().channels )
            push_config(new delaysum_wave_t(input_cfg().channels,
                                       input_cfg().fragsize,
                                       weights.data,
                                       delay.data));
    }

    void delaysum_wave_if_t::release(){
        /** Do nothing in release. */
    }
}
MHAPLUGIN_CALLBACKS(delaysum_wave,delaysum::delaysum_wave_if_t,wave,wave)
MHAPLUGIN_DOCUMENTATION\
(delaysum_wave,
 "spatial beamformer",
 "This plugin allows to delay and "
 "sum multiple input channels using individual "
 "delays and weights. After each channel is delayed "
 "it is multiplied with the given weight and then "
 "added to the single output channel. This plugin was formerly known as delaysum."
 )

    // Local Variables:
    // compile-command: "make"
    // c-basic-offset: 4
    // indent-tabs-mode: nil
    // coding: utf-8-unix
    // End:
