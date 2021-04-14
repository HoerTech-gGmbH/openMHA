// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2020 2021 HörTech gGmbH
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

// This is a function try block. Really ugly syntax but the only way to handle exceptions
// in the ctor initializer list. See http://www.gotw.ca/gotw/066.htm
lsl2ac::save_var_t::save_var_t(const lsl::stream_info &info_,
                               const algo_comm_t &ac_,
                               overrun_behavior ob_, int buflen_,
                               int chunksize_, int nchannels_,
                               int nsamples_) try
    : stream(info_, buflen_, chunksize_),
      ac(ac_),
      ts_name(info_.name() + "_ts"),
      tc_name(info_.name() + "_tc"),
      new_name(info_.name() + "_new"),
      tic(std::chrono::steady_clock::now()),
      ob(ob_),
      name(info_.name()),
      nchannels(nchannels_), /* Conversion int->int32_t is okay b/c parser only accepts values that fit*/
      nsamples(nsamples_),   /* Parser only accepts values>0, so no problems b/c of signedness */
      chunksize(chunksize_), /* Parser only accepts values>0, so no problems b/c of signedness */
      bufsize(0),
      n_new_samples(0)
{
  if (nchannels != 0 && nchannels != info_.channel_count())
    throw MHA_Error(__FILE__, __LINE__,
                    "Expected %i channels in lsl stream, got %zu", nchannels,
                    nsamples);
  // channel_count will not change, we can set it and forget about it
  if (nchannels == 0)
    nchannels = info_.channel_count();
  // Precedence for channel bufsize=maximum number of elements in AC variable:
  // 1. nsamples*nchannels (fixed size)
  // 2. chunksize*nchannels (maximum number of samples that can arrive simultaneously)
  // 3. nchannels (can not go smaller)
  if(nsamples)
      bufsize = nsamples * nchannels;
  else if(chunksize)
      bufsize = chunksize*nchannels;
  else
      bufsize=nchannels;
  buf.resize(bufsize);
  std::fill(buf.begin(), buf.end(), 0.0f);
  ts_buf.resize(bufsize / nchannels);
  std::fill(ts_buf.begin(),ts_buf.end(),0.0);
  stream.open_stream();
  // According to lsl doc the first call to time_correction takes several ms,
  // subsequent calls are 'instant'. Calling it here to avoid blocking later.
  tc = stream.time_correction();
  cv.stride = info_.channel_count();
  cv.num_entries = info_.channel_count() * nsamples; // Not problematic if zero initially - will be reset on first pull
  cv.data_type = MHA_AC_FLOAT;
  cv.data = buf.data();
  ac.insert_var(ac.handle, info_.name().c_str(), cv);
  ts.stride = 1;
  ts.num_entries = nsamples;
  ts.data_type = MHA_AC_DOUBLE;
  ts.data = ts_buf.data();
  insert_vars();
} catch (MHA_Error &e) {
  // The framework can handle MHA_Errors. Just re-throw
  throw;
} catch (std::exception &e) {
  // LSL does not document its exceptions very well, but all exceptions I
  // encountered inherit from std::exception Rethrow as MHA_Error
  throw MHA_Error(__FILE__, __LINE__, "Could not subscribe to stream %s: %s",
                  info_.name().c_str(), e.what());
} catch (...) {
  // No matter what happened before, throw an MHA_Error so that the framework
  // can handle that
  throw MHA_Error(__FILE__, __LINE__,
                  "Could not subscribe to stream %s: Unknown error",
                  info_.name().c_str());
}

lsl::stream_info lsl2ac::save_var_t::info() {
    return stream.info();
}

void lsl2ac::save_var_t::receive_frame()  {
    if(skip){
        return;
    }
    else {
        n_new_samples=0;
        if (ob == overrun_behavior::Ignore) {
            pull_samples_ignore();
        } else if (ob == overrun_behavior::Discard) {
            pull_samples_discard();
        } else {
            throw MHA_Error(__FILE__, __LINE__,
                            "Bug: Unknown overrun behavior %i",
                            static_cast<int>(ob));
        }
        get_time_correction();
        insert_vars();
    }
}

void lsl2ac::save_var_t::pull_samples_ignore() {
    try{
    std::size_t n = 0;
    n = stream.pull_chunk_multiplexed(buf.data(), ts_buf.data(), bufsize,
                                      bufsize / nchannels,
                                      /*timeout =*/0.0);
    if (n<bufsize and nsamples != 0) {
        std::rotate(buf.begin(), buf.begin() + n, buf.end());
        std::rotate(ts_buf.begin(), ts_buf.begin() + n / nchannels, ts_buf.end());
    } else {
        cv.num_entries = n;
        ts.num_entries = n / nchannels;
    }
    n_new_samples = n / nchannels;
    }
    // If the stream is recoverable, lsl does not throw, instead tries to
    // recover.
    // Handle any other exception as a permanent underrun
    catch (...) {
      n_new_samples = 0;
      stream.close_stream();
      skip = true;
    }
}

void lsl2ac::save_var_t::pull_samples_discard() {
    try{
        std::size_t n = 0 ;
    do {
        n = stream.pull_chunk_multiplexed(buf.data(), ts_buf.data(), bufsize,
                                          bufsize / nchannels,
                                          /*timeout =*/0.0);
        if (n < bufsize and nsamples != 0) {
            std::rotate(buf.begin(), buf.begin() + n, buf.end());
            std::rotate(ts_buf.begin(), ts_buf.begin() + n / nchannels, ts_buf.end());
        } else {
            cv.num_entries = n;
            ts.num_entries = n / nchannels;
        }
        n_new_samples += n / nchannels;
    } while (stream.samples_available());
    }
    // If the stream is recoverable, lsl does not throw, instead tries to
    // recover.
    // Handle any other exception as a permanent underrun
    catch (...) {
      n_new_samples = 0;
      stream.close_stream();
      skip = true;
    }
}

void lsl2ac::save_var_t::get_time_correction() {
    auto toc=std::chrono::steady_clock::now();
    if(toc-tic>std::chrono::seconds(5)){
        tc=stream.time_correction();
        tic=toc;
    }
}

void lsl2ac::save_var_t::insert_vars(){
    ac.insert_var(ac.handle,name.c_str(),cv);
    ac.insert_var(ac.handle,ts_name.c_str(), ts);
    ac.insert_var_double(ac.handle,tc_name.c_str(), &tc);
    ac.insert_var_int(ac.handle, new_name.c_str(), &n_new_samples);
}

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
        varlist.emplace(std::make_pair(name,std::make_unique<save_var_t>(matching_streams[0],
                                                                         ac_,
                                                                         ob_,
                                                                         bufsize_,
                                                                         chunksize_,
                                                                         nchannels_,
                                                                         nsamples_)));
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
    " An LSL stream named NAME results in the follwing AC variables: NAME containing the data,"
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
    " The configuration regarding the AC variable size and the LSL stream inlet applies plugin wide. To use per-stream"
    " configuration this plugin must be instantiated multiple times.\n"
    " This implementation should only be used for evaluation purposes and"
    " is not yet intended for production use.")
/*
 * Local variables:
 * c-basic-offset: 4
 * compile-command: "make"
 * End:
 */
