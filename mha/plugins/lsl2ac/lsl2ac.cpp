// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2020 HörTech gGmbH
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

// This is a function try block. Really ugly syntax but the only way to handle exceptions
// in the ctor initializer list. See http://www.gotw.ca/gotw/066.htm
lsl2ac::save_var_t::save_var_t(const lsl::stream_info& info_, const algo_comm_t& ac_, underrun_behavior ub_,
                               overrun_behavior ob_, int buflen_, int chunksize_) try :
    stream(info_,buflen_,chunksize_),
        buf(new mha_real_t[info_.channel_count()]),
        ac(ac_),
        ts_name(info_.name()+"_ts"),
        tc_name(info_.name()+"_tc"),
        tic(std::chrono::steady_clock::now()),
        ub(ub_),
        ob(ob_),
        name(info_.name())
        {
            for(int i=0; i<info_.channel_count(); i++)
                buf.get()[i]=0.0;
            stream.open_stream();
            // According to lsl doc the first call to time_correction takes several ms, subsequent
            // calls are 'instant'. Calling it here to avoid blocking later.
            tc=stream.time_correction();
            cv.stride=info_.channel_count();
            cv.num_entries=info_.channel_count();
            cv.data_type=MHA_AC_FLOAT;
            cv.data=buf.get();
            ac.insert_var(ac.handle,info_.name().c_str(),cv);
            ac.insert_var_double(ac.handle,ts_name.c_str(), &ts);
            ac.insert_var_double(ac.handle,tc_name.c_str(), &tc);
        }
 catch (MHA_Error& e) {
    //The framework can handle MHA_Errors. Just re-throw
    throw;
 }
 catch (std::exception & e) {
     // LSL does not document its exceptions very well, but all exceptions I encountered inherit from std::exception
     // Rethrow as MHA_Error
     throw MHA_Error(__FILE__,__LINE__,"Could not subscribe to stream %s: %s",info_.name().c_str(),e.what());
 }
 catch (...) {
     //No matter what happened before, throw an MHA_Error so that the framework can handle that
     throw MHA_Error(__FILE__,__LINE__,"Could not subscribe to stream %s: Unknown error",info_.name().c_str());
 }

lsl::stream_info lsl2ac::save_var_t::info() {
    return stream.info();
}

unsigned  lsl2ac::save_var_t::num_entries() {
    return stream.info().channel_count();
}

void lsl2ac::save_var_t::receive_frame()  {
    if(skip){
        return;
    }
    else{
        if(ub==underrun_behavior::Zero)
            impl_receive_frame_zero();
        else if(ub==underrun_behavior::Copy)
            impl_receive_frame_copy();
        else if(ub==underrun_behavior::Abort)
            impl_receive_frame_abort();
        else
            throw MHA_Error(__FILE__,__LINE__,"Bug: Unknown underrun behavior: %i!",static_cast<int>(ub));
    }
}

double lsl2ac::save_var_t::pull_sample(){
    double tmp=0.0;
    if(ob==overrun_behavior::Discard){
        while(stream.samples_available()){
            tmp=stream.pull_sample(buf.get(),num_entries(),0.0f);
        }
    }
    else{
        tmp=stream.pull_sample(buf.get(),num_entries(),0.0f);
    }
    return tmp;
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
    ac.insert_var_double(ac.handle,ts_name.c_str(), &ts);
    ac.insert_var_double(ac.handle,tc_name.c_str(), &tc);
}

void lsl2ac::save_var_t::impl_receive_frame_zero(){
    try {
        ts=pull_sample();
        if(ts==0.0){
            for(size_t i=0;i<num_entries();i++)
                buf[i]=0.0f;
        }
        get_time_correction();
        insert_vars();
    }
    //If the stream is recoverable, lsl does not throw, instead tries to recover.
    // Handle any other exception as a permanent underrun
    catch(...){
        for(size_t i=0;i<num_entries();i++)
            buf[i]=0.0f;
        stream.close_stream();
        skip=true;
    }
}

void lsl2ac::save_var_t::impl_receive_frame_copy(){
    try {
        double old_ts=ts;
        //If no new samples are there, buf does not get touched...
        ts=pull_sample();
        //...but we need to reset ts to the old value
        if(ts==0.0){
            ts=old_ts;
        }
        get_time_correction();
        insert_vars();
    }
    //If the stream is recoverable, lsl does not throw, instead tries to recover.
    // Handle any other exception as a permanent underrun
    catch(...){
        stream.close_stream();
        skip=true;
    }
}
void lsl2ac::save_var_t::impl_receive_frame_abort(){
    class UnderrunError{};
    try {
        ts=pull_sample();
        if(ts==0.0){
            throw UnderrunError();
        }
        else{
            get_time_correction();
            insert_vars();
        }
    }
    catch(UnderrunError &e){
        throw MHA_Error(__FILE__,__LINE__,"Stream: %s has no new samples!",name.c_str());
    }
    catch(std::exception &e){
        throw MHA_Error(__FILE__,__LINE__,"Error in stream %s: %s",name.c_str(),e.what());
    }
    catch(...){
        throw MHA_Error(__FILE__,__LINE__,"Unknown error in stream %s!",name.c_str());
    }
}

