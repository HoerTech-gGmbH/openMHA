// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2020 2021 HörTech gGmbH
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

#include "trigger2lsl.hh"

using namespace lsl;
using namespace trigger2lsl;
trigger2lsl_if_t::trigger2lsl_if_t(algo_comm_t iac, const std::string & )
    : MHAPlugin::plugin_t<trigger2lsl_rt_t>("ac variable file recorder",iac)//,ac(iac)
{
    insert_member(rising_edge);
    insert_member(falling_edge);
    insert_member(threshold);
    insert_member(stream_name);
    insert_member(use_edge_position);
    insert_member(min_debounce);
    patchbay.connect(&rising_edge.writeaccess,this,&trigger2lsl_if_t::update);
    patchbay.connect(&falling_edge.writeaccess,this,&trigger2lsl_if_t::update);
    patchbay.connect(&threshold.writeaccess,this,&trigger2lsl_if_t::update);
    patchbay.connect(&stream_name.writeaccess,this,&trigger2lsl_if_t::update);
    patchbay.connect(&use_edge_position.writeaccess,this,&trigger2lsl_if_t::update);
    patchbay.connect(&min_debounce.writeaccess,this,&trigger2lsl_if_t::update);
}

template <class mha_signal_t> mha_signal_t* trigger2lsl_if_t::process(mha_signal_t* s)
{
    poll_config()->process(s);
    return s;
}

void trigger2lsl_if_t::prepare(mhaconfig_t&)
{
    update();
}

void trigger2lsl_if_t::release()
{
}

void trigger2lsl_if_t::update()
{
    if(is_prepared())
        push_config(new trigger2lsl_rt_t(stream_name.data,
                                         rising_edge.data,
                                         falling_edge.data,
                                         threshold.data,
                                         channel.data,
                                         input_cfg().srate,
                                         use_edge_position.data,
                                         min_debounce.data));
}

trigger2lsl_rt_t::trigger2lsl_rt_t(const std::string& stream_name_,
                                   const std::string& rising_edge_,
                                   const std::string& falling_edge_,
                                   mha_real_t threshold_,
                                   int channel_,
                                   mha_real_t sampling_rate_,
                                   bool use_edge_position_,
                                   int min_debounce_):
    stream(lsl::stream_info(stream_name_,
                            "Markers",
                            1,
                            lsl::IRREGULAR_RATE,
                            lsl::cf_string,
                            "id23443")),
    rising_edge(rising_edge_),
    falling_edge(falling_edge_),
    threshold(threshold_),
    channel(channel_),
    sampling_rate(sampling_rate_),
    use_edge_position(use_edge_position_),
    min_debounce(min_debounce_)
{
}

void trigger2lsl_rt_t::process(mha_wave_t* wave){
    auto t0=lsl_local_clock();
    for(unsigned fr=0;fr<wave->num_frames;++fr){
        if((state && value(*wave,channel,fr)>threshold) or
           (!state && value(*wave,channel,fr)<threshold))
           debounce_counter++;
        else
            debounce_counter=0;
        if(debounce_counter+1>min_debounce){
            if(use_edge_position)
                stream.push_sample(state ? &rising_edge : &falling_edge,
                                   t0 + fr/sampling_rate);
            else
                stream.push_sample(state ? &rising_edge : &falling_edge);
            state=!state;
            debounce_counter=0;
        }
    }
}

MHAPLUGIN_CALLBACKS(trigger2lsl,trigger2lsl_if_t,wave,wave)
MHAPLUGIN_DOCUMENTATION\
(trigger2lsl, "data-export network-communication lab-streaming-layer",
 "This plugin creates sends lsl string markers when an audio channel crosses a configurable threshold. "
 " It is able to detect rising and falling edges. On a rising edge, the string in \"rising\\_edge\" is sent,"
 " on a falling edge the string in \"falling\\_edge\" is sent. As a simple debouncing measure the respective"
 " marker is sent only after the threshold was crossed for a configurable number of consecutive samples. Optionally, the timestamp of the marker"
 " can be corrected by the position of the crossing within the audio block."
 )

/*
 * Local Variables:
 * compile-command: "make"
 * c-basic-offset: 4
 * End:
 */
