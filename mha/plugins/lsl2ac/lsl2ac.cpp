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

#include "lsl2ac.hh"
#include <algorithm>
using namespace std::string_literals;
lsl2ac::lsl2ac_t::lsl2ac_t(algo_comm_t iac, const std::string &)
    : MHAPlugin::plugin_t<lsl2ac::cfg_t>("Receive LSL streams and copy"
                                         " them to AC variables.",iac)
{
    //Nota bene: The configuration variables are not connected to the patchbay because
    // we either lock them anyway during prepare or they are not used within the context
    // of the cfg_t class.
    insert_member(streams);
    insert_member(activate);
    insert_member(overrun_behavior);
    insert_member(nchannels);
    insert_member(buffersize);
    insert_member(chunksize);
    insert_member(nsamples);
    insert_item("available_streams", &available_streams);
    patchbay.connect(&available_streams.prereadaccess,this,&lsl2ac_t::get_all_stream_names);
}

void lsl2ac::lsl2ac_t::prepare(mhaconfig_t&)
{
    update();
    setlock(true);
}

void lsl2ac::lsl2ac_t::release()
{
    setlock(false);
}

void lsl2ac::lsl2ac_t::process()
{
    poll_config();
    if(activate.data)
        cfg->process();
}

void lsl2ac::lsl2ac_t::update(){
    if(is_prepared()){
        auto c=new cfg_t(ac, static_cast<lsl2ac::overrun_behavior>(overrun_behavior.data.get_index()),
                         buffersize.data, chunksize.data, streams.data, nchannels.data, nsamples.data);
        push_config(c);
    }
}

void lsl2ac::lsl2ac_t::setlock(bool lock_){
    overrun_behavior.setlock(lock_);
    buffersize.setlock(lock_);
    chunksize.setlock(lock_);
    streams.setlock(lock_);
    nchannels.setlock(lock_);
    nsamples.setlock(lock_);
}

void lsl2ac::lsl2ac_t::get_all_stream_names()
{
    available_streams.data.clear();
    std::vector<lsl::stream_info> streams = lsl::resolve_streams();
    available_streams.data.resize(streams.size());
    for(const auto& istream : streams)
        available_streams.data.push_back(istream.name());
}

lsl2ac::cfg_t::cfg_t(const algo_comm_t& ac_,
                     overrun_behavior ob_,
                     int bufsize_,
                     int chunksize_,
                     const std::vector<std::string>& streamnames_,
                     int nchannels_,
                     int nsamples_)
{
    for(auto& name : streamnames_) {
        //Find all streams with matching name and take the first one, throw if none found
        auto matching_streams=lsl::resolve_stream("name",name,/*minimum=*/1,/*timeout=*/1.0);
        if(!matching_streams.size())
            throw MHA_Error(__FILE__,__LINE__,"No stream with name %s found!",name.c_str());

        switch(matching_streams[0].channel_format()){
        case lsl::channel_format_t::cf_float32:
            varlist.emplace(name,std::make_unique<save_var_t<mha_real_t>>(matching_streams[0],
                                                                          ac_,
                                                                          ob_,
                                                                          MHA_AC_MHAREAL,
                                                                          bufsize_,
                                                                          chunksize_,
                                                                          nchannels_,
                                                                          nsamples_));
            break;
        case lsl::channel_format_t::cf_double64:
            varlist.emplace(name,std::make_unique<save_var_t<double>>(matching_streams[0],
                                                                      ac_,
                                                                      ob_,
                                                                      MHA_AC_DOUBLE,
                                                                      bufsize_,
                                                                      chunksize_,
                                                                      nchannels_,
                                                                      nsamples_));
            break;
            // LSL takes care of type conversion from short integers to ints
        case lsl::channel_format_t::cf_int8:
        case lsl::channel_format_t::cf_int16:
        case lsl::channel_format_t::cf_int32:
            varlist.emplace(name,std::make_unique<save_var_t<int>>(matching_streams[0],
                                                                   ac_,
                                                                   ob_,
                                                                   MHA_AC_INT,
                                                                   bufsize_,
                                                                   chunksize_,
                                                                   nchannels_,
                                                                   nsamples_));
            break;
        case lsl::channel_format_t::cf_string:
            // char arrays and marker streams both use cf_string - distinguish by type meta data
            if(matching_streams[0].type().find("Marker"s) != std::string::npos)
                varlist.emplace(name,std::make_unique<save_var_t<std::string>>(matching_streams[0],
                                                                        ac_,
                                                                        ob_,
                                                                        bufsize_,
                                                                        chunksize_,
                                                                        nsamples_));
            else
                varlist.emplace(name,std::make_unique<save_var_t<char>>(matching_streams[0],
                                                                        ac_,
                                                                        ob_,
                                                                        MHA_AC_CHAR,
                                                                        bufsize_,
                                                                        chunksize_,
                                                                        nchannels_,
                                                                        nsamples_));
            break;
        case lsl::channel_format_t::cf_int64:
            throw MHA_Error(__FILE__,__LINE__,"Stream %s: Channel format int64 is not supported.",matching_streams[0].name().c_str());
        case lsl::channel_format_t::cf_undefined:
                            throw MHA_Error(__FILE__,__LINE__,"Stream %s:Can not store undefined channel format in AC variable",matching_streams[0].name().c_str());
        default:
                            throw MHA_Error(__FILE__,__LINE__,"Stream %s:Unknown lsl channel format: %i",matching_streams[0].name().c_str(),matching_streams[0].channel_format());
        }
    }
}