lsl2ac::lsl2ac_t::lsl2ac_t(algo_comm_t iac,const char* chain, const char* algo)
    : MHAPlugin::plugin_t<lsl2ac::cfg_t>("Receive LSL streams and copy"
                                         " them to AC variables.",iac)
{
    //Nota bene: The configuration variables are not connected to the patch bay because
    // we either lock them anyway during prepare or they are not used within the context
    // of the cfg_t class.
    insert_member(streams);
    insert_member(activate);
    insert_member(underrun_behavior);
    insert_member(overrun_behavior);
    insert_member(buffersize);
    insert_member(chunksize);
    insert_item("available_streams",&available_streams);
    patchbay.connect(&available_streams.prereadaccess,this,&lsl2ac_t::get_all_stream_names);
}

void lsl2ac::lsl2ac_t::prepare(mhaconfig_t& cf)
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
        auto c=new cfg_t(ac, static_cast<lsl2ac::underrun_behavior>(underrun_behavior.data.get_index()),
                         static_cast<lsl2ac::overrun_behavior>(overrun_behavior.data.get_index()),
                         buffersize.data, chunksize.data, streams.data);
        push_config(c);
    }
}

void lsl2ac::lsl2ac_t::setlock(bool lock_){
    underrun_behavior.setlock(lock_);
    overrun_behavior.setlock(lock_);
    buffersize.setlock(lock_);
    chunksize.setlock(lock_);
    streams.setlock(lock_);
}

void lsl2ac::lsl2ac_t::get_all_stream_names()
{
    available_streams.data.clear();
    std::vector<lsl::stream_info> streams = lsl::resolve_streams();
    available_streams.data.resize(streams.size());
    for(const auto& istream : streams)
        available_streams.data.push_back(istream.name());
}

lsl2ac::cfg_t::cfg_t(const algo_comm_t& ac_, underrun_behavior ub_, overrun_behavior ob_, int bufsize_, int chunksize_, const std::vector<std::string>& streamnames_)
{
    for(auto& name : streamnames_) {
        //Find all streams with matching name and take the first one, throw if none found
        auto matching_streams=lsl::resolve_stream("name",name,/*minimum=*/1,/*timeout=*/1.0);
        if(!matching_streams.size())
            throw MHA_Error(__FILE__,__LINE__,"No stream with name %s found!",name.c_str());
        varlist.emplace(std::make_pair(name,std::make_unique<save_var_t>(matching_streams[0],ac_,ub_,ob_,bufsize_,chunksize_)));
    }
}

void lsl2ac::cfg_t::process(){
    for(auto& var : varlist){
        var.second->receive_frame();
    }
}

MHAPLUGIN_CALLBACKS(lsl2ac,lsl2ac::lsl2ac_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(lsl2ac,lsl2ac::lsl2ac_t,spec,spec)
MHAPLUGIN_DOCUMENTATION\
(lsl2ac,
 "data-import network-communication",
 " This plugin provides a mechanism to receive lsl streams and"
 " convert them to ac variables."
 " It currently only supports MHA\\_AC\\_FLOAT-type variables. Type conversions from other"
 " types of stream should be handled in the background."
 "\n"
 "This is a very early version of the plugin and lacks many features."
 " It is probably not real-time safe. The behavior on underrun and overrun is configurable."
 " On overrun, i.e. there's more than one sample available, the user can decide if all samples but the newest "
 " should be discarded or if the overrun should be ignored and only the oldest sample should be saved to AC,"
 " leaving newer samples in the LSL buffers."
 " If there is no new data, the ac variables get, depending on the configuration variable underrun\\_behavior,"
 " either zero'ed, copied from the last pull, or the plugin will abort."
 " The maximum buffer size and chunk length the LSL inlet shall use are configurable. For more details on the meaning of these"
 " please consult the LSL documentation.\n"
 " This implementation should only be used for evaluation purposes and"
 " is not intended for production use.")
/*
 * Local variables:
 * c-basic-offset: 4
 * compile-command: "make"
 * End:
 */