void lsl2ac::cfg_t::process(){
    for(auto& var : varlist){
        var.second->receive_frame();
    }
}

MHAPLUGIN_CALLBACKS(lsl2ac,lsl2ac::lsl2ac_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(lsl2ac,lsl2ac::lsl2ac_t,spec,spec)
MHAPLUGIN_DOCUMENTATION(
                        lsl2ac, "data-import network-communication",
                        " This plugin provides a mechanism to receive lsl streams and"
                        " convert them to ac variables."
                        " It currently only supports MHA\\_AC\\_FLOAT-type variables. Type "
                        "conversions from other"
                        " types of stream should be handled in the background."
                        "\n"
                        "This is a beta version of the plugin. It is probably real-time safe.\n"
                        " An LSL stream named NAME results in the following AC variables: NAME containing the data,"
                        " NAME\\_ts containing the time stamps, NAME\\_ts containing the offset between receiver and sender clocks,"
                        " and NAME\\_new containing the number of new samples per channel since the last process callback.\n "
                        " The size of the AC variables is configurable via the nchannels and "
                        "nsamples configuration variables."
                        " nchannels controls the stride of the AC variable and must be equal to the number of channels of the"
                        " AC variables or be left on default to accept"
                        " any number of channels of the LSL stream. nsamples corresponds to the number of samples per channel of the AC"
                        " variable. Leaving nsamples on default means that the AC variable will be resized according to the number of"
                        " samples received, up to a maximum of chunksize samples."
                        " If the size of the AC variable is fixed and there are less samples available in the LSL buffers than are needed"
                        " to fill the AC variable, the oldest samples are overwritten and the contents of the AC variable are reordered so"
                        " that the oldest samples come first."
                        " On overrun, i.e. there are more samples available than fit in the AC variable, the user can decide if all samples "
                        " but the newest"
                        " should be discarded or if the overrun should be ignored and only the oldest samples should be saved to AC,"
                        " leaving newer samples in the LSL buffers. Warning: If the overrun behavior is set to discard, the plugin pulls"
                        " new samples as long as samples are ready for pickup in the LSL buffers. If the sender is considerably faster than the receiver"
                        " this may cause the plugin to hang indefinitely."
                        " The buffer length and chunk size of the LSL inlet are configurable. For more details on the meaning of these"
                        " variables please consult the LSL documentation.\n"
                        " In the case of string type streams e.g. marker streams, only one entry is saved into the AC variable. The nchannels"
                        " configuration variable is ignored for string streams, marker streams with more than one channel are not supported."
                        " To nsamples configuration variable is used to determine the size of the character buffer used to store the received strings"
                        " Longer marker strings are cut off memory safe, they may however cause memory allocations within lsl, so the size should not be chosen"
                        " too small.\n"
                        " The configuration regarding the AC variable size and the LSL stream inlet applies plugin wide. To use per-stream"
                        " configuration this plugin must be instantiated multiple times.\n")
/*
 * Local variables:
 * c-basic-offset: 4
 * compile-command: "make"
 * End:
 */
